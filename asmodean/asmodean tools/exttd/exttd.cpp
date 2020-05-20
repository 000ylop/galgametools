// exttd.cpp, v1.0 2007/03/25
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from Morning's .FRC (*.ttd) archives.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>
#include "lzss.h"

using std::string;

struct TTDHDR {
  unsigned char signature[4]; // ".FRC"
  unsigned long key;
  unsigned long unknown1;
  unsigned long entry_count;
  unsigned long unknown2;
};

struct TTDENTRY {
  unsigned long length;
  unsigned long offset;
  unsigned long unknown;
  char          filename[32];
};

struct DSFFHDR {
  unsigned char signature[4]; // "DSFF"
  unsigned long original_length;
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
    fprintf(stderr, "exttd v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.ttd>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  TTDHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long entries_len = sizeof(TTDENTRY) * hdr.entry_count;
  TTDENTRY*     entries     = new TTDENTRY[hdr.entry_count];
  read(fd, entries, entries_len);

  {
    unsigned long* p   = (unsigned long*) entries;
    unsigned long* end = p + entries_len / sizeof(unsigned long);

    while (p < end) {
      *p++ ^= hdr.key;
    }
  }

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (!memcmp(buff, "DSFF", 4)) {
      DSFFHDR* dsffhdr = (DSFFHDR*) buff;

      unsigned long  out_len    = dsffhdr->original_length;
      unsigned char* out_buff   = new unsigned char[out_len];
      unsigned long  out_actual = unlzss(buff + sizeof(*dsffhdr), 
                                         len  - sizeof(*dsffhdr), 
                                         out_buff, 
                                         out_len,
                                         4096,
                                         16, // silly :)
                                         3);

      delete [] buff;

      len  = out_actual;
      buff = out_buff;
    }

    int out_fd = open_or_die(entries[i].filename, 
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  return 0;
}


