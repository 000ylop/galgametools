// exlibiarc.cpp, v1.0 2007/05/28
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from LiBi's *.arc archives.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <direct.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>
#include "lzss.h"

using std::string;

struct ARCHDR {
  unsigned long entry_count;
};

struct ARCENTRY {
  char          filename[20];
  unsigned long original_length;
  unsigned long length;
  unsigned long offset;
};

void unobfuscate(ARCENTRY& entry) {
  for (unsigned long i = 0; i < sizeof(entry.filename); i++) {
    entry.filename[i] ^= 0xFF;
  }
}

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
    fprintf(stderr, "exlibiarc v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.arc>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  ARCHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  ARCENTRY* entries = new ARCENTRY[hdr.entry_count];
  read(fd, entries, sizeof(ARCENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unobfuscate(entries[i]);

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (entries[i].length != entries[i].original_length) {
      unsigned long  temp_len  = entries[i].original_length;
      unsigned char* temp_buff = new unsigned char[temp_len];
      unlzss(buff, len, temp_buff, temp_len);

      delete [] buff;

      len  = temp_len;
      buff = temp_buff;
    }

    int out_fd = open_or_die(entries[i].filename, 
                             O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
                             S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}

