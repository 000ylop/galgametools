// kgcg2bmp.cpp, v1.01 2008/04/27
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts GCGK (*.kg) images to bitmaps.

#include "as-util.h"

struct KGHDR {
  unsigned char  signature[4]; // "GCKG"
  unsigned short width;
  unsigned short height;
  unsigned long  data_length;
};

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "kgcg2bmp v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.kg> [output.bmp]\n", argv[0]);   
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename = as::get_file_prefix(in_filename) + ".bmp";

  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  KGHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long* offset_table = new unsigned long[hdr.height];
  read(fd, offset_table, sizeof(unsigned long) * hdr.height);

  unsigned char* buff = new unsigned char[hdr.data_length];  
  read(fd, buff, hdr.data_length);
  close(fd);

  unsigned long  out_len  = hdr.width * hdr.height * 4;
  unsigned char* out_buff = new unsigned char[out_len];
  memset(out_buff, 0, out_len);

  for (int y = 0; y < hdr.height; y++) {
    unsigned char* src      = buff     + offset_table[y];
    unsigned char* out_line = out_buff + (hdr.height - y - 1) * hdr.width * 4;

    for (int x = 0; x < hdr.width;) {
      unsigned char c = *src++;
      unsigned int  n = *src++;

      if (n == 0) {
        n = 256;
      }

      if (c) {
        for (unsigned int i = 0; i < n; i++) {
          out_line[((x + i) * 4) + 3] = c;
          out_line[((x + i) * 4) + 2] = *src++;
          out_line[((x + i) * 4) + 1] = *src++;
          out_line[((x + i) * 4) + 0] = *src++;
        }
      }

      x += n;
    }
  }

  as::write_bmp(out_filename, out_buff, out_len, hdr.width, hdr.height, 4);

  delete [] out_buff;
  delete [] buff;

  return 0;
}

