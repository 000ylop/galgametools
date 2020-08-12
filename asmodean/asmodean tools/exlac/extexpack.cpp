// extexpack.cpp, v1.0 2008/12/27
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Extracts TEX PACK0.02 (*.TEX) archives used by Leaf.

#include "as-util.h"

struct TPHDR {
  unsigned char signature[12]; // "TEX PACK0.02"
  unsigned long entry_count;
  unsigned long unknown[4];
};

struct TPENTRY {
  char          filename[32];
  unsigned long offset;  
  unsigned long length;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "extexpack v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.tex>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  TPHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long entries_len = sizeof(TPENTRY) * hdr.entry_count;
  TPENTRY*      entries     = new TPENTRY[hdr.entry_count];
  read(fd, entries, entries_len);

  unsigned long data_base = sizeof(hdr) + entries_len;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, data_base + entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    as::write_file(in_filename + "+" + entries[i].filename, buff, len);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}