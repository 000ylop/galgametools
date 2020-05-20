// exatuworks.cpp, v1.01 2011/12/24
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from Atu Work's CG.DAT archives.

#include "as-util.h"

// â¥êFäËñ]
#define VERSION 2

struct DATHDR {
  unsigned long entry_count;
  unsigned char pad[12];
};

struct DATENTRY {
  unsigned long offset;
  unsigned long length;
  char          filename[24];
};

void unobfuscate(unsigned char* buff, unsigned long len) {
#if VERSION == 2
  static const unsigned char KEY[] = { 0x41, 0x11, 0x54, 0x16, 0x22, 0x33, 0x05 };
#else
  static const unsigned char KEY[] = { 0x22, 0x33, 0x41, 0x11, 0x05, 0x54, 0x16 };
#endif

  for (unsigned long i = 0; i < len; i++) {
    buff[i] ^= KEY[i % sizeof(KEY)];
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exatuworks v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <cg.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  DATHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long entries_len = sizeof(DATENTRY) * hdr.entry_count;
  DATENTRY*     entries     = new DATENTRY[hdr.entry_count];
  read(fd, entries, entries_len);
  unobfuscate((unsigned char*)entries, entries_len);

  for (unsigned long i = 0; i <hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);
    unobfuscate(buff, 32);

    as::write_file(entries[i].filename, buff, len);

    delete [] buff;

  }

  return 0;
}


