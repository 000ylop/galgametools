// exbsa, v1.1 2010/08/29
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from newer BSS-Arc (*.bsa) archives.

#include "as-util.h"
#include <list>

// #define BSSARC_VERSION 1
#define BSSARC_VERSION 2

#pragma pack(1)
struct BSSARCHDR {
  unsigned char  signature[8]; // "BSS-Arc"
  unsigned short version;      // ??
  unsigned short entry_count;
  unsigned long  toc_offset;
};

struct BSSARCENTRY {
#if BSSARC_VERSION >= 2
  unsigned long filename_offset;
#else
  char          filename[32];
#endif
  unsigned long offset;
  unsigned long length;
};

struct BSSCOMPHDR {
  unsigned char  signature[16]; // "BSS-Composition"
  unsigned char  unknown1;
  unsigned long  entry_count;
  unsigned char  unknown2;
  unsigned short base_width;
  unsigned short base_height;
  unsigned char  unknown3[6];
};

struct BSSGRAPHDR {
  unsigned char  signature[16]; // "BSS-Graphics"
  unsigned char  unknown1[16];
  unsigned short offset_x;
  unsigned short offset_y;
  unsigned char  unknown2[4];
  unsigned short width;
  unsigned short height;
  unsigned char  unknown3;
  unsigned char  frame_count;
  unsigned char  unknown4[2];
  unsigned char  type; // 0 = 32-bit, 1 = 24-bit, 2 = 8-bit?
  unsigned char  unknown5[5];
  unsigned long  length;
  unsigned char  unknown6[6];  
};
#pragma pack()

void unrle(unsigned char* buff,
           unsigned long  len,
           unsigned char* out_buff,
           unsigned long  pixel_bytes)
{
  unsigned char* end     = buff + len;

  while (buff < end) {
    unsigned long n = *buff++;

    if (n & 0x80) {
      n = 1 - (char)n;

      while (n--) {
        *out_buff = *buff;
        out_buff += pixel_bytes;
      }

      buff++;
    } else {
      n++;

      while (n--) {
        *out_buff = *buff++;
        out_buff += pixel_bytes;
      }
    }
  }
}

bool process_graphic(const string&  prefix, 
                     unsigned char* buff)
{
  BSSGRAPHDR*    hdr  = (BSSGRAPHDR*) buff;
  unsigned char* data = (unsigned char*) (hdr + 1);

  if (memcmp(hdr->signature, "BSS-Graphics", 12)) {
    return false;
  }

  unsigned long pixel_bytes = 0;
  
  switch (hdr->type) {
  case 0:
    pixel_bytes = 4;
    break;
  case 1:
    pixel_bytes = 3;
    break;
  case 2:
    pixel_bytes = 1;
    break;
  default:
    return false;
  };

  unsigned long  out_len  = hdr->width * hdr->height * pixel_bytes;
  unsigned char* out_buff = new unsigned char[out_len];
  unsigned char* pal      = NULL;

  for (unsigned long i = 0; i < pixel_bytes; i++) {
    unsigned long data_len = *(unsigned long*) data;
    data += 4;
    unrle(data, data_len, out_buff + i, pixel_bytes);
    data += data_len;
  }

  if (pixel_bytes == 1) {
    pal = data;
  }

  string filename = prefix;

  if (hdr->offset_x || hdr->offset_y) {
    filename += as::stringf("+x%dy%d", hdr->offset_x, hdr->offset_y);
  }

  filename += ".bmp";

  as::write_bmp_ex(filename, 
                   out_buff,
                   out_len,
                   hdr->width,
                   hdr->height,
                   pixel_bytes,
                   256,
                   pal);

  delete [] out_buff;

  return true;
}

bool process_composition(const string&  prefix, 
                         unsigned char* buff) 
{
  BSSCOMPHDR*    hdr  = (BSSCOMPHDR*) buff;
  unsigned char* data = (unsigned char*) (hdr + 1);

  if (memcmp(hdr->signature, "BSS-Composition", 15)) {
    return false;
  }

  bool has_error = false;

  for (unsigned long i = 0; i < hdr->entry_count; i++) {
    if (!process_graphic(prefix + as::stringf("+%03d", i), data)) {
      fprintf(stderr, "%s: failed to extract entry %d\n", prefix.c_str(), i);
      has_error = true;
    }

    BSSGRAPHDR* grahdr = (BSSGRAPHDR*) data;
    data += sizeof(*grahdr) + grahdr->length;
  }

  return !has_error;
}

class directories_t {
public:
  void push(const string& d) {
    dirnames.push_back(d);
  }

  void pop(void) {
    dirnames.pop_back();
  }

  string get_path(const string& f) {
    string s;

    for (dirnames_t::iterator i = dirnames.begin();
         i != dirnames.end();
         i++) {
      s += *i + "/";
    }

    return s + f;
  }

private:
  typedef std::list<string> dirnames_t;
  dirnames_t dirnames;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exbsa v1.1 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bsa>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  BSSARCHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  BSSARCENTRY* entries = new BSSARCENTRY[hdr.entry_count];
  lseek(fd, hdr.toc_offset, SEEK_SET);
  read(fd, entries, sizeof(BSSARCENTRY) * hdr.entry_count);

#if BSSARC_VERSION >= 2
  unsigned long filenames_len  = as::get_file_size(fd) - tell(fd);
  char*         filenames_buff = new char[filenames_len];
  read(fd, filenames_buff, filenames_len);
#endif

  directories_t dirs;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
#if BSSARC_VERSION >= 2
    char* filename = filenames_buff + entries[i].filename_offset;
#else
    char* filename = entries[i].filename;
#endif

    switch (filename[0]) {
      case '>':
        dirs.push(filename + 1);
        break;

      case '<':
        dirs.pop();
        break;

      default:
        {
          unsigned long  len  = entries[i].length;
          unsigned char* buff = new unsigned char[len];
          lseek(fd, entries[i].offset, SEEK_SET);
          read(fd, buff, len);

          string full_filename = dirs.get_path(filename);

          as::make_path(full_filename);          

          if (!process_graphic(as::get_file_prefix(full_filename), buff) &&
              !process_composition(as::get_file_prefix(full_filename), buff))
          {
            as::write_file(full_filename, buff, len);
          }

          delete [] buff;
        }

    }
  }

#if BSSARC_VERSION >= 2
  delete [] filenames_buff;
#endif

  delete [] entries;

  return 0;
}
