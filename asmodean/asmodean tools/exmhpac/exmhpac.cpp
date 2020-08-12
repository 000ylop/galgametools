// exmhpac.cpp, v1.0 2012/07/14
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.pac archives used by マインドハッカー -お前の心は俺のモノ-.

#include "as-util.h"

struct PACHDR {
  unsigned char signature[4]; // 82CF82AD
  unsigned long entry_count;
  unsigned long data_base;
  unsigned long unknown1;
};

struct PACENTRY {
  char          filename[256];
  unsigned long offset;
  unsigned long length;
  unsigned long filename_length;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmhpac v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pac>\n", argv[0]);
    return -1;
  }

  string in_filename = argv[1];

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PACHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  PACENTRY* entries = new PACENTRY[hdr.entry_count];
  read(fd, entries, sizeof(PACENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    string filename(entries[i].filename, entries[i].filename_length);

    as::make_path(filename);
    as::write_slice(fd, filename, hdr.data_base + entries[i].offset, entries[i].length);
  }

  delete [] entries;

  close(fd);

  return 0;
}
