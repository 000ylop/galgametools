// ani2bmp.cpp, v1.01 2009/09/20
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts minori's *.ani parts to bitmaps.

#include "as-util.h"

#pragma pack (1)
struct ANIHDR {
  unsigned short unknown1;
  unsigned short entry_count;
  unsigned long  unknown2;
};

struct ANIENTRY {
  // Prefixed by null terminated name
  unsigned short width;
  unsigned short height;
  unsigned short depth;
  unsigned short offsetx;
  unsigned short offsety;
};
#pragma pack()

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "ani2bmp v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.ani>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  string prefix(as::get_file_prefix(in_filename));
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  ANIHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = as::get_file_size(fd);;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  unsigned char* p = buff;

  for (unsigned short i = 0; i < hdr.entry_count; i++) {
    char* name = (char*) p;
    p += strlen(name) + 1;

    ANIENTRY*      entry = (ANIENTRY*) p;
    unsigned char* data  = (unsigned char*) (entry + 1);

    unsigned long  out_stride = entry->width * entry->depth / 8;
    unsigned long  out_len    = entry->height * out_stride;

    as::write_bmp(as::stringf("%s+%03d+%s+x%dy%d.bmp", prefix.c_str(), i, name, entry->offsetx, entry->offsety),
                  data,
                  out_len,
                  entry->width,
                  entry->height,
                  entry->depth / 8,
                  as::WRITE_BMP_FLIP);

    p += sizeof(*entry) + out_len;
  }

  delete [] buff;

  close(fd);

  return 0;
}
