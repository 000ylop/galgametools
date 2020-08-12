// expkd.cpp, v1.0 2007/03/17
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from TOP's PACK (*.pkd) archives.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>

using std::string;

struct PKDHDR {
  unsigned char signature[4]; // "PACK"
  unsigned long entry_count;
};

struct PKDENTRY {
  char          filename[128];
  unsigned long length;
  unsigned long offset;
};

void unobfuscate(unsigned char* buff, unsigned long len, unsigned char key) {
  unsigned char* end = buff + len;

  while (buff < end) {
    *buff++ ^= key;
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

void make_path(const string& filename) {
  char temp[4096] = { 0 };
  strcpy(temp, filename.c_str());

  for (unsigned long i = 0; i < filename.length(); i++) {
    if (temp[i] == '\\' || temp[i] == '/') {
      char t  = temp[i];
      temp[i] = '\0';
      mkdir(temp);
      temp[i] = t;
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "expkd, v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pkd>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  PKDHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long entries_len = sizeof(PKDENTRY) * hdr.entry_count;
  PKDENTRY*     entries     = new PKDENTRY[hdr.entry_count];
  read(fd, entries, entries_len);

  // Try to guess key (if any)
  unsigned char key = entries[0].filename[sizeof(entries[0].filename) - 1];

  unobfuscate((unsigned char*)entries, entries_len, key);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  =  entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);
    unobfuscate(buff, len, key);

    make_path(entries[i].filename);

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
