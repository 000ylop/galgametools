// merge_11ecfrag2.cpp, v1.0 2009/11/16
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges tiled fragments used by 11eyes CrossOver (bg).
// Input is two bitmaps and a descriptor file.

#include "as-util.h"

#include <algorithm>
#include <limits>
#include <set>
#include <vector>

struct FRAGENTRY0 {
  unsigned short width_blocks;
  unsigned short height_blocks;
};

struct FRAGENTRY1 {
  unsigned short offset_x;
  unsigned short offset_y;
  unsigned short width_blocks;
  unsigned short height_blocks;
};

struct FRAGENTRY2 {
  unsigned char x;
  unsigned char y;
};

struct FRAGENTRY {
  unsigned short offset_x;
  unsigned short offset_y;
  unsigned short width_blocks;
  unsigned short height_blocks;
  FRAGENTRY2*    entry2;
};

typedef std::vector<FRAGENTRY> entries_t;

static const unsigned long BLOCK_WIDTH      = 16;
static const unsigned long BLOCK_HEIGHT     = 16;
static const unsigned long DST_BLOCK_WIDTH  = BLOCK_WIDTH  - 0;
static const unsigned long DST_BLOCK_HEIGHT = BLOCK_HEIGHT - 0;

int main(int argc, char** argv) {
  if (argc < 4) {
    fprintf(stderr, "merge_11ecfrag2 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s auto auto <input> [-cleanup] [merge index1] [merge index2] ...\n\n", argv[0]);
    fprintf(stderr, "usage: %s <input1.bmp> <input2.bmp> <input> [-cleanup] [merge index1] [merge index2] ...\n", argv[0]);
    return -1;
  }

  string in1_filename(argv[1]);
  string in2_filename(argv[2]);
  string dat_filename(argv[3]); 
  bool   do_cleanup = argc > 4 && !strcmp(argv[4], "-cleanup");

  string prefix(as::get_file_prefix(dat_filename));

  typedef std::set<unsigned long> indexes_t;
  indexes_t indexes;

  for (int i = do_cleanup ? 5 : 4; i < argc; i++) {
    indexes.insert(atol(argv[i]));
  }

  bool do_merge = !indexes.empty();

  if (in1_filename == "auto") {
    in1_filename = as::stringf("%05d.bmp", atol(as::get_file_prefix(dat_filename, true).c_str()) - 2);
  }

  if (in2_filename == "auto") {
    in2_filename = as::stringf("%05d.bmp", atol(as::get_file_prefix(dat_filename, true).c_str()) - 1);
  }

  int            fd       = as::open_or_die(dat_filename, O_RDONLY | O_BINARY);
  unsigned long  dat_len  = as::get_file_size(fd);
  unsigned char* dat_buff = new unsigned char[dat_len];  
  read(fd, dat_buff, dat_len);
  close(fd);

  entries_t entries;

  unsigned char* p = dat_buff;

  FRAGENTRY0* entry0 = (FRAGENTRY0*) dat_buff;
  p += sizeof(*entry0);

  FRAGENTRY entry;  

  entry.offset_x      = 0;
  entry.offset_y      = 0;
  entry.width_blocks  = entry0->width_blocks;
  entry.height_blocks = entry0->height_blocks;
  entry.entry2        = (FRAGENTRY2*) p;
  p += entry0->width_blocks * entry0->height_blocks * sizeof(FRAGENTRY2);

  entries.push_back(entry);

  {
    unsigned short entry_count = *(unsigned short*) p;
    p += 2;

    for (unsigned long i = 0; i < entry_count; i++) {
      unsigned long offset = *(unsigned short*) p;
      p += 2;

      offset *= 4;

      FRAGENTRY1* entry1 = (FRAGENTRY1*) (dat_buff + offset);      

      entry.offset_x      = entry1->offset_x;
      entry.offset_y      = entry1->offset_y;
      entry.width_blocks  = entry1->width_blocks;
      entry.height_blocks = entry1->height_blocks;
      entry.entry2        = (FRAGENTRY2*) (entry1 + 1);
      entries.push_back(entry);      
    }
  }
  
  unsigned long  src1_len    = 0;
  unsigned char* src1_buff   = NULL;
  unsigned long  src1_width  = 0;
  unsigned long  src1_height = 0;
  unsigned long  src1_depth  = 0;
  unsigned long  src1_stride = 0;

  as::read_bmp(in1_filename,
               src1_buff,
               src1_len,
               src1_width,
               src1_height,
               src1_depth,
               src1_stride);

  unsigned long  src2_len    = 0;
  unsigned char* src2_buff   = NULL;
  unsigned long  src2_width  = 0;
  unsigned long  src2_height = 0;
  unsigned long  src2_depth  = 0;
  unsigned long  src2_stride = 0;

  as::read_bmp(in2_filename,
               src2_buff,
               src2_len,
               src2_width,
               src2_height,
               src2_depth,
               src2_stride);
  
  unsigned long  src_len    = src1_len + src2_len;
  unsigned char* src_buff   = new unsigned char[src_len];
  unsigned long  src_width  = src1_width;
  unsigned long  src_height = src1_height + src2_height;
  unsigned long  src_depth  = src1_depth;
  unsigned long  src_stride = src1_stride;

  for (unsigned long y = 0; y < src_height; y++) {
    unsigned char* dst_line = src_buff  + (src_height - y - 1) * src_stride;
    unsigned char* src_line = src2_buff + y * src2_stride;

    if (y > src2_height - 1) {
      src_line = src1_buff + (y - src2_height) * src1_stride;
    }

    memcpy(dst_line, src_line, src_stride);
  }

  delete [] src2_buff;
  delete [] src1_buff;

  unsigned short max_width_blocks  = 0;
  unsigned short max_height_blocks = 0;
  for (unsigned long i = 0; i < entries.size(); i++) {
    if (entries[i].width_blocks && entries[i].height_blocks) {
      max_width_blocks  = std::max<unsigned short>(entries[i].offset_x + entries[i].width_blocks, 
                                                   max_width_blocks);

      max_height_blocks = std::max<unsigned short>(entries[i].offset_y + entries[i].height_blocks, 
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

  for (unsigned long i = 0; i < entries.size(); i++) {
    FRAGENTRY2* entry2 = entries[i].entry2;

    if (do_merge && indexes.find(i) == indexes.end()) {
      continue;
    }   

    for (unsigned long y = 0; y < entries[i].height_blocks; y++) {
      for (unsigned long x = 0; x < entries[i].width_blocks; x++) {
        if (entry2->x != 0xFF || entry2->y != 0xFF) {
          for (unsigned long yy = 0; yy < BLOCK_HEIGHT; yy++) {
            unsigned char* src_line = src_buff + ((entry2->y * BLOCK_HEIGHT) + yy) * src_stride;
            unsigned char* dst_line = out_buff + (((entries[i].offset_y + y) * DST_BLOCK_HEIGHT) + yy) * out_stride;

            for (unsigned long xx = 0; xx < BLOCK_WIDTH; xx++) {
              as::RGBA* src_pix = (as::RGBA*) (src_line + ((entry2->x * BLOCK_WIDTH) + xx) * src_depth);
              as::RGBA* dst_pix = (as::RGBA*) (dst_line + (((entries[i].offset_x + x) * DST_BLOCK_WIDTH) + xx) * out_depth);

              *dst_pix = *src_pix;
            }
          }
        }

        entry2++;
      }
    }

    prefix += as::stringf("+%03d", i);

    indexes.erase(i);

    if (indexes.empty() || i == entries.size() - 1) {
      as::write_bmp(prefix + ".bmp",
                    out_buff,
                    out_len,
                    out_width,
                    out_height,
                    out_depth,
                    as::WRITE_BMP_FLIP);

      prefix = as::get_file_prefix(dat_filename);
      memset(out_buff, 0, out_len);
    }
  }

  delete [] out_buff;
  delete [] src_buff;
  delete [] dat_buff;

  if (do_cleanup) {
    unlink(in1_filename.c_str());
    unlink(in2_filename.c_str());
    unlink(dat_filename.c_str());
  }

  return 0;
}
