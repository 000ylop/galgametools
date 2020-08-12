// ge2bmp.cpp, v1.01 2008/06/28
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts GE (*.pgd) images to bitmaps.

#include <algorithm>
#include "as-util.h"

struct PGDHDR {
  unsigned char  signature[2]; // "GE"
  unsigned short header_length;
  unsigned char  unknown1[8];
  unsigned long  width;
  unsigned long  height;
  unsigned long  width2;
  unsigned long  height2;
};

struct PGDDATAHDR {
  unsigned char  signature;       // "11_C"
  unsigned long  original_length;
  unsigned long  length;          // includes size of this header
};

void un11c(unsigned char* buff,
           unsigned long  len,
           unsigned char* out_buff,
           unsigned long  out_len)
{  
  unsigned char* out_start = out_buff;
  unsigned char* out_end   = out_buff + out_len;

  unsigned char* end = buff + len;

  while (out_buff < out_end) {
    unsigned char flags = *buff++;

    for (unsigned long i = 0; i < 8 && out_buff < out_end; i++) {
      unsigned long  n   = 0;
      unsigned char* src = NULL;

      if (flags & 1) {
        unsigned long p = *(unsigned short*) buff;
        buff += 2;        

        n = *buff++;

        if (out_buff - out_start < 4092) {
          src = out_start + p;
        } else {
          src = out_buff - (4092 - p);
        }
      } else {
        n   = *buff++;
        src = buff;
        buff += n;
      }

      while (n--) {
        *out_buff++ = *src++;
      }

      flags >>= 1;
    }
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "ge2bmp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pgd> [output.bmp]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename = as::get_file_prefix(in_filename) + ".bmp";

  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PGDHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  PGDDATAHDR datahdr;
  read(fd, &datahdr, sizeof(datahdr));

  unsigned long  len  = datahdr.length - sizeof(datahdr);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  unsigned long  out_len  = datahdr.original_length;
  unsigned char* out_buff = new unsigned char[out_len];
  un11c(buff, len, out_buff, out_len);

  unsigned long  rgba_len  = hdr.width * hdr.height * 4;
  unsigned char* rgba_buff = new unsigned char[rgba_len];

  unsigned long color_len = hdr.width * hdr.height;

  for (unsigned long y = 0; y < hdr.height; y++) {
    unsigned char* r_line    = out_buff + y * hdr.width;
    unsigned char* g_line    = r_line + color_len;
    unsigned char* b_line    = g_line + color_len;
    unsigned char* a_line    = b_line + color_len;
    unsigned char* rgba_line = rgba_buff + y * hdr.width * 4;

    for (unsigned long x = 0; x < hdr.width; x++) {
      rgba_line[x * 4 + 0] = r_line[x];
      rgba_line[x * 4 + 1] = g_line[x];
      rgba_line[x * 4 + 2] = b_line[x];
      rgba_line[x * 4 + 3] = a_line[x];
    }
  }

  as::write_bmp(out_filename,
                rgba_buff,
                rgba_len,
                hdr.width,
                hdr.height,
                4,
                as::WRITE_BMP_FLIP);

  delete [] rgba_buff;
  delete [] out_buff;
  delete [] buff;

  return 0;
}
