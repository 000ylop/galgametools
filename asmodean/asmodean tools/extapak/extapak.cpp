// extapak.cpp, v1.0 2010/10/14
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.pak archives used by íqë„ÉAÉtÉ^Å[ Å`It's a Wonderful LifeÅ` (XBOX360).

#include "as-util.h"

struct PAKHDR {
  unsigned long entry_count;
  unsigned long length;
};

struct PAKENTRY {
  unsigned long offset;
  unsigned long length;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "extapak v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix = as::get_file_prefix(in_filename, true);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PAKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  PAKENTRY* entries = new PAKENTRY[hdr.entry_count];
  read(fd, entries, sizeof(PAKENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    string filename = prefix + as::stringf("+%05d", i);

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    as::write_file(filename + as::guess_file_extension(buff, len), buff, len);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
