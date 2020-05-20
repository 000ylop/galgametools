// mergencpng.cpp, v1.0 2011/04/12
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges images used by non color.  Images should already be converted
// to raw bitmaps prior to using this.

#include "as-util.h"
#include <map>

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "mergencpng v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.png>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix = as::get_file_prefix(in_filename);
  string path   = as::get_file_path(in_filename);

  int            fd   = as::open_or_die(in_filename, O_RDONLY | O_BINARY); 
  unsigned long  len  = as::get_file_size(fd);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  unsigned char* p   = buff + 8;
  unsigned char* end = p + len;

  string        base_name;
  unsigned long offset_x = 0;
  unsigned long offset_y = 0;

  while (p < end) {
    unsigned long  tag_len = as::flip_endian(*(unsigned long*)p);
    p += 4;

    unsigned char* tag = p;
    p += 4;

    unsigned char* tag_buff = p;
    p += tag_len;
    p += 4;

    if (!memcmp(tag, "tEXt", 4)) {
      string field = (char*) tag_buff;
      string value((char*)tag_buff + field.length() + 1, tag_len - field.length() - 1);

      if (field == "Name") {
        base_name = value;
      } else if (field == "Left") {
        offset_x = atoi(value.c_str());
      } else if (field == "Top") {
        offset_y = atoi(value.c_str());
      }
    } else if (!memcmp(tag, "IEND", 4)) {
      break;
    }
  }

  if (!base_name.empty()) {
    unsigned long  base_len    = 0;
    unsigned char* base_buff   = NULL;
    unsigned long  base_width  = 0;
    unsigned long  base_height = 0;
    unsigned long  base_depth  = 0;
    unsigned long  base_stride = 0;

    as::read_bmp(path + as::get_file_prefix(base_name) + ".bmp",
                 base_buff,
                 base_len,
                 base_width,
                 base_height,
                 base_depth,
                 base_stride);

    unsigned long  delta_len    = 0;
    unsigned char* delta_buff   = NULL;
    unsigned long  delta_width  = 0;
    unsigned long  delta_height = 0;
    unsigned long  delta_depth  = 0;
    unsigned long  delta_stride = 0;

    as::read_bmp(prefix + ".bmp",
                 delta_buff,
                 delta_len,
                 delta_width,
                 delta_height,
                 delta_depth,
                 delta_stride);

    string out_filename = prefix + ".bmp";

    for (unsigned long y = 0; y < delta_height; y++) {
      unsigned char* base_line  = base_buff  + (y + base_height - delta_height - offset_y) * base_stride;
      unsigned char* delta_line = delta_buff + y * delta_stride;

      for (unsigned long x = 0; x < delta_width; x++) {
        as::RGBA* base_pixel  = (as::RGBA*) (base_line  + (x + offset_x) * base_depth);
        as::RGBA* delta_pixel = (as::RGBA*) (delta_line + x * delta_depth);

        if (delta_pixel->a) {
          base_pixel->r = delta_pixel->r;
          base_pixel->g = delta_pixel->g;
          base_pixel->b = delta_pixel->b;

          if (base_depth == 4) {
            base_pixel->a = delta_pixel->a;
          }
        }
      }
    }

    as::write_bmp(out_filename,
                  base_buff,
                  base_len,
                  base_width,
                  base_height,
                  base_depth);    

    delete [] delta_buff;
    delete [] base_buff;
  }

  delete [] buff;

  return 0;
}
