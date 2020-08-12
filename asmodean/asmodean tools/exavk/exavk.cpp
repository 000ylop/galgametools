// exavk.cpp, v1.0 2008/04/13
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts images from AV King's cg.pak+cg.bin archive with
// the alpha channel...

#include "as-util.h"
#include "as-lzss.h"

struct HEDHDR {
  unsigned char signature[4]; // "hed"
  unsigned long entry_count;
};

struct HEDENTRY {
  unsigned long offset;
  unsigned long length;
};

struct HIPHDR {
  unsigned char signature[4]; // "hip"
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long depth;
  unsigned long parts_offset;
  unsigned long anim_offset;
};

struct HIZHDR {
  unsigned char signature[4]; // "hiz"
  unsigned long type;         // ??
  unsigned long width;
  unsigned long height;
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long length;
  unsigned long total_width;
  unsigned long total_height;
  unsigned long offset_x;
  unsigned long offset_y;
  unsigned long unknown3[8];
};

void process_hiz(const string& filename, unsigned char* buff) {
  HIZHDR*        hizhdr    = (HIZHDR*) buff;
  unsigned long  data_len  = hizhdr->length;
  unsigned char* data_buff = buff + sizeof(*hizhdr);

  // Occasionally the fields aren't obfuscated
  if (hizhdr->width & 0xAA000000) {
    hizhdr->width    ^= 0xAA5A5A5A;
    hizhdr->height   ^= 0xAC9326AF;
    hizhdr->unknown1 ^= 0x375A8437;
    hizhdr->unknown2 ^= 0x19615D6A;
    hizhdr->length   ^= 0x136a9326;
  }

  unsigned long  unlz_len  = hizhdr->width * hizhdr->height * 4;
  unsigned char* unlz_buff = new unsigned char[unlz_len];
  unsigned long  unlz_act  = as::unlzss(data_buff, hizhdr->length, unlz_buff, unlz_len);

  unsigned long  out_len  = hizhdr->width * hizhdr->height * 4;
  unsigned char* out_buff = new unsigned char[out_len];

  unsigned long channel_len = hizhdr->width * hizhdr->height;

  for (unsigned long y = 0; y < hizhdr->height; y++) {
    unsigned char* r_line    = unlz_buff + y * hizhdr->width;
    unsigned char* g_line    = r_line + channel_len;
    unsigned char* b_line    = g_line + channel_len;
    unsigned char* a_line    = b_line + channel_len;
    unsigned char* rgba_line = out_buff + (hizhdr->height - y - 1) * hizhdr->width * 4;

    for (unsigned long x = 0; x < hizhdr->width; x++) {
      rgba_line[x * 4 + 0] = r_line[x];
      rgba_line[x * 4 + 1] = g_line[x];
      rgba_line[x * 4 + 2] = b_line[x];
      rgba_line[x * 4 + 3] = a_line[x];
    }
  }

  as::write_bmp(filename, 
                out_buff,
                out_len,
                hizhdr->width,
                hizhdr->height,
                4);

  delete [] out_buff;
  delete [] unlz_buff;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "exavk v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <cg.hed> <cg.bin>\n", argv[0]);
    return -1;
  }

  string hed_filename(argv[1]);
  string bin_filename(argv[2]);

  string prefix = as::get_file_prefix(hed_filename, true);
  int    fd     = as::open_or_die(hed_filename, O_RDONLY | O_BINARY);

  HEDHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  HEDENTRY* entries = new HEDENTRY[hdr.entry_count];
  read(fd, entries, sizeof(HEDENTRY) * hdr.entry_count);
  close(fd);

  fd = as::open_or_die(bin_filename, O_RDONLY | O_BINARY);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    string name = as::stringf("%s_%05d", prefix.c_str(), i);

    HIPHDR* hiphdr = (HIPHDR*) buff;

    if (!memcmp(hiphdr->signature, "hip", 3)) {
      process_hiz(name + ".bmp", buff + sizeof(*hiphdr));

      if (hiphdr->parts_offset) {
        process_hiz(name + "_parts.bmp", buff + hiphdr->parts_offset);
      }

      if (hiphdr->anim_offset) {
        as::write_file(name + "_parts.anm",
                       buff + hiphdr->anim_offset,
                       len  - hiphdr->anim_offset);
      }
    } else {
      as::write_file(name + as::guess_file_extension(buff, len), buff, len);
    }

    delete [] buff;
  }

  close(fd);

  delete [] entries;

  return 0;
}
