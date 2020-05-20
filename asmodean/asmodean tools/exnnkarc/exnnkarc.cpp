// exnnkarc.cpp, v1.0 2011/03/14
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts SCXA (*.arc) archives used by 
// Piaキャロットへようこそ！！4　-夏の恋活- (XBOX360).

#include "as-util.h"

struct ARCHDR {
  unsigned char signature[4]; // "SCXA"
  unsigned long data_base;
  unsigned long entry_count;
};

struct ARCENTRY1 {
  unsigned long entry2_offset;
};

struct ARCENTRY2 {
  unsigned long offset;
  unsigned long length;
  char          filename[1];
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exnnkarc v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.arc>\n", argv[0]);
    return -1;
  }

  string filename(argv[1]);

  int fd = as::open_or_die(filename, O_RDONLY | O_BINARY);

  ARCHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  entries1_len = hdr.entry_count * sizeof(ARCENTRY1);
  ARCENTRY1*     entries1     = new ARCENTRY1[hdr.entry_count];
  read(fd, entries1, entries1_len);

  unsigned long  entries2_len  = hdr.data_base - sizeof(hdr) - entries1_len;
  unsigned char* entries2_buff = new unsigned char[entries2_len];
  read(fd, entries2_buff, entries2_len);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    ARCENTRY2* entry2 = (ARCENTRY2*) (entries2_buff + entries1[i].entry2_offset);

    as::make_path(entry2->filename);

    as::write_slice(fd, 
                    entry2->filename,
                    (unsigned long long) hdr.data_base + entry2->offset,
                    entry2->length);
  }

  delete [] entries2_buff;
  delete [] entries1;

  close(fd);
  
  return 0;
}
