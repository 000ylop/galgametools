// fcb2bmp.cpp, v1.1 2008/07/28
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool decompresses F&C's fcb1 (*.fcb) graphics.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>
#include "arc4common.h"
#include "zlib.h"

using std::string;

struct FCBHDR {
  unsigned char signature[4]; // "fcb1"
  unsigned long width;
  unsigned long height;
  unsigned long unknown1;
};

struct FCBZHDR {
  unsigned long type;
  unsigned long original_length;
  unsigned long length;
};

// Is this a known algorithm?  Why doesn't everybody just use deflate...
unsigned long unfcb1(unsigned char* buff, 
                     unsigned long  len,
                     unsigned char* out_buff,
                     unsigned long  out_len,
                     unsigned long  width)
{
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;  

  unsigned long  first_r = 0x80;
  unsigned long  first_g = 0x80;
  unsigned long  first_b = 0x80;
  unsigned long  first_a = 0xFF;  

  unsigned long  last_r  = 0;
  unsigned long  last_g  = 0;
  unsigned long  last_b  = 0;
  unsigned long  last_a  = 0;

  for (unsigned long x = 0; out_buff < out_end; x++) {
    if (!(x % width)) {
      last_r = first_r;
      last_g = first_g;
      last_b = first_b;
      last_a = first_a;
      x      = 0;
    }

    unsigned long c = *buff++;

    unsigned long r = 0;
    unsigned long g = 0;
    unsigned long b = 0;
    unsigned long a = 0;

    if (c & 0x80) {
      if (c & 0x40) {
        if (c & 0x20) {
          if (c & 0x10) {
            if (c & 0x08) {
              if (c == 0xFE) {
                r = *buff++ - 0x80;
                g = *buff++ - 0x80;
                b = *buff++ - 0x80;
              } else {
                r = *buff++ - 0x80;
                g = *buff++ - 0x80;
                b = *buff++ - 0x80;
                a = *buff++ - 0x80;
              }
            } else {
              c = (c << 8) | *buff++;
              c = (c << 8) | *buff++;
              c = (c << 8) | *buff++;

              r = ((c >> 20) & 0x7F) - 0x40;
              g = ((c >> 14) & 0x3F) - 0x20;
              b = ((c >> 8)  & 0x3F) - 0x20;
              a = (c         & 0xFF) - 0x80;
            }
          } else {
            c = (c << 8) | *buff++;
            c = (c << 8) | *buff++;

            r = ((c >> 14) & 0x3F) - 0x20;
            g = ((c >> 10) & 0x0F) - 0x08;
            b = ((c >> 6)  & 0x0F) - 0x08;
            a = (c         & 0x3F) - 0x20;
          }
        } else {
          c = (c << 8) | *buff++;
          c = (c << 8) | *buff++;

          r = ((c >> 13) & 0xFF) - 0x80;
          g = ((c >> 7)  & 0x3F) - 0x20;
          b = (c         & 0x7F) - 0x40;
        }
      } else {
        c = (c << 8) | *buff++;

        r = ((c >> 8) & 0x3F) - 0x20;
        g = ((c >> 4) & 0x0F) - 0x08;
        b = (c        & 0x0F) - 0x08;
      }
    } else {
      r = ((c >> 4) & 0x07) - 0x04;      
      g = ((c >> 2) & 0x03) - 0x02;
      b = (c        & 0x03) - 0x02;
    }

    last_r += r + b;
    last_g += r;
    last_b += r + g;
    last_a += a;

    *out_buff++ = last_r;
    *out_buff++ = last_g;
    *out_buff++ = last_b;
    *out_buff++ = last_a;

    if (x == 0) {
      first_r = last_r;
      first_g = last_g;
      first_b = last_b;
      first_a = last_a;
    }
  }

  return out_len - (out_end - out_buff);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "fcb2bmp, v1.1 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.fcb> [output.bmp]\n\n", argv[0]);
    return -1;
  }

  string in_filename  = argv[1];
  string out_filename = get_file_prefix(in_filename) + ".bmp";
  
  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  FCBHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = get_file_size(fd) - sizeof(hdr);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  unsigned long  out_len  = 0;
  unsigned char* out_buff = NULL;

  FCBZHDR* zhdr = (FCBZHDR*) buff;

  // Only seen the ZLIB type for FCB graphics in EVE Go
  if (zhdr->type == 0x5A6D68) {
    zhdr->original_length = flip_endian(zhdr->original_length);
    zhdr->length          = flip_endian(zhdr->length);

    out_len  = zhdr->original_length;
    out_buff = new unsigned char[out_len];
    uncompress(out_buff, &out_len, buff + sizeof(*zhdr), zhdr->length);
  } else {
    uncompress_sequence(buff, len, out_buff, out_len, in_filename);
  }

  unsigned long  bmp_len    = hdr.width * hdr.height * 4;
  unsigned char* bmp_buff   = new unsigned char[bmp_len];
  unsigned long  bmp_actual = unfcb1(out_buff, out_len, bmp_buff, bmp_len, hdr.width);

  unsigned char* flip_buff  = new unsigned char[bmp_len];

  for (unsigned long y = 0; y < hdr.height; y++) {
    unsigned char* src = bmp_buff  + y * hdr.width * 4;
    unsigned char* dst = flip_buff + (hdr.height - y - 1) * hdr.width * 4;

    for (unsigned long x = 0; x < hdr.width; x++) {
      dst[x * 4 + 0] = src[x * 4 + 2];
      dst[x * 4 + 1] = src[x * 4 + 1];
      dst[x * 4 + 2] = src[x * 4 + 0];
      dst[x * 4 + 3] = src[x * 4 + 3];
    }
  }

  int out_fd = open_or_die(out_filename,
                           O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                           S_IREAD | S_IWRITE);

  {
    BITMAPFILEHEADER bmf;
    BITMAPINFOHEADER bmi;

    memset(&bmf, 0, sizeof(bmf));
    memset(&bmi, 0, sizeof(bmi));

    bmf.bfType     = 0x4D42;
    bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + bmp_actual;
    bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi);

    bmi.biSize     = sizeof(bmi);
    bmi.biWidth    = hdr.width;
    bmi.biHeight   = hdr.height;
    bmi.biPlanes   = 1;
    bmi.biBitCount = 32;
    
    write(out_fd, &bmf, sizeof(bmf));
    write(out_fd, &bmi, sizeof(bmi));
  }

  write(out_fd, flip_buff, bmp_actual);
  close(out_fd);

  delete [] flip_buff;
  delete [] bmp_buff;
  delete [] out_buff;

  return 0;
}
