// merge_11ecfrag.cpp, v1.0 2009/11/15
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges tiled fragments used by 11eyes CrossOver (chara)

#include "as-util.h"

#include <algorithm>
#include <limits>
#include <set>

struct FRAGHDR {
  unsigned short entry1_count;
  unsigned short entry2_count; // ??
  unsigned short width;
  unsigned short height;
  unsigned short width2;
  unsigned short height2;
  unsigned long  unknown1;
};

struct FRAGENTRY1 {
  unsigned short unknown1;
  unsigned short unknown2;
  unsigned long  entry2_offset;
  unsigned short offset_x;
  unsigned short offset_y;
  unsigned short width_blocks;
  unsigned short height_blocks;
};

struct FRAGENTRY2 {
  unsigned char x;
  unsigned char y;
};

static const unsigned long BLOCK_WIDTH      = 32;
static const unsigned long BLOCK_HEIGHT     = 32;
static const unsigned long DST_BLOCK_WIDTH  = BLOCK_WIDTH  - 2;
static const unsigned long DST_BLOCK_HEIGHT = BLOCK_HEIGHT - 2;

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "merge_11ecfrag v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bmp> auto [merge index1] [merge index2] ...\n\n", argv[0]);
    fprintf(stderr, "usage: %s <input.bmp> <input> [merge index1] [merge index2] ...\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string dat_filename(argv[2]);
  string prefix(as::get_file_prefix(in_filename));

  typedef std::set<unsigned long> indexes_t;
  indexes_t indexes;

  for (int i = 3; i < argc; i++) {
    indexes.insert(atol(argv[i]));
  }

  bool do_merge = !indexes.empty();

  if (dat_filename == "auto") {
    dat_filename = as::stringf("%05d", atol(as::get_file_prefix(in_filename, true).c_str()) + 1);
  }

  int            fd       = as::open_or_die(dat_filename, O_RDONLY | O_BINARY);
  unsigned long  dat_len  = as::get_file_size(fd);
  unsigned char* dat_buff = new unsigned char[dat_len];
  read(fd, dat_buff, dat_len);
  close(fd);

  FRAGHDR*    hdr      = (FRAGHDR*) dat_buff;
  FRAGENTRY1* entries1 = (FRAGENTRY1*) (hdr + 1);

  unsigned long  src_len    = 0;
  unsigned char* src_buff   = NULL;
  unsigned long  src_width  = 0;
  unsigned long  src_height = 0;
  unsigned long  src_depth  = 0;
  unsigned long  src_stride = 0;

  as::read_bmp(in_filename,
               src_buff,
               src_len,
               src_width,
               src_height,
               src_depth,
               src_stride);

  {
    unsigned char* temp_buff = new unsigned char[src_len];

    for (unsigned long y = 0; y < src_height; y++) {
      unsigned char* dst_line = temp_buff + (src_height - y - 1) * src_stride;
      unsigned char* src_line = src_buff  + y * src_stride;

      memcpy(dst_line, src_line, src_stride);
    }

    delete [] src_buff;

    src_buff = temp_buff;
  } 

  unsigned short max_width_blocks  = 0;
  unsigned short max_height_blocks = 0;
  for (unsigned long i = 0; i < hdr->entry1_count; i++) {
    if (entries1[i].width_blocks && entries1[i].height_blocks) {
      max_width_blocks  = std::max<unsigned short>(entries1[i].offset_x + entries1[i].width_blocks, 
                                                   max_width_blocks);

      max_height_blocks = std::max<unsigned short>(entries1[i].offset_y + entries1[i].height_blocks, 
                                                   max_height_blocks);
    }   
  }

  unsigned long  out_width  = max_width_blocks  * DST_BLOCK_WIDTH;
  unsigned long  out_height = max_height_blocks * DST_BLOCK_HEIGHT;
  unsigned long  out_depth  = 4;
  unsigned long  out_stride = out_width * out_depth;
  unsigned long  out_len    = out_height * out_stride;
  unsigned char* out_buff   = new unsigned char[out_len];
  memset(out_buff, 0, out_len);

  for (unsigned long i = 0; i < hdr->entry1_count; i++) {    
    FRAGENTRY2* entry2 = (FRAGENTRY2*) (dat_buff + entries1[i].entry2_offset);

    if (do_merge && indexes.find(i) == indexes.end()) {
      continue;
    }   

    for (unsigned long y = 0; y < entries1[i].height_blocks; y++) {
      for (unsigned long x = 0; x < entries1[i].width_blocks; x++) {
        if (entry2->x != 0xFF || entry2->y != 0xFF) {
          for (unsigned long yy = 1; yy < BLOCK_HEIGHT; yy++) {
            if (y == entries1[i].height_blocks - 1 && yy == BLOCK_HEIGHT - 1) break;

            unsigned char* src_line = src_buff + ((entry2->y * BLOCK_HEIGHT) + yy) * src_stride;
            unsigned char* dst_line = out_buff + (((entries1[i].offset_y + y) * DST_BLOCK_HEIGHT) + yy - 1) * out_stride;

            for (unsigned long xx = 1; xx < BLOCK_WIDTH; xx++) {
              if (x == entries1[i].width_blocks - 1 && xx == BLOCK_WIDTH - 1) break;

              as::RGBA* src_pix = (as::RGBA*) (src_line + ((entry2->x * BLOCK_WIDTH) + xx) * src_depth);
              as::RGBA* dst_pix = (as::RGBA*) (dst_line + (((entries1[i].offset_x + x) * DST_BLOCK_WIDTH) + xx - 1) * out_depth);

              dst_pix->r = src_pix->r;
              dst_pix->g = src_pix->g;
              dst_pix->b = src_pix->b;
              dst_pix->a = src_pix->a;
            }
          }
        }

        entry2++;
      }
    }

    prefix += as::stringf("+%03d", i);

    indexes.erase(i);

    if (indexes.empty() || i == hdr->entry1_count - 1) {
      as::write_bmp(prefix + ".bmp",
                    out_buff,
                    out_len,
                    out_width,
                    out_height,
                    out_depth,
                    as::WRITE_BMP_FLIP);

      prefix = as::get_file_prefix(in_filename);
      memset(out_buff, 0, out_len);
    }
  }

  delete [] out_buff;
  delete [] src_buff;
  delete [] dat_buff;

  return 0;
}
