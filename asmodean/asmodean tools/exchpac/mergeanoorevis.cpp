// mergeanoorevis.cpp, v1.02 2013/01/21
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges the event images from îﬁèóÇÕÉIÉåÇ©ÇÁÇÕÇ»ÇÍÇ»Ç¢ and others.
// Extract visual.dat from config.pac.

#include "as-util.h"

struct DATENTRY1 {
  uint32_t unknown1;
  uint32_t unknown2;
  uint32_t unknown3;
  uint32_t unknown4;
  uint32_t unknown5;
  uint32_t unknown6;
  uint32_t unknown7;
  uint32_t unknown8;
};

struct DATENTRY2 {
  uint32_t offset_x;
  uint32_t offset_y;
  uint32_t width;
  uint32_t height;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "mergeanoorevis, v1.02 by asmodean\n\n");
    fprintf(stderr, "usage: %s <visual.dat>\n", argv[0]);
    return -1;
  }

  string dat_filename(argv[1]);
  
  int fd = as::open_or_die(dat_filename, O_RDONLY | O_BINARY);

  uint32_t entries_len = as::get_file_size(fd);
  auto     entries     = new uint8_t[entries_len];
  read(fd, entries, entries_len);
  close(fd);

  auto p   = entries;
  auto end = p + entries_len;

  uint32_t* unknown_count = (uint32_t*) p;
  p += sizeof(uint32_t) * (*unknown_count + 1);

  while (p < end) {
    DATENTRY1* entry1 = (DATENTRY1*) p;
    p += sizeof(*entry1);

    string base_filename = (char*) p;
    p += base_filename.length() + 1;

    string delta_filename = (char*) p;
    p += delta_filename.length() + 1;

    DATENTRY2* entry2 = (DATENTRY2*) p;
    p += sizeof(*entry2);

    if (delta_filename.empty()) {
      continue;
    }

    if (!as::is_file_readable(delta_filename)) {
      fprintf(stderr, "%s: delta not found (skipped)\n", delta_filename.c_str());
      continue;
    }
    
    as::image_t base(base_filename);

    as::image_t delta(delta_filename);
    delta.set_offset(entry2->offset_x, entry2->offset_y);
    delta.set_transparent(as::RGBA::BLACK);

    base.blend_noresize(delta);
    base.write(as::get_file_prefix(delta_filename) + ".bmp");
  }

  delete [] entries;

  return 0;
}
