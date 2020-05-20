// exisa.cpp, v1.1 2011/04/29
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from ISM ARCHIVED (*.isa) archives.

#include "as-util.h"

#define ISA_VERSION 2

struct ISAHDR {
  unsigned char  signature[12]; // "ISM ARCHIVED"
  unsigned short entry_count;
  unsigned short unknown;
};

struct ISAENTRY {
#if ISA_VERSION >= 2
  char          filename[52];
#else
  char          filename[36];
#endif
  unsigned long offset;
  unsigned long length;
  unsigned long pad;
};

void unobfuscate(unsigned char* buff, unsigned long len) {
  unsigned long* buff_words     = (unsigned long*) buff;
  unsigned long  buff_words_len = len / 4;

  for (unsigned long i = 0; i < buff_words_len; i++) {
    buff_words[i] ^= ~(len + buff_words_len - i);
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exisa v1.1 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.isa>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  ISAHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  ISAENTRY*      entries     = new ISAENTRY[hdr.entry_count];
  unsigned long  entries_len = sizeof(ISAENTRY) * hdr.entry_count;
  read(fd, entries, entries_len);

  if (entries[0].filename[sizeof(entries[0].filename) - 1]) {
    unobfuscate((unsigned char*) entries, entries_len);
  }

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    as::write_slice(fd, entries[i].filename, entries[i].offset, entries[i].length);
  }

  delete [] entries;

  close(fd);

  return 0;
}
