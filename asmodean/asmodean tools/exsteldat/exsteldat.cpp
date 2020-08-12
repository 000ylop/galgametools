// exsteldat.cpp, v1.01 2013/03/07
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from Rosebleu's NEKOPACK (*.dat) archives.

#include "as-util.h"

struct DATHDR {
  unsigned char signature[8];
  unsigned long unknown1; // hash for parity check?
  unsigned long unknown2;
};

struct DATDATAHDR {
  unsigned long seed;
  unsigned long length;
};

struct DATENTRYDIR {
  unsigned long name_hash;
  unsigned long entry_count;
};

struct DATENTRYFILE {
  unsigned long name_hash;
  unsigned long length;
};

void unobfuscate(unsigned char* buff,
                 unsigned long  len,
                 unsigned long  seed)
{
  unsigned long t1 = seed ^ (seed + 0x5D588B65);
  unsigned long t2 = t1   ^ (seed - 0x359D3E2A);

  unsigned long t3 = t2   ^ (t1   - 0x70E44324);
  unsigned long t4 = t3   ^ (t2   + 0x6C078965);

  unsigned long long key = ((unsigned long long)t4 << 32) | t3;;

  unsigned long long* p   = (unsigned long long*) buff;
  unsigned long long* end = p + (len / 8);

  while (p < end) {
    *p ^= key;

    // paddw
    unsigned short* ka = (unsigned short*) &key;
    unsigned short* pa = (unsigned short*) p;
    ka[0] += pa[0];
    ka[1] += pa[1];
    ka[2] += pa[2];
    ka[3] += pa[3];

    p++;
  }
}

void read_unobfuscate(int fd, unsigned char*& buff, unsigned long& len) {
  DATDATAHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  len                      = hdr.length;
  unsigned long padded_len = (len + 7) & ~7;
  buff                     = new unsigned char[padded_len];

  read(fd, buff, len);

  if (hdr.seed) {
    unobfuscate(buff, padded_len, hdr.seed);
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exsteldat v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  DATHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = 0;
  unsigned char* toc_buff = NULL;
  read_unobfuscate(fd, toc_buff, toc_len);

  unsigned char* toc_p   = toc_buff;
  unsigned char* toc_end = toc_buff + toc_len;

  for (unsigned long i = 0; toc_p < toc_end; i++) {
    DATENTRYDIR* dir = (DATENTRYDIR*) toc_p;
    toc_p += sizeof(*dir);

    for (unsigned long j = 0; j < dir->entry_count; j++) {
      DATENTRYFILE* file = (DATENTRYFILE*) toc_p;
      toc_p += sizeof(*file);

      unsigned long  len  = 0;
      unsigned char* buff = NULL;
      read_unobfuscate(fd, buff, len);

      char filename[4096] = { 0 };
      sprintf(filename, "%05d+%05d", i, j);

      int out_fd = as::open_or_die(filename + as::guess_file_extension(buff, len),
                                   O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                                   S_IREAD | S_IWRITE);
      write(out_fd, buff, len);
      close(out_fd);

      delete [] buff;
    }
  }

  delete [] toc_buff;

  return 0;
}
