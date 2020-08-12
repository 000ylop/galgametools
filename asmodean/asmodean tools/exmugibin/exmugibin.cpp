// exmugibin.cpp, v1.0 2006/11/21
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)
//   icq:   55244079

// This tool extracts/decompresses data from MugiChoco Club's BIN archives.
// The number of TOC entries is fixed at 2048.  You'll need to rebuild
// if they ever change that.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include "lzss.h"

static const unsigned int MAX_TOC_SIZE = 2048; 

struct BINHDR {
  char          filenames[MAX_TOC_SIZE][16];
  unsigned long offsets[MAX_TOC_SIZE];
  unsigned long original_lengths[MAX_TOC_SIZE];
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmugibin v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bin>\n", argv[0]);
    return -1;
  }

  char* in_filename = argv[1];
  
  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  BINHDR hdr;

  read(fd, &hdr, sizeof(hdr));

  for (unsigned long i = 0; hdr.filenames[i][0] != '\0'; i++) {
    unsigned long  len  = hdr.offsets[i + 1] - hdr.offsets[i];
    unsigned char* buff = new unsigned char[len];

    lseek(fd, hdr.offsets[i], SEEK_SET);
    read(fd, buff, len);

    if (len != hdr.original_lengths[i]) {
      unsigned long  temp_len = hdr.original_lengths[i];
      unsigned char* temp_buff = new unsigned char[temp_len];

      unlzss(buff, len, temp_buff, temp_len);

      std::swap(buff, temp_buff);
      std::swap(len, temp_len);

      delete [] temp_buff;
    }

    int out_fd = open(hdr.filenames[i], 
                      O_CREAT | O_WRONLY | O_BINARY, 
                      S_IREAD | S_IWRITE);

    if (out_fd == -1) {
      fprintf(stderr, "Could not open %s (%s)\n", hdr.filenames[i], strerror(errno));
      return -1;
    }

    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  close(fd);     

  return 0;
}
