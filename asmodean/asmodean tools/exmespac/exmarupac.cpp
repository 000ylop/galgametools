// exmarupac.cpp, v1.1 2009/08/14
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts files from *.PAC archives used by STRIKE's É}ÉãîÈêléñïîóΩêJâ€.
// Use pgd2tga for the graphics.

#include "as-util.h"

#define PAC_VERSION 3

#pragma pack(1)
struct PACHDR {
#if PAC_VERSION == 3
  unsigned char signature[4]; // "PAC"
  unsigned long unknown1;
  unsigned long entry_count;
  unsigned char junk[2040];
#else
  unsigned long entry_count;
  unsigned char junk[1018];
#endif
};

struct PACENTRY {
#if PAC_VERSION == 1
  unsigned char filename[16];
#else
  unsigned char filename[32];
#endif
  unsigned long length;
  unsigned long offset;
};
#pragma pack()

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmarupac v1.1 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pac>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PACHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  PACENTRY* entries = new PACENTRY[hdr.entry_count];
  read(fd, entries, sizeof(PACENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];

    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    char filename[1024] = { 0 };
    memcpy(filename, entries[i].filename, sizeof(entries[i].filename));

    as::write_file(filename, buff, len);

    delete [] buff;
  }

  delete [] entries;
  
  close(fd); 

  return 0;
}
