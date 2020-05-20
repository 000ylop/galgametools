// expf2.cpp, v1.0 2012/10/24
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts PF2 (*.pfs) archives.

#include "as-util.h"

#pragma pack(1)

struct PF2HDR {
  uint8_t  signature[3]; // "PF2"
  uint32_t toc_length;
  uint32_t unknown1;
  uint32_t entry_count;
};

struct PF2ENTRY1 {
  uint32_t filename_length;
};

struct PF2ENTRY2 {
  uint32_t unknown1;
  uint32_t unknown2;
  uint32_t unknown3;
  uint32_t offset;
  uint32_t length;
};

#pragma pack()

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "expf2 v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pfs>\n", argv[0]);
    return -1;
  }

  string in_filename = argv[1];

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY); 

  PF2HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  auto toc_buff = new uint8_t[hdr.toc_length];
  read(fd, toc_buff, hdr.toc_length);

  auto p = toc_buff;
  
  for (uint32_t i = 0; i < hdr.entry_count; i++) {
    auto entry1 = (PF2ENTRY1*) p;
    p += sizeof(*entry1);

    string filename((char*)p, entry1->filename_length);
    p += entry1->filename_length;

    auto entry2 = (PF2ENTRY2*) p;
    p += sizeof(*entry2);

    as::make_path(filename);
    as::write_slice(fd, filename, entry2->offset, entry2->length);
  }
  
  delete [] toc_buff;

  close(fd);

  return 0;
}
