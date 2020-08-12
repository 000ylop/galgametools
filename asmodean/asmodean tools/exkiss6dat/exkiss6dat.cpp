// exkiss6dat.cpp, v1.0 2009/11/29
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts CAPYBARA DAT 001 (*.DAT) archives.

#include "as-util.h"

static const unsigned long DATENTRY_COUNT = 65535;

struct DATENTRY {
  unsigned long offset;
  unsigned long length;
};

struct DATHDR {
  unsigned char signature[16]; // "CAPYBARA DAT 001"
  unsigned long fn_offset;
  unsigned long fn_length;
  DATENTRY      entries[DATENTRY_COUNT];
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exkiss6dat, v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  DATHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  fn_len  = hdr.fn_length;
  unsigned char* fn_buff = new unsigned char[fn_len];
  lseek(fd, hdr.fn_offset, SEEK_SET);
  read(fd, fn_buff, fn_len);

  unsigned char* p = fn_buff;
  
  for (unsigned long i = 0; i < DATENTRY_COUNT; i++) {
    char* filename = (char*) p;

    while (*p != 0x0D) p++;
    *p = '\0';
    p += 2;

    if (filename == string(":END")) {
      break;
    }

    if (hdr.entries[i].offset) {
      as::write_slice(fd, 
                      filename,
                      hdr.entries[i].offset,
                      hdr.entries[i].length);
    }
  }

  delete [] fn_buff;

  close(fd);

  return 0;
}
