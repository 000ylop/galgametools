// unlzr.cpp, v1.01 2009/01/28
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Uncompresses LZR data (mostly TIM2 graphics) used by GUST.

#include "as-util.h"

struct LZRHDR {
  unsigned char signature[4]; // "LZR"
  unsigned long flag_bits;
  unsigned long data_length;
  unsigned long original_length;
};

void unlzr(unsigned char* flag_buff,
           unsigned char* buff,
           unsigned char* out_buff,
           unsigned long  out_len)
{
  unsigned char* out_end = out_buff + out_len;

  while (out_buff < out_end) {
    unsigned char flags = *flag_buff++;

    for (unsigned long i = 0; i < 4 && out_buff < out_end; i++) {
      unsigned long n = 0;
      unsigned long p = 0;

      switch (flags >> 6) {
      case 0:
        *out_buff++ = *buff++;
        break;

      case 1:
        n = *buff++;

        while (n--) {
          *out_buff++ = *buff++;
        }
        break;

      case 2:
        n = *buff++;
        
        while (n--) {
          *out_buff++ = *buff;
        }

        buff++;
        break;

      case 3:
        p = *buff++ << 4;
        n = *buff++;

        p |= n >> 4;
        n &= 0xF;

        while (n--) {
          *out_buff = *(out_buff - p);
          out_buff++;
        }
        break;
      }

      flags <<= 2;
    }
  }
}

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "unlzr v1.01, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.tm2.lzr> <output.tm2>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename(argv[2]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  
  LZRHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  flag_len  = (hdr.flag_bits + 7) / 8;
  unsigned char* flag_buff = new unsigned char[flag_len];
  read(fd, flag_buff, flag_len);

  unsigned long  data_len  = hdr.data_length;
  unsigned char* data_buff = new unsigned char[data_len];
  read(fd, data_buff, data_len);
  close(fd);

  unsigned long  out_len   = hdr.original_length;
  unsigned char* out_buff  = new unsigned char[out_len];
  unlzr(flag_buff, data_buff, out_buff, out_len);

  as::write_file(out_filename, out_buff, out_len);

  delete [] out_buff;
  delete [] data_buff;
  delete [] flag_buff;

  return 0;
}