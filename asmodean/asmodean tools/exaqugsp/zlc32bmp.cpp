// zlc32bmp.cpp, v1.0 2008/04/29
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool decompresses ZLC3 (*.bmz) data.  Usually bitmaps...

#include "as-util.h"
#include "zlib.h"

struct ZLC3HDR {
  unsigned char signature[4]; // "ZLC3"
  unsigned long original_length;
};

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "zlc32bmp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bmz>\n", argv[0]);   
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename;

  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  ZLC3HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = as::get_file_size(fd) - sizeof(hdr);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  unsigned long  out_len  = hdr.original_length;
  unsigned char* out_buff = new unsigned char[out_len];
  uncompress(out_buff, &out_len, buff, len);

  if (out_filename.empty()) {
    out_filename = as::get_file_prefix(in_filename) + as::guess_file_extension(out_buff, out_len);
  }

  fd = as::open_or_die(out_filename,
                       O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                       S_IREAD | S_IWRITE);
  write(fd, out_buff, out_len);
  close(fd);

  delete [] out_buff;
  delete [] buff;

  return 0;
}

