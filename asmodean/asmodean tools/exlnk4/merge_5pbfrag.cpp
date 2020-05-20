// merge_5pbfrag.cpp, v1.01 2013/04/28
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges tiled fragments used by 5pb on the Xbox 360.

#include "as-util.h"

#include <algorithm>
#include <limits>
#include <set>

struct FRAGHDR {
  unsigned long entry1_count;
  unsigned long entry2_count;
};

struct FRAGENTRY1 {
  unsigned short unknown1;
  unsigned short unknown2;
  unsigned long  entry2_start;
  unsigned long  entry2_count;
};

struct FRAGENTRY2 {
  float dst_x;
  float dst_y;
  float src_x;
  float src_y;
};

static const unsigned long BLOCK_WIDTH  = 32;
static const unsigned long BLOCK_HEIGHT = 32;

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "merge_5pbfrag v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bmp> auto [merge index1] [merge index2] ...\n\n", argv[0]);
    fprintf(stderr, "usage: %s <input.bmp> <input.dat> [merge index1] [merge index2] ...\n", argv[0]);
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
    dat_filename = as::stringf("%05d", atol(prefix.c_str()) + 1);
  }

  int fd = as::open_or_die(dat_filename, O_RDONLY | O_BINARY);

  FRAGHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  hdr.entry1_count = as::flip_endian(hdr.entry1_count);
  hdr.entry2_count = as::flip_endian(hdr.entry2_count);

  FRAGENTRY1* entries1 = new FRAGENTRY1[hdr.entry1_count];
  read(fd, entries1, sizeof(FRAGENTRY1) * hdr.entry1_count);

  FRAGENTRY2* entries2 = new FRAGENTRY2[hdr.entry2_count];
  read(fd, entries2, sizeof(FRAGENTRY2) * hdr.entry2_count);
  close(fd);

  float min_x = std::numeric_limits<float>::max();
  float min_y = std::numeric_limits<float>::max();
  float max_x = std::numeric_limits<float>::min();
  float max_y = std::numeric_limits<float>::min();

  for (unsigned long i = 0; i < hdr.entry1_count; i++) {    
    entries1[i].entry2_start = as::flip_endian(entries1[i].entry2_start);
    entries1[i].entry2_count = as::flip_endian(entries1[i].entry2_count);

    for (unsigned long j = 0; j < entries1[i].entry2_count; j++) {
      FRAGENTRY2& entry2 = entries2[entries1[i].entry2_start + j];

      entry2.dst_x = as::flip_endian_float(entry2.dst_x);
      entry2.dst_y = as::flip_endian_float(entry2.dst_y);
      entry2.src_x = as::flip_endian_float(entry2.src_x);
      entry2.src_y = as::flip_endian_float(entry2.src_y);

      min_x = std::min(min_x, entry2.dst_x);
      min_y = std::min(min_y, entry2.dst_y);
      max_x = std::max(max_x, entry2.dst_x);
      max_y = std::max(max_y, entry2.dst_y);
    }
  }

  float offset_x = std::max(-min_x, 0.0f);
  float offset_y = std::max(-min_y, 0.0f);

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

  unsigned long  out_width  = (unsigned long) (offset_x + max_x + BLOCK_WIDTH);
  unsigned long  out_height = (unsigned long) (offset_y + max_y + BLOCK_HEIGHT);
  unsigned long  out_depth  = 4;
  unsigned long  out_stride = out_width * out_depth;
  unsigned long  out_len    = out_height * out_stride;
  unsigned char* out_buff   = new unsigned char[out_len];
  memset(out_buff, 0, out_len);

  for (unsigned long i = 0; i < hdr.entry1_count; i++) {    
    if (do_merge && indexes.find(i) == indexes.end()) {
      continue;
    }

    for (unsigned long j = 0; j < entries1[i].entry2_count; j++) {
      FRAGENTRY2& entry2 = entries2[entries1[i].entry2_start + j];

      entry2.dst_x += offset_x;
      entry2.dst_y += offset_y;

      entry2.src_x *= 2048;
      entry2.src_x -= 1;

      // This seems clunky...
      if (src_height <= 256) {
        entry2.src_y *= 256;
      } else if (src_height <= 512) {
        entry2.src_y *= 512;
      } else if (src_height <= 1024) {
        entry2.src_y *= 1024;
      } else {
        entry2.src_y *= 2048;
      }

      entry2.src_y -= 1;

      for (unsigned long y = 0; y < BLOCK_HEIGHT; y++) {
        unsigned char* src_line = src_buff + (unsigned long) (entry2.src_y + y) * src_stride;
        unsigned char* dst_line = out_buff + (unsigned long) (entry2.dst_y + y) * out_stride;

        for (unsigned long x = 0; x < BLOCK_WIDTH; x++) {
          as::RGBA* src_pix = (as::RGBA*) (src_line + (unsigned long) (entry2.src_x + x) * src_depth);
          as::RGBA* dst_pix = (as::RGBA*) (dst_line + (unsigned long) (entry2.dst_x + x) * out_depth);

          *dst_pix = *src_pix;
        }
      }
    }

    prefix += as::stringf("+%03d", i);

    indexes.erase(i);

    if (indexes.empty() || i == hdr.entry1_count - 1) {
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

  return 0;
}
