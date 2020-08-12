// exdpmx.cpp, v1.11 2009/01/02
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from DPMX (*.dpm) archives.

#include "as-util.h"

struct DPMXHDR {
  unsigned char signature[4];
  unsigned long data_offset;
  unsigned long entry_count;
  unsigned long unknown1;
};

struct DPMXENTRY {
  char          filename[16];
  unsigned long unknown1; // 0xFFFFFFFF
  unsigned long seed;
  unsigned long offset;
  unsigned long length;
};

void unobfuscate(unsigned char* buff,
                 unsigned long  len,
                 as::val32_t    seed)
{
  // These could vary but seem to be constant for useful data
  unsigned char seed2    = 0xAA;
  unsigned char seed3    = 0x55;

  unsigned char seed1    = seed.as_byte[2] ^ (seed.as_byte[0] + 0x55);
  unsigned char seed4    = seed.as_byte[3] ^ (seed.as_byte[1] + 0xAA);  

  unsigned char mutator1 = (unsigned char) (seed1 + seed2);
  unsigned char mutator2 = (unsigned char) (seed3 + seed4);
  unsigned char key      = 0;

  unsigned char* end = buff + len;

  while (buff < end) {
    key += mutator1 ^ (*buff - mutator2);
    *buff++ = key;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exdpmx v1.11 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dpm|input.exe>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  
  DPMXHDR hdr;

  // Search for start of archive in executables
  while (true) {     
    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
      fprintf(stderr, "%s: does not contain a DPMX archive.\n", in_filename.c_str());
      return -1;
    }

    if (!memcmp(hdr.signature, "DPMX", 4) && 
        hdr.data_offset == hdr.entry_count * sizeof(DPMXENTRY) + sizeof(hdr)) {
      hdr.data_offset += tell(fd) - sizeof(hdr);
      break;
    }

    lseek(fd, 4 - (long)sizeof(hdr), SEEK_CUR);
  }

  DPMXENTRY* entries = new DPMXENTRY[hdr.entry_count];
  read(fd, entries, sizeof(DPMXENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, hdr.data_offset + entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (entries[i].seed) {
      unobfuscate(buff, len, entries[i].seed);
    }

    as::write_file(entries[i].filename, buff, len);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
