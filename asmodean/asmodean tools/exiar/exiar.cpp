// exiar.cpp, v1.03 2010/04/06
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts graphics from iar (*.iar) archives.  Image fragments
// are merged with the base.

// Thanks to puu for the decompression code.  I started implementing it myself
// but my code wasn't looking any prettier, so I stopped :).

#include "as-util.h"
#include "decode_lz.h"

// Define this to support V3 of IAR achives.  The TOC entries have a slightly
// different structure.
#define IAR_VERSION 4

struct IARHDR {
  unsigned char signature[4]; // "iar "
  unsigned long version;
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long entry_count;
  unsigned long entry_count2; // why?
};

struct IARENTRY {
  unsigned long offset;
#if IAR_VERSION >= 3
  unsigned long unknown;
#endif
};

struct IARDATAHDR {
  unsigned long flags;
  unsigned long unknown1;
  unsigned long original_length;
  unsigned long unknown2;
  unsigned long length;
  unsigned long pal_length;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long width;
  unsigned long height;
  unsigned long stride;
#if IAR_VERSION >= 4
  unsigned char unknown5[28];
#else
  unsigned char unknown5[20];
#endif
};

static const unsigned long FLAGS_8BIT  = 0x00000002;
static const unsigned long FLAGS_24BIT = 0x0000001c;
static const unsigned long FLAGS_ALPHA = 0x00000020;
static const unsigned long FLAGS_FRAG  = 0x00000800;

struct IARFRAGHDR {
  unsigned long base_index;
  unsigned long start_line;
  unsigned long line_count;
};

struct IARFRAGLINE {
  unsigned short seq_count;
};

struct IARFRAGSEQ {
  unsigned short skip_length;
  unsigned short copy_length;
};

void read_entry(unsigned long   fd,
                IARENTRY*       entries, 
                unsigned long   index, 
                IARDATAHDR&     datahdr, 
                unsigned char*& out_buff, 
                unsigned long&  out_len,
                unsigned short& out_depth)
{
  lseek(fd, entries[index].offset, SEEK_SET);
  read(fd, &datahdr, sizeof(datahdr));

  unsigned long  len  = datahdr.length;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  out_len  = datahdr.original_length;

  if (datahdr.length != datahdr.original_length) {
    out_buff = new unsigned char[out_len];
    decode_lz(buff, len, out_buff, out_len);

    delete [] buff;
  } else {
    out_buff = buff;
  }

  if (datahdr.flags & FLAGS_8BIT) {
    out_depth = 8;
  } else {
    out_depth = 24;
  } 
    
  // Assume we never have 8-bit data with alpha
  if (datahdr.flags & FLAGS_ALPHA) {
    out_depth = 32;
  }
}

void merge_fragment(int             fd,
                    IARENTRY*       entries,
                    const string&   prefix, 
                    unsigned long   index,
                    unsigned long&  width,
                    unsigned long&  height,
                    unsigned long&  stride,
                    unsigned char*  buff,
                    unsigned char*& out_buff,
                    unsigned long&  out_len) 
{
  IARFRAGHDR* hdr = (IARFRAGHDR*) buff;
  buff += sizeof(*hdr);
 
  unsigned short depth = 0;

  {
    IARDATAHDR     datahdr;
    unsigned long  temp_len  = 0;
    unsigned char* temp_buff = NULL;
    read_entry(fd, entries, hdr->base_index, datahdr, temp_buff, temp_len, depth);

	  unsigned long y_offset = 0;
    unsigned long x_offset = 0;

    unsigned long out_y_offset = height - datahdr.height;

	  if (datahdr.height > height) {
      y_offset      = datahdr.height - height;
      out_y_offset  = 0;
	  }

    if (datahdr.width > width) {
      x_offset      = datahdr.stride - stride;
    }

    out_len  = height * stride;
    out_buff = new unsigned char[out_len];
    memset(out_buff, 0, out_len);

    for (unsigned long y = y_offset; y < datahdr.height; y++) {
      memcpy(out_buff + (y - y_offset + out_y_offset) * stride,
             temp_buff + y * datahdr.stride,
             datahdr.stride - x_offset);
    }

    delete [] temp_buff;
  }

  depth /= 8;

  unsigned long current_line = hdr->start_line;

  for (unsigned long i = 0; i < hdr->line_count; i++) {
    IARFRAGLINE* line_hdr = (IARFRAGLINE*) buff;
    buff += sizeof(*line_hdr);

    unsigned char* out_line = out_buff + current_line * stride;
    current_line++;

    unsigned long x = 0;

    for (int j = 0; j < line_hdr->seq_count; j++) {
      IARFRAGSEQ* line_seq = (IARFRAGSEQ*) buff;
      buff += sizeof(*line_seq);

      unsigned long skip_bytes = line_seq->skip_length * depth;
      unsigned long copy_bytes = line_seq->copy_length * depth;

      x += skip_bytes;

      memcpy(out_line + x, buff, copy_bytes);
      buff += copy_bytes;
      x    += copy_bytes;
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exiar v1.03 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.iar>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix(as::get_file_prefix(in_filename, true));

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  IARHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  IARENTRY* entries = new IARENTRY[hdr.entry_count];
  read(fd, entries, sizeof(IARENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {  
    IARDATAHDR     datahdr;
    unsigned long  out_len  = 0;
    unsigned char* out_buff = NULL;
    unsigned short depth    = 0;

    read_entry(fd, entries, i, datahdr, out_buff, out_len, depth);

    if (datahdr.flags & FLAGS_FRAG) {
      unsigned long  temp_len  = 0;
      unsigned char* temp_buff = NULL;

      merge_fragment(fd,
                     entries,
                     prefix,
                     i, 
                     datahdr.width,
                     datahdr.height,
                     datahdr.stride,
                     out_buff,
                     temp_buff,
                     temp_len);

      delete [] out_buff;

      out_len  = temp_len;
      out_buff = temp_buff;
    }

    as::write_bmp_ex(as::stringf("%s_%05d.bmp", prefix.c_str(), i),
                     out_buff,
                     out_len,
                     datahdr.width,
                     datahdr.height,
                     depth / 8,
                     256,
                     NULL,
                     as::WRITE_BMP_FLIP);

    delete [] out_buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
