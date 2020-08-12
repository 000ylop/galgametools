// exuni2.cpp, v1.0 2010/11/01
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts UNI2 (*.uni) archives.

#include <windows.h>
#include <Xcompress.h>

#include "as-util.h"

struct UNIHDR {
  unsigned char signature[4]; // "UNI2"
  unsigned long unknown1;
  unsigned long entry_count;
  unsigned long toc_offset;
  unsigned long data_offset;

  void flip_endian(void) {
    unknown1    = as::flip_endian(unknown1);
    entry_count = as::flip_endian(entry_count);
    toc_offset  = as::flip_endian(toc_offset);
    data_offset = as::flip_endian(data_offset);
  }
};

struct UNIENTRY {
  unsigned long unknown1;
  unsigned long offset;
  unsigned long length_blocks;
  unsigned long length;

  void flip_endian(void) {
    unknown1      = as::flip_endian(unknown1);
    offset        = as::flip_endian(offset);
    length_blocks = as::flip_endian(length_blocks);
    length        = as::flip_endian(length);
  }
};

struct CWABHDR {
  unsigned char signature[4]; // "CWAB"
  unsigned long entry_count;

  void flip_endian(void) {
    entry_count = as::flip_endian(entry_count);
  }
};

struct CWABENTRY {
  unsigned long length;
  char          filename[60];

  void flip_endian(void) {
    length = as::flip_endian(length);
  }
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exuni2 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.uni>\n\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix = as::get_file_prefix(in_filename, true);
  
  int fd = as::open_or_die(in_filename, O_RDWR| O_BINARY);

  UNIHDR hdr;
  read(fd, &hdr, sizeof(hdr));
  hdr.flip_endian();

  UNIENTRY* entries = new UNIENTRY[hdr.entry_count];
  lseek(fd, hdr.toc_offset * 2048, SEEK_SET);
  read(fd, entries, sizeof(UNIENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    entries[i].flip_endian();

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, (hdr.data_offset + entries[i].offset) * 2048, SEEK_SET);
    read(fd, buff, len);

    string out_prefix = prefix + as::stringf("+%05d", i);

	  if (*(unsigned long*)buff == 0xEE12F50F) {
      XCOMPRESS_FILE_HEADER_LZXNATIVE* xhdr = (XCOMPRESS_FILE_HEADER_LZXNATIVE*) buff;
      unsigned char*                   p    = (unsigned char*) (xhdr + 1);

      xhdr->ContextFlags                         = as::flip_endian(xhdr->ContextFlags);
      xhdr->CodecParams.Flags                    = as::flip_endian(xhdr->CodecParams.Flags);
      xhdr->CodecParams.WindowSize               = as::flip_endian(xhdr->CodecParams.WindowSize);
      xhdr->CodecParams.CompressionPartitionSize = as::flip_endian(xhdr->CodecParams.CompressionPartitionSize);

      xhdr->UncompressedSizeLow    = as::flip_endian(xhdr->UncompressedSizeLow);
      xhdr->CompressedSizeLow      = as::flip_endian(xhdr->CompressedSizeLow);
      xhdr->UncompressedBlockSize  = as::flip_endian(xhdr->UncompressedBlockSize);
      xhdr->CompressedBlockSizeMax = as::flip_endian(xhdr->CompressedBlockSizeMax);

      unsigned long  out_len  = 0;
      unsigned char* out_buff = new unsigned char[xhdr->UncompressedSizeLow];

      XMEMDECOMPRESSION_CONTEXT context = NULL;
      XMemCreateDecompressionContext(XMEMCODEC_LZX, &xhdr->CodecParams, xhdr->ContextFlags, &context);

      while (out_len != xhdr->UncompressedSizeLow) {
        unsigned long block_len = as::flip_endian(*(unsigned long*) p);
        p += 4;

        SIZE_T decomp_len = xhdr->UncompressedSizeLow;   
        XMemDecompress(context, out_buff + out_len, &decomp_len, p, block_len);
        p += block_len;

        out_len += decomp_len;
      }

      XMemDestroyDecompressionContext(context);

      delete [] buff;

      len  = out_len;
      buff = out_buff;
    }

    if (!memcmp(buff, "CWAB", 4)) {
      CWABHDR* cwabhdr = (CWABHDR*) buff;
      cwabhdr->flip_endian();

      CWABENTRY* cwabentries = (CWABENTRY*) (cwabhdr + 1);

      unsigned char* p = (unsigned char*) (cwabentries + cwabhdr->entry_count);     

      for (unsigned long i = 0; i < cwabhdr->entry_count; i++) {
        cwabentries[i].flip_endian();

        as::write_file(out_prefix + "+" + cwabentries[i].filename,
                       p,
                       cwabentries[i].length);

        p += cwabentries[i].length;
      }
    } else {
      as::write_file(out_prefix + as::guess_file_extension(buff, len),
                     buff,
                     len);
    }

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
