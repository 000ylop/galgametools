// excatcab.cpp, v1.0 2007/03/17
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from CATTLEYA's PackDat3 (*.cab) archives.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>

using std::string;

struct CABHDR {
  unsigned char signature[8]; // "PackDat3A"
  unsigned long entry_count;
};

struct CABENTRY {
  char          filename[256];
  unsigned long offset;
  unsigned long length;
  unsigned long original_length; // ??
};

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "excatcab, v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.cab>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  CABHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long entries_len = sizeof(CABENTRY) * hdr.entry_count;
  CABENTRY*     entries     = new CABENTRY[hdr.entry_count];
  read(fd, entries, entries_len);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  =  entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    int out_fd = open_or_die(entries[i].filename, 
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  delete [] entries;

  return 0;
}
