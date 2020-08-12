// merge_puregoddess.cpp, v1.08 2013/03/03
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges delta images from èÉåâÅöèóê_Ç≥Ç‹Ç¡ÅI and others.

#include "as-util.h"
#include <set>

struct SCRHDR {
  unsigned long length;
  unsigned long unknown1;
  unsigned long entry_count;
  unsigned long unknown2;
};

bool unobfuscate(unsigned char* buff, unsigned long len) {
  static const char          KEYS[][256] = { "VISHNU", "NANAMI", "artemina_daisuki", "SHIINAMACHI", "SAYAKA", "TUKIMI", "dltmes", "IORI" };
  static const unsigned long KEY_COUNT   = sizeof(KEYS) / sizeof(KEYS[0]);

  for (unsigned long i = 0; buff[0] && i < KEY_COUNT; i++) {
    unsigned long key_len = strlen(KEYS[i]);

    for (unsigned long j = 0; j < len; j++) {
      unsigned char c = buff[j] + (unsigned char)KEYS[i][(j + 16) % key_len];

      if (j == 0) {
        // This is pretty lame but I don't feel like redesigning for manual game selection
        unsigned char c2 = buff[j   + 1] + (unsigned char)KEYS[i][(j   + 1 + 16) % key_len];
        unsigned char c3 = buff[j   + 2] + (unsigned char)KEYS[i][(j   + 2 + 16) % key_len];
        unsigned char c4 = buff[len - 1] + (unsigned char)KEYS[i][(len - 1 + 16) % key_len];

        if (c != 0 || c2 != 0x23 || !isupper(c3) || c4 != 0) {
          break;
        }
      }

      buff[j] = c;
    }
  }

  return buff[0] == 0;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "merge_puregoddess v1.08 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input>\n\n", argv[0]);
    fprintf(stderr, "\t<input> should be _BGSET or _SPRSET extracted from md_scr.med.");
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  SCRHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = hdr.length;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  if (!unobfuscate(buff, len)) {
    fprintf(stderr, "%s: unsupported obfuscation (tell asmodean)\n", in_filename.c_str());
    return 1;
  }

  as::make_path("parts/");
  as::make_path("merged/");

  typedef std::set<string> used_t;
  used_t used;

  char* p = (char*) buff;

  for (unsigned long i = 0; i < hdr.entry_count;) {
    if (p[0] == '#') {
      string name = p + 1;
      p += strlen(p) + 1;
      i++;

      as::split_strings_t fields = as::splitstr(p, ",");
      p += strlen(p) + 1;
      i++;

      string base_filename = fields[0] + ".bmp";

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

      used.insert(base_filename);

      while (p[0] != '#') {
        fields = as::splitstr(p, ",");
        p += strlen(p) + 1;
        i++;

        string delta_filename = string(fields[0]) + ".bmp";
        long   offset_x       = 0;
        long   offset_y       = 0;

        if (fields.size() > 1) {
          offset_x = atol(fields[1].c_str());
          offset_y = atol(fields[2].c_str());
        }

        if (!as::is_file_readable(delta_filename)) {
          fprintf(stderr, "%s: delta not found (%s)\n", name.c_str(), delta_filename.c_str());
          continue;
        }

        if (offset_x < 0 || offset_y < 0) {
          fprintf(stderr, "%s: bad offset x=%d y=%d (%s)\n", name.c_str(), offset_x, offset_y, delta_filename.c_str());
          continue;
        }

        as::blend_bmp(delta_filename, 
                      offset_x,
                      offset_y,
                      base_buff,
                      base_len,
                      base_width,
                      base_height,
                      base_depth);

        used.insert(delta_filename);
      }

      as::write_bmp("merged/" + name + ".bmp",
                    base_buff,
                    base_len,
                    base_width,
                    base_height,
                    base_depth);

      delete [] base_buff;
    } else {
      p += strlen(p) + 1;
      i++;
    }
  }

  for (used_t::iterator i = used.begin();
       i != used.end();
       ++i) {
    rename(i->c_str(), ("parts/" + *i).c_str());
  }

  delete [] buff;

  close(fd);
  
  return 0;
}
