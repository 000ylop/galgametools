// exakbdat.cpp, v1.0 2010/10/03
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.dat archives used by ‚ ‚©‚×‚¥‚»‚Ó‚Æ‚Â‚£'s desktop
// accessories.

#include "as-util.h"

#pragma pack(1)
struct DATHDR {
  unsigned short entry_count;
};

struct DATENTRY {
  char           filename[34];
  unsigned short obfuscation_length;
  unsigned long  length;
  unsigned long  offset;
};
#pragma pack()

void unobfuscate(void* buff, unsigned long len) {
  unsigned char* p = (unsigned char*) buff;

  while (len--) {
    *p++ ^= 0x80;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exakbdat v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix(as::get_file_prefix(in_filename));
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  
  DATHDR hdr;
  read(fd, &hdr, sizeof(hdr));
  unobfuscate(&hdr, sizeof(hdr));  

  unsigned long entries_len = sizeof(DATENTRY) * hdr.entry_count;
  DATENTRY*     entries     = new DATENTRY[hdr.entry_count];
  read(fd, entries, entries_len);
  unobfuscate(entries, entries_len);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);
    unobfuscate(buff, std::min<unsigned long>(entries[i].length, entries[i].obfuscation_length));

    as::write_file(prefix + "+" + entries[i].filename, buff, len);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
