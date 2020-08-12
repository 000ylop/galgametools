// exkizpak.cpp, v1.02 2011/12/21
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Extracts KCAP (*.PAK) archives used by ç≠Å|Ç´Ç∏Ç†Ç∆Å|.

#include <windows.h>
#include "as-util.h"
#include "as-lzss.h"

#define KCAP_VERSION 2

// Fix broken targa images in WHITE ALBUM 2..
#define FIX_TGA

struct KCAPHDR {
  unsigned char signature[4]; // "KCAP"
  unsigned long unknown1;
#if KCAP_VERSION >= 2
  unsigned long unknown2;
#endif
  unsigned long entry_count;
};

struct KCAPENTRY {
  unsigned long compressed_flag;
  char          filename[24];
#if KCAP_VERSION >= 2
  unsigned long unknown1;
  unsigned long unknown2;
#endif
  unsigned long offset;  
  unsigned long length;
};

struct DATAHDR {
  unsigned long length;
  unsigned long original_length;
};

struct TGAHDR {
  unsigned char  junk[12];
  unsigned short width;
  unsigned short height;
  unsigned char  bpp;
  unsigned char  flags;
};

void unbjr(unsigned char* buff, const string& filename) {
  BITMAPFILEHEADER* bmf = (BITMAPFILEHEADER*) buff;
  BITMAPINFOHEADER* bmi = (BITMAPINFOHEADER*) (bmf + 1);

  unsigned long  width     = bmi->biWidth;
  unsigned long  height    = abs(bmi->biHeight);
  unsigned long  depth     = bmi->biBitCount / 8;
  unsigned long  stride    = (width * depth + 3) & ~3;
  unsigned char* pixels    = buff + bmf->bfOffBits;

  unsigned long  temp_len  = height * stride;
  unsigned char* temp_buff = new unsigned char[temp_len];
  memcpy(temp_buff, pixels, temp_len);

  unsigned long key1 = 0x10000;
  unsigned long key2 = 0;
  unsigned long key3 = 0;

  for (unsigned long i = 1; i < filename.length() + 1; i++) {
    key1 -= filename[i - 1];
    key2 += filename[i - 1];
    key3 ^= filename[i - 1];
  }

  for (unsigned long y = 0; y < height; y++) {
    key3 += 7;

    unsigned char* src_line = temp_buff + (key3 % height) * stride;
    unsigned char* dst_line = pixels + y * stride;

    for (unsigned long x = 0; x < stride; x++) {
      if (x & 1) {
        dst_line[x] = (unsigned char) (0x10000 - key2 + src_line[x]);
      } else {
        dst_line[x] = (unsigned char) (0x100FF - key1 - src_line[x]);
      }
    }
  }

  delete [] temp_buff;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exkizpak v1.02, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  KCAPHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  if (!memcmp(hdr.signature, "LAC", 3)) {
    fprintf(stderr, "%s: not a KCAP archive - use exlac\n", in_filename.c_str());
    return 0;
  }

  if (memcmp(hdr.signature, "KCAP", 4)) {
    fprintf(stderr, "%s: not a KCAP archive (might be WMV video)\n", in_filename.c_str());
    return 0;
  }

  KCAPENTRY* entries = new KCAPENTRY[hdr.entry_count];
  read(fd, entries, sizeof(KCAPENTRY) * hdr.entry_count);

  string dir;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    if (!entries[i].length) {
      // This doesn't seem right ...
      //dir = entries[i].filename + string("/");
      continue;
    }

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];    
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);    

    if (entries[i].compressed_flag) {
      DATAHDR* datahdr = (DATAHDR*) buff;

      unsigned long  out_len  = datahdr->original_length;
      unsigned char* out_buff = new unsigned char[out_len];
      as::unlzss(buff + sizeof(*datahdr), len - sizeof(*datahdr), out_buff, out_len);

      delete [] buff;

      len  = out_len;
      buff = out_buff;
    }

    string out_name = dir + entries[i].filename;

    if (as::stringtol(out_name).find(".bjr") != string::npos) {
      unbjr(buff, entries[i].filename);
      out_name = as::get_file_prefix(out_name) + "+bjr" + as::guess_file_extension(buff, len);
    }

#ifdef FIX_TGA
    if (out_name.find(".tga") != string::npos) {
      TGAHDR* tgahdr = (TGAHDR*) buff;

      if (!tgahdr->bpp) {
        tgahdr->bpp = 32;
      }

      if (!tgahdr->flags && tgahdr->bpp == 32) {
        tgahdr->flags = 8;
      }
    }
#endif

    as::make_path(out_name);
    as::write_file(out_name, buff, len);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}