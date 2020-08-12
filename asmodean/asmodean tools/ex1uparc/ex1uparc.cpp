// ex1uparc.cpp, v1.0 2013/05/07
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts \x00ARC (*.arc) archives.

#include "as-util.h"
#include <memory>

struct ARCHDR {
  uint8_t  signature[4]; // "\x00ARC"
  uint32_t data_offset;
  uint32_t entry_count;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "ex1uparc v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.arc>\n", argv[0]);
    return -1;
  }

  string in_filename = argv[1];

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY); 

  ARCHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  uint32_t toc_len  = hdr.data_offset - sizeof(hdr);
  auto     toc_buff = new uint8_t[toc_len];
  read(fd, toc_buff, toc_len);

  auto     p      = toc_buff;
  uint32_t offset = hdr.data_offset;

  for (uint32_t i = 0; i < hdr.entry_count; i++) {
    uint32_t filename_len = *(uint32_t*) p;
    p += 4;

    wchar_t filename[1024] = { 0 };
    memcpy(filename, p, filename_len);
    p += filename_len;

    uint32_t len = *(uint32_t*) p;
    p += 4;

    string out_filename = as::convert_wchar(filename);

    as::make_path(out_filename);

    as::write_slice(fd,
                    out_filename,
                    offset,
                    len);

    offset += len;
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}
