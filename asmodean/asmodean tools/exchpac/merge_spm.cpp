// merge_spm.cpp, v1.11 2013/01/22
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges images specified by an SPM VER-2.00 (*.spm).

#include "as-util.h"

#pragma pack(1)

struct SPMHDR {
  uint8_t  signature[13]; // "SPM VER-2.00\0"
  uint32_t entry_count;
};

struct SPMENTRYHDR {
  uint32_t entry_count;
  uint32_t width;
  uint32_t height;
  int32_t  base_x;
  int32_t  base_y;
  int32_t  base_cx;
  int32_t  base_cy;
  uint32_t unknown1;
  uint32_t unknown2;
  uint32_t unknown3;
  uint32_t unknown4;
};

struct SPMENTRY {
  uint32_t index;
  int32_t  dst_x;
  int32_t  dst_y;
  int32_t  dst_cx;
  int32_t  dst_cy;
  uint32_t width;
  uint32_t height;
  int32_t  src_x;
  int32_t  src_y;
  int32_t  src_cx;
  int32_t  src_cy;
  uint32_t unknown1;
  uint32_t unknown2; // 256
  uint32_t unknown3;
};

#pragma pack()

string get_filename(uint8_t* buff, uint32_t index) {
  uint32_t count = *(uint32_t*) buff;
  
  char* p = (char*) (buff + 4);

  for (uint32_t i = 0; i < index; i++) {
    p += strlen(p) + 1;
  }

  return p;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "merge_spm v1.11 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.spm>\n\n", argv[0]);
    return -1;
  }

  string in_filename = argv[1];
  string prefix      = as::get_file_prefix(in_filename);
  string path        = as::get_file_path(in_filename);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  uint32_t spm_len  = as::get_file_size(fd);
  auto     spm_buff = new uint8_t[spm_len];
  read(fd, spm_buff, spm_len);
  close(fd);

  auto p = spm_buff;

  auto hdr = (SPMHDR*) spm_buff;
  p += sizeof(*hdr);

  for (uint32_t i = 0; i < hdr->entry_count; i++) {
    auto entryhdr = (SPMENTRYHDR*) p;
    p += sizeof(*entryhdr) + sizeof(SPMENTRY) * entryhdr->entry_count;
  }

  auto filenames = p;

  p = (uint8_t*) (hdr + 1);

  typedef map<string, as::image_t> cache_t;
  cache_t cache;

  for (uint32_t i = 0; i < hdr->entry_count; i++) {
    auto entryhdr = (SPMENTRYHDR*) p;
    p += sizeof(*entryhdr);

    as::image_t image(entryhdr->width, entryhdr->height, 4);

    string out_prefix = prefix + as::stringf("+%03d", i);
    bool   missing    = false;

    for (uint32_t j = 0; j < entryhdr->entry_count; j++) {
      auto entry = (SPMENTRY*) p;
      p += sizeof(*entry);

      string filename = get_filename(filenames, entry->index);
      auto&  parts    = cache[filename];

      if (!parts.valid()) {
        string full_filename = path + filename;

        if (as::is_file_readable(full_filename)) {
          parts.open(full_filename);
        } else {
          fprintf(stderr, "%s: missing part %s\n", out_prefix.c_str(), filename.c_str());
          missing = true;
        }
      }

      if (parts.valid()) {
        auto chunk = parts.cut(entry->src_x, 
                               entry->src_y,
                               entry->width,
                               entry->height,
                               as::image_t::FLIP_OFFSET);

        chunk.set_offset(entry->dst_x, entry->dst_y);

        image.blend(chunk);      
      }
    }

    if (missing) {
      out_prefix += "+checkme";
    }

    image.trim();
    image.write(out_prefix + ".bmp");
  }

  delete [] spm_buff;

  return 0;
}
