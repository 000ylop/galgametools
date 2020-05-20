// hpal2bmp.cpp, v1.0 2012/07/26
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool inserts (replaces) an HPAL (*.hpl) palette into a bitmap.

#include "as-util.h"

struct HPALHDR {
  unsigned char signature[4]; // "HPAL"
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long pal_size;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long unknown5;
  unsigned long unknown6;

  void flip_endian(void) {
    as::flip_endian_multi(&pal_size,
                          NULL);
  }
};

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "hpal2bmp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.hpl> <input.bmp> [output.bmp]\n", argv[0]);
    return -1;
  }

  string hpal_filename = argv[1];  
  string bmp_filename  = argv[2];  
  string out_filename  = as::get_file_prefix(bmp_filename) + "+" + as::get_file_prefix(as::get_filename(hpal_filename)) + ".bmp";

  if (argc > 3) out_filename = argv[3];

  int fd = as::open_or_die(hpal_filename, O_RDONLY | O_BINARY);

  HPALHDR hdr;
  read(fd, &hdr, sizeof(hdr));
  hdr.flip_endian();

  unsigned long  pal_size  = hdr.pal_size;
  as::RGBA*      pal       = new as::RGBA[pal_size];
  read(fd, pal, pal_size * sizeof(as::RGBA));

  for (unsigned long i = 0; i < hdr.pal_size; i++) {
    std::swap(pal[i].r, pal[i].a);
    std::swap(pal[i].g, pal[i].b);
  }

  as::image_t bmp(bmp_filename);
  bmp.set_palette(pal, pal_size);
  bmp.write(out_filename);
  
  delete [] pal;

  close(fd);
  
  return 0;
}
