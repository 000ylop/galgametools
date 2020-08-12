// crx2bmp.cpp, v1.05 2012/06/16
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts CIRCUS' CRXG (*.crx) graphics to bitmaps.

#include "as-util.h"
#include "crx-common.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "crx2bmp v1.05 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.crx> [output.bmp]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  
  string out_filename = as::get_file_prefix(in_filename) + ".bmp";

  if (argc > 2) out_filename = argv[2];

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  unsigned long  len  = as::get_file_size(fd);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  if (!proc_crxg(in_filename, out_filename, buff, len)) {
    fprintf(stderr, "%s: unsupported format?\n", in_filename.c_str());
  }

  delete [] buff;
  
  return 0;
}
