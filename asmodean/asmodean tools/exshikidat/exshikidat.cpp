// exshikidat.cpp, v1.01 2011/07/31
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.dat archives used by ÉVÉLÉKÉ~. (YaneSDK?)

#include "as-util.h"

struct DATHDR {
  unsigned short entry_count;
};

struct DATENTRY {
  char           filename[34];
  unsigned short obfuscated_length;
  unsigned long  length;
  unsigned long  offset;
};

void read_unobfuscate(int           fd,
                      void*         buff, 
                      unsigned long len,
                      unsigned long unobfuscate_len)
{
  read(fd, buff, len);

  unsigned char* p   = (unsigned char*) buff;
  unsigned char* end = p + std::min(len, unobfuscate_len);

  while (p < end) {
    *p++ ^= 0x80;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exshikidat v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  DATHDR hdr;
  read_unobfuscate(fd, &hdr, sizeof(hdr), sizeof(hdr));

  unsigned long entries_len = sizeof(DATENTRY) * hdr.entry_count;
  DATENTRY*     entries     = new DATENTRY[hdr.entry_count];
  read_unobfuscate(fd, entries, entries_len, entries_len);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read_unobfuscate(fd, buff, len, entries[i].obfuscated_length);

    as::make_path(entries[i].filename);
    as::write_file(entries[i].filename, buff, len);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
