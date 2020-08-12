// exlfpim.cpp, v1.0 2009/12/19
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.pim archives.

#include "as-util.h"

struct PIMHDR {
  unsigned short entry_count;
};

struct PIMENTRY {
  unsigned long length;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exlfpim v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pim>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix(as::get_file_prefix(in_filename));

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PIMHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  hdr.entry_count = as::flip_endian_short(hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    PIMENTRY entry;
    read(fd, &entry, sizeof(entry));

    unsigned long  len  = as::flip_endian(entry.length);
    unsigned char* buff = new unsigned char[len];
    read(fd, buff, len);

    as::write_file(prefix + as::stringf("%03d", i) + as::guess_file_extension(buff, len),
                   buff,
                   len);

    delete [] buff;
  }

  close(fd);

  return 0;
}
