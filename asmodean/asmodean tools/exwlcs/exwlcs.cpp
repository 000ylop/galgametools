// exwlcs, v1.0 2008/07/26
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data lcsebody+lcsebody.lst used by WG‹æ`VEŠw‰€—dGæ`.

#include "as-util.h"

struct LCSHDR {
  unsigned long entry_count;
};

struct LCSENTRY {
  unsigned long offset;
  unsigned long length;
  char          filename[64];
  unsigned long unknown;
};

unsigned long unobfuscate(LCSHDR& hdr) {
  unsigned long key = (unsigned char) (hdr.entry_count >> 24);
  key = (key << 24) | (key << 16) | (key << 8) | key;

  hdr.entry_count ^= key;

  return key;
}

void unobfuscate(LCSENTRY& entry, unsigned long key) {
  entry.offset ^= key;
  entry.length ^= key;
  
  for (unsigned long i = 0; entry.filename[i]; i++) {
    entry.filename[i] ^= key;
  }
}

void unobfuscate(unsigned char* buff, unsigned long len) {  
  unsigned char  key = buff[3]; // Hopefully always :)
  unsigned char* end = buff + len;

  while (buff < end) {
    *buff++ ^= key;
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "exwlcs v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.lst> <input>\n", argv[0]);
    return -1;
  }

  string lst_filename(argv[1]);
  string dat_filename(argv[2]);

  int fd = as::open_or_die(lst_filename, O_RDONLY | O_BINARY);

  LCSHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long key = unobfuscate(hdr);  

  LCSENTRY* entries = new LCSENTRY[hdr.entry_count];
  read(fd, entries, sizeof(LCSENTRY) * hdr.entry_count);
  close(fd);  

  fd = as::open_or_die(dat_filename, O_RDONLY | O_BINARY);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unobfuscate(entries[i], key);

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    // Assume that any data we don't recognize is obfuscated 
    if (as::guess_file_extension(buff, len).empty()) {
      unobfuscate(buff, len);
    }

    as::write_file(entries[i].filename + as::guess_file_extension(buff, len), 
                   buff,
                   len);

    delete [] buff;
  }

  delete [] entries;

  return 0;
}
