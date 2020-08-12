// exkleinpak.cpp, v1.0 2009/02/07
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts PACK (*.pak) archives used by KLEIN.

#include "as-util.h"

struct PACKHDR {
  unsigned char signature[4]; // "PACK"
  unsigned long entry_count;
  unsigned long unknown1;
  unsigned long unknown2;
};

struct PACKENTRY {
  char          filename[64];
  unsigned long length;
  unsigned long original_length;
  unsigned long offset;
};

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "exkleinpak v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak>\n", argv[0]);
    return -1;
  }

  string filename(argv[1]);  

  int fd = as::open_or_die(filename, O_RDONLY | O_BINARY);

  PACKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  PACKENTRY* entries = new PACKENTRY[hdr.entry_count];
  read(fd, entries, sizeof(PACKENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    if (entries[i].length != entries[i].original_length) {
      printf("Entry %d (%s) is compressed.\n", i, entries[i].filename);
    }

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    as::write_file(entries[i].filename, buff, len);

    delete [] buff;
  }


  close(fd);

  return 0;
}

