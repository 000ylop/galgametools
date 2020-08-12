// exyatpkg.cpp, v1.0 2010/09/28
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.pkg archives used by Yatagarasu.

#include "as-util.h"

struct PKGHDR {
  unsigned long unknown1;
  unsigned long entry_count;
};

struct PKGENTRY {
  char          filename[128];
  unsigned long length;
  unsigned long offset;
};

void read_unobfuscate(int            fd, 
                      unsigned char* buff,
                      unsigned long  len,
                      unsigned char* key_buff, 
                      unsigned long  key_len)
{
  read(fd, buff, len);

  for (unsigned long i = 0; i < len; i++) {
    buff[i] ^= key_buff[i % key_len];
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exyatpkg v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pkg>\n\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  const unsigned long key_len  = 4;
  unsigned char key_buff[key_len] = { 0 };
  lseek(fd, sizeof(PKGHDR) + sizeof(PKGENTRY) - 12, SEEK_SET);
  read(fd, key_buff, key_len);

  PKGHDR hdr;
  lseek(fd, 0, SEEK_SET);
  read_unobfuscate(fd, (unsigned char*)&hdr, sizeof(hdr), key_buff, key_len);

  PKGENTRY* entries = new PKGENTRY[hdr.entry_count];
  read_unobfuscate(fd, (unsigned char*)entries, sizeof(PKGENTRY) * hdr.entry_count, key_buff, key_len);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read_unobfuscate(fd, buff, len, key_buff, key_len);

    as::write_file(entries[i].filename, buff, len);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
