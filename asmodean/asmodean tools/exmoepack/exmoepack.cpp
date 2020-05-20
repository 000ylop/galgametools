// exmoepack.cpp, v1.0 2010/07/04
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts MoePack (*.pac) archives.

#include "as-util.h"

struct PACHDR {
  unsigned char signature[8]; // "MoePack"
  unsigned long entry_count;
  unsigned long filenames_length;
};

struct PACENTRY {
  unsigned long filename_offset;
  unsigned long offset;
  unsigned long length;
  unsigned long pad;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmoepack v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pac>\n", argv[0]);    

    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);  

  PACHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  PACENTRY* entries = new PACENTRY[hdr.entry_count];
  read(fd, entries, sizeof(PACENTRY) * hdr.entry_count);

  unsigned long  filenames_len  = hdr.filenames_length;
  char*          filenames_buff = new char[filenames_len];
  read(fd, filenames_buff, filenames_len);

  unsigned long base_offset = tell(fd);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    char* filename = filenames_buff + entries[i].filename_offset;

    as::make_path(filename);
    as::write_slice(fd, filename, base_offset + entries[i].offset, entries[i].length);
  }

  delete [] filenames_buff;
  delete [] entries;

  close(fd);

  return 0;
}
