// exshupk.cpp, v1.0 2006/12/25
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Extracts data from BANANA shu-shu's *.pk archives.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

struct PKHDR {
  unsigned long entry_count;
};

// For some reason the offset/length are big-endian
struct PKENTRY {
  // unsigned char filename_len;
  // unsigned char filename[filename_len];
  unsigned long offset;
  unsigned long length;
};

unsigned long flip_endian(unsigned long x) {
  return (x >> 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x << 24);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exshupk v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pk>\n", argv[0]);
    return -1;
  }

  char* in_filename = argv[1];
  
  int toc_fd = open(in_filename, O_RDONLY | O_BINARY);

  if (toc_fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  PKHDR hdr;
  read(toc_fd, &hdr, sizeof(hdr));

  int dat_fd = open(in_filename, O_RDONLY | O_BINARY);

  if (dat_fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned char filename_len;
    read(toc_fd, &filename_len, sizeof(filename_len));

    unsigned char filename[4096] = { 0 };
    read(toc_fd, filename, filename_len);

    for (unsigned char j = 0; j < filename_len; j++) {
      filename[j] -= filename_len + 1 - j;
    }

    PKENTRY entry;
    read(toc_fd, &entry, sizeof(entry));

    entry.offset = flip_endian(entry.offset);
    entry.length = flip_endian(entry.length);

    unsigned long  len  = entry.length;
    unsigned char* buff = new unsigned char[len];

    lseek(dat_fd, entry.offset, SEEK_SET);
    read(dat_fd, buff, len);

    int out_fd = open((char*)filename, O_CREAT | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE);

    if (out_fd == -1) {
      fprintf(stderr, "Could not open %s (%s)\n", filename, strerror(errno));
      return -1;
    }

    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  close(dat_fd);
  close(toc_fd);

  return 0;
}
