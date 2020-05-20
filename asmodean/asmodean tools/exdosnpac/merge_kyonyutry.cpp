// merge_kyonyutry.cpp, v1.03 2012/07/28
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges delta images from ‹“ûƒgƒ‰ƒC.

#include "as-util.h"
#include <set>

#define VERSION 2

string get_next_line(unsigned char*& p) {
  unsigned short len = *(unsigned short*)p;
  p += 2;

  string line((char*)p, len);
  p += len;

  return line;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "merge_kyonyutry v1.03 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input> [-skip_missing]\n\n", argv[0]);
    fprintf(stderr, "\t<input> should be _filemacro from srp.pac.");
    return -1;
  }

  string in_filename(argv[1]);
  bool   skip_missing = false;

  if (argc > 2) {
    skip_missing = !strcmp(argv[2], "-skip_missing");
  }

#if VERSION >=2
  int            fd   = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  unsigned long  len  = as::get_file_size(fd);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  unsigned char* p    = buff;
  string         line = get_next_line(p);
#else
  FILE*  fh   = as::open_or_die_file(in_filename, "r");
  string line = as::read_line(fh);
#endif

  if (line != "[FILEMACRO]") {
    fprintf(stderr, "%s: unknown format\n", in_filename.c_str());
    return -1;
  }

  as::make_path("used/");
  as::make_path("merged/");

  typedef std::set<string> used_t;
  used_t used;

  while (true) {
#if VERSION >=2
    line = get_next_line(p);
#else
    line = as::read_line(fh);
#endif

    if (line == "[END]") {
      break;
    }

    as::split_strings_t fields = as::splitstr(line, "=");
    as::split_strings_t layers = as::splitstr(fields.at(1), "/");

    string name          = fields.at(0) + "=" + layers.at(0);
    string base_filename = layers.at(0) + ".bmp";

    if (!as::is_file_readable(base_filename)) {
      fprintf(stderr, "%s: base not found (%s)\n", name.c_str(), base_filename.c_str());
      continue;
    }
    
    unsigned long  base_len    = 0;
    unsigned char* base_buff   = NULL;
    unsigned long  base_width  = 0;
    unsigned long  base_height = 0;
    unsigned long  base_depth  = 0;
    unsigned long  base_stride = 0;

    as::read_bmp(base_filename,
                 base_buff,
                 base_len,
                 base_width,
                 base_height,
                 base_depth,
                 base_stride);

    bool missing = false;

    for (as::split_strings_t::iterator i = layers.begin() + 1;
         i != layers.end();
         ++i) {
      unsigned long offset_x       = 0;
      unsigned long offset_y       = 0;
      string        delta_filename = as::find_filename_with_xy(*i, &offset_x, &offset_y);

      if (!as::is_file_readable(delta_filename)) {
        fprintf(stderr, "%s: delta not found (%s)\n", name.c_str(), i->c_str());
        missing = true;
        continue;
      }

      name += "+" + *i;

      as::blend_bmp(delta_filename, 
                    offset_x,
                    offset_y,
                    base_buff,
                    base_len,
                    base_width,
                    base_height,
                    base_depth,
                    as::BLEND_BMP_FLIP_OFFSET);

      used.insert(delta_filename);
    }

    if (!missing || !skip_missing) {
      used.insert(base_filename);

      as::write_bmp("merged/" + name + ".bmp",
                    base_buff,
                    base_len,
                    base_width,
                    base_height,
                    base_depth);
    }

    delete [] base_buff;
  }

  for (used_t::iterator i = used.begin();
       i != used.end();
       ++i) {
    rename(i->c_str(), ("used/" + *i).c_str());
  }

#if VERSION >= 2
  close(fd);
#else
  fclose(fh);
#endif
  
  return 0;
}
