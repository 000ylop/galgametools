// excircuspck.cpp, v1.0 2012/06/10
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.pck archives used by D.C.III and others.

#include "as-util.h"

struct PCKHDR {
  unsigned long entry_count;
};

struct PCKENTRY1 {
  unsigned long offset;
  unsigned long length;
};

struct PCKENTRY2 {
  char          filename[56];
  unsigned long offset;
  unsigned long length;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "excircuspck v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pck>\n", argv[0]);

    return -1;
  }

  string in_filename(argv[1]);
  string prefix = as::get_file_prefix(in_filename) + "+";

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PCKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  if (!memcmp(&hdr, "CRXB", 4)) {
    fprintf(stderr, "%s: this is a CRXB archive (use excrxb)\n", in_filename.c_str());
    return 0;
  }

  lseek(fd, hdr.entry_count * sizeof(PCKENTRY1), SEEK_CUR);

  PCKENTRY2* entries = new PCKENTRY2[hdr.entry_count];
  read(fd, entries, hdr.entry_count * sizeof(PCKENTRY2));

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    as::write_slice(fd, prefix + entries[i].filename, entries[i].offset, entries[i].length);
  }

  delete [] entries;

  close(fd);

  return 0;
}
