// exlnk4.cpp, v1.0 2009/04/09
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts LNK4 (*.dat) archives.

#include <windows.h>
#include <Xcompress.h>

#include "as-util.h"

struct LNK4HDR {
  unsigned char signature[4]; // "LNK4"
  unsigned long data_base;
};

struct LNK4ENTRY {
  unsigned long offset; // lba
  unsigned long length; // blocks
};

struct IMGHDR {
  unsigned short width;
  unsigned short height;
  unsigned short depth;
  unsigned short unknown1;
};

HRESULT __cdecl DecompressData(XCOMPRESS_FILE_HEADER_LZXNATIVE& xhdr, 
                               unsigned char*                   buff, 
                               SIZE_T                           len, 
                               unsigned char*&                  out_buff, 
                               unsigned long&                   out_len)
{
    XMEMDECOMPRESSION_CONTEXT decmpContext = NULL;
    HRESULT                   hr           = E_FAIL;

    xhdr.ContextFlags                         = as::flip_endian(xhdr.ContextFlags);
    xhdr.CodecParams.Flags                    = as::flip_endian(xhdr.CodecParams.Flags);
    xhdr.CodecParams.WindowSize               = as::flip_endian(xhdr.CodecParams.WindowSize);
    xhdr.CodecParams.CompressionPartitionSize = as::flip_endian(xhdr.CodecParams.CompressionPartitionSize);

    xhdr.UncompressedSizeLow    = as::flip_endian(xhdr.UncompressedSizeLow);
    xhdr.CompressedSizeLow      = as::flip_endian(xhdr.CompressedSizeLow);
    xhdr.UncompressedBlockSize  = as::flip_endian(xhdr.UncompressedBlockSize);
    xhdr.CompressedBlockSizeMax = as::flip_endian(xhdr.CompressedBlockSizeMax);

    out_buff = new unsigned char[xhdr.UncompressedSizeLow];
    out_len  = 0;

    if (FAILED(hr = XMemCreateDecompressionContext(XMEMCODEC_LZX, &xhdr.CodecParams, xhdr.ContextFlags, &decmpContext))) {
        return E_FAIL;
    }

    unsigned char* p = buff;

    while (out_len != xhdr.UncompressedSizeLow) {
      unsigned long block_len = as::flip_endian(*(unsigned long*) p);
      p += 4;

      SIZE_T decomp_len = xhdr.UncompressedSizeLow;   
      hr = XMemDecompress(decmpContext, out_buff + out_len, &decomp_len, p, block_len);
      p += block_len;

      out_len += decomp_len;
    }

    XMemDestroyDecompressionContext(decmpContext);

    return hr;
}


int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exlnk4 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = as::open_or_die(in_filename, O_RDWR| O_BINARY);

  LNK4HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = hdr.data_base - sizeof(hdr);
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);

  LNK4ENTRY* entries = (LNK4ENTRY*) toc_buff;

  for (unsigned long i = 0; i == 0 || entries[i].offset; i++) {
    unsigned long  len  = entries[i].length * 1024;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, hdr.data_base + entries[i].offset * 2048, SEEK_SET);
    read(fd, buff, len);

    if (*(unsigned long*)buff == 0xEE12F50F) {
      XCOMPRESS_FILE_HEADER_LZXNATIVE* xhdr      = (XCOMPRESS_FILE_HEADER_LZXNATIVE*) buff;    
      unsigned char*                   data_buff = buff + sizeof(*xhdr);
      unsigned long                    data_len  = len - sizeof(*xhdr);

      unsigned long  out_len  = 0;
      unsigned char* out_buff = NULL;
      DecompressData(*xhdr, data_buff, data_len, out_buff, out_len);

      IMGHDR* imghdr = (IMGHDR*) out_buff;

      if (imghdr->depth == 8 || imghdr->depth == 24 || imghdr->depth == 32) {
        unsigned long options = as::WRITE_BMP_FLIP;

        if (imghdr->depth != 8) {
          options |= as::WRITE_BMP_BIGENDIAN;
        }

        as::write_bmp(as::stringf("%05d.bmp", i), 
                      out_buff + sizeof(*imghdr), 
                      out_len - sizeof(*imghdr), 
                      imghdr->width, 
                      imghdr->height,
                      imghdr->depth / 8,
                      options);
      } else {
        as::write_file(as::stringf("%05d", i), out_buff, out_len);
      }

      delete [] out_buff;
    } else {
      as::write_file(as::stringf("%05d", i), buff, len);
    }

    delete [] buff;
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}
