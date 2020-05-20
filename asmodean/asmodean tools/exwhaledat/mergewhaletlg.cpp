// mergewhaletlgt.cpp, v1.01 2010/12/29
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges images used by Whale.  Images should already be converted
// to raw bitmaps prior to using this.

#include "as-util.h"
#include <map>

struct taginfo_t {
  unsigned char* buff;
  unsigned long  len;

  unsigned long as_long(void) {
    switch (len) {
      // Not sure if this is a bug or what
      //case 0:
      //  return 0;

      case 1:
        return *(unsigned char*)buff;
        break;

      case 2:
        return *(unsigned short*)buff;
        break;

      case 4:
        return *(unsigned long*)buff;
        break;
    }

    return -1;
  }

  string as_string(void) {
    return string((char*)buff, len);
  }
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "mergewhaletlg v1.01, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.tlg>\n", argv[0]);
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

  unsigned char* p   = buff + len - 4;
  unsigned char* end = p - 384;

  while (p > end) {
    if (!memcmp(p, "tags", 4)) {
      p += 4;
      unsigned long  tags_len = *(unsigned long*) p;
      p += 4;

      unsigned char* tags_end = p + tags_len;

      typedef std::map<unsigned long, taginfo_t> tags_t;
      tags_t tags;

      while (p < tags_end) {
        string len_str;

        while (*p != ':') len_str += (char)*p++;
        p++; // ':'

        unsigned long id = *p++;

        p++; // '='

        len_str.clear();

        while (*p != ':') len_str += (char)*p++;
        p++; // ':'

        taginfo_t taginfo = { p, atol(len_str.c_str()) };

        p += taginfo.len;
        p++; // ','

        tags[id] = taginfo;
      }

      string        base_name = tags[1].as_string();
      unsigned long offset_x  = tags[2].as_long();
      unsigned long offset_y  = tags[3].as_long();

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

      if (offset_x == -1 || offset_y == -1) {
        if (offset_x == -1) offset_x = 0;
        if (offset_y == -1) offset_y = 0;

        out_filename = prefix + "+checkme.bmp";
      }

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


      break;
    } else {
      p--;
    }
  }


  return 0;
}
