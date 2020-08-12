// exnpf.cpp, v1.0 2008/01/09
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from PACK (*.npf) archives.

#include "as-util.h"

struct PACKHDR {
  unsigned char signature[4]; // "PACK"
  unsigned long entry_count;
  unsigned long unknown;
};

struct FATHDR {
  unsigned char signature[4]; // "FAT "
  unsigned long unknown1;
  unsigned long entry_count;
  unsigned long unknown2;
  unsigned long unknown3;
};

struct PACKENTRY {
  unsigned long filename_offset;
  unsigned long offset;
  long          key;
  unsigned long filename_length;
  unsigned long length;
};

void unobfuscate(unsigned char* buff, unsigned long len, long key) {
  long key1 = key;

  if (!key1) {
    key1 = 0x67895;
  }

  long key2 = ((key1 >> 12) ^ (key1 << 18)) - 0x579E2B8D;

  unsigned char* end = buff + len;

  while (buff < end) {
    long key3 = key2 + ((key1 >> 10) ^ (key1 << 14));    

    *buff++ ^= (unsigned char)key3 - 0x49 + (unsigned char)(key2 >> 12);

    key2 = key3 - 0x15633649 + ((key2 >> 12) ^ (key2 << 18));
  }
}

void read_unobfuscate(int fd, void* buff, unsigned long len, long key) {
  read(fd, buff, len);
  unobfuscate((unsigned char*) buff, len, key);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exnpf v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.npf>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int toc_fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  int dat_fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PACKHDR hdr;
  read(toc_fd, &hdr, sizeof(hdr));

  FATHDR fathdr;
  read_unobfuscate(toc_fd, &fathdr, sizeof(fathdr), 0x46415420);

  unsigned long entries_len = sizeof(PACKENTRY) * fathdr.entry_count;
  PACKENTRY*    entries     = new PACKENTRY[fathdr.entry_count];
  read_unobfuscate(toc_fd, entries, entries_len, fathdr.entry_count);

  for (unsigned long i = 0; i < fathdr.entry_count; i++) {
    unsigned long  filename_len = entries[i].filename_length;
    char*          filename     = new char[filename_len + 1];
    memset(filename, 0, filename_len + 1);
    read_unobfuscate(toc_fd, filename, filename_len, entries[i].key);

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(dat_fd, entries[i].offset, SEEK_SET);
    read_unobfuscate(dat_fd, buff, len, entries[i].key);    

    as::make_path(filename);

    int out_fd = as::open_or_die(filename,
                                 O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                                 S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
    delete [] filename;
  }

  close(dat_fd);
  close(toc_fd);

  return 0;
}
