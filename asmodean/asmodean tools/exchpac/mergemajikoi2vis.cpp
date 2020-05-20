// mergemajikoi2vis.cpp, v1.0 2012/01/26
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges the event images from ê^åïÇ≈éÑÇ…óˆÇµÇ»Ç≥Ç¢ÅIS.
// Extract visual.dat from config.pac.

#include "as-util.h"
#include "as-png.h"

struct DATENTRY1 {
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long unknown5;
  unsigned long unknown6;
  unsigned long unknown7;
  unsigned long unknown8;
};

struct DATENTRY2 {
  unsigned long offset_x;
  unsigned long offset_y;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "mergemajikoi2vis, v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <visual.dat>\n", argv[0]);
    return -1;
  }

  string dat_filename(argv[1]);
  
  int fd = as::open_or_die(dat_filename, O_RDONLY | O_BINARY);

  unsigned long  entries_len = as::get_file_size(fd);
  unsigned char* entries     = new unsigned char[entries_len];
  read(fd, entries, entries_len);
  close(fd);

  unsigned char* p   = entries;
  unsigned char* end = p + entries_len;

  unsigned long* unknown_count = (unsigned long*) p;
  p += sizeof(unsigned long) * (*unknown_count + 1);

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
      fprintf(stderr, "%s: not readable (skipped)\n", delta_filename.c_str());
      continue;
    }

    unsigned long  base_len    = 0;
    unsigned char* base_buff   = NULL;
    unsigned long  base_width  = 0;
    unsigned long  base_height = 0;
    unsigned long  base_depth  = 0;
    unsigned long  base_stride = 0;

    as::read_png(base_filename,
                 base_buff,
                 base_len,
                 base_width,
                 base_height,
                 base_depth,
                 base_stride);

    as::blend_png(delta_filename, 
                  entry2->offset_x,
                  entry2->offset_y,
                  base_buff,
                  base_len,
                  base_width,
                  base_height,
                  base_depth,
                  as::BLEND_BMP_FLIP_OFFSET);

    as::write_bmp(as::get_file_prefix(delta_filename) + ".bmp",
                  base_buff,
                  base_len,
                  base_width,
                  base_height,
                  base_depth,
                  as::WRITE_BMP_FLIP);

    delete [] base_buff;
  }

  delete [] entries;

  return 0;
}
