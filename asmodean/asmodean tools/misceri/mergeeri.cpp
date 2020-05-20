// mergeeri.cpp, v1.0 2012/10/26
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges Entis Rasterized Image (*.eri) deltas.
// Convert to PNG first using erisacvt or whatever.

#include "as-util.h"

struct ERIHDR {
  uint8_t signature[5]; // "Entis"
  uint8_t unknown1[11];
  uint8_t signature2[22]; // "Entis Rasterized Image"
  uint8_t unknown2[26];
};

struct ERICHUNK {
  uint8_t  signature[8];
  uint32_t length;
  uint32_t unknown;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "mergeeri v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.eri>\n", argv[0]);
    return -1;
  }

  string eri_filename = argv[1];

  int fd = as::open_or_die(eri_filename, O_RDONLY | O_BINARY);

  ERIHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  uint32_t descript_len  = 0;
  uint8_t* descript_buff = NULL;
  
  while (true) {
    ERICHUNK chunk;
    if (read(fd, &chunk, sizeof(chunk)) != sizeof(chunk)) {
      break;
    }

    if (!memcmp(chunk.signature, "Header  ", 8)) {
      uint32_t header_len  = chunk.length;
      auto     header_buff = new uint8_t[header_len];
      read(fd, header_buff, header_len);

      auto p   = header_buff;
      auto end = p + header_len;

      while (p < end) {
        auto chunk2 = (ERICHUNK*) p;
        p += sizeof(*chunk2);

        if (!memcmp(chunk2->signature, "descript", 8)) {
          descript_len  = chunk2->length;
          descript_buff = new uint8_t[descript_len + 2];
          memset(descript_buff, 0, descript_len + 2);
          memcpy(descript_buff, p, descript_len);
          break;
        } 

        p += chunk2->length;
      }

      delete [] header_buff;

      break;
    }

    lseek(fd, chunk.length, SEEK_CUR);
  }

  if (descript_buff == NULL) {
    fprintf(stderr, "%s: no descript tag found\n", eri_filename.c_str());
    return 1;
  }

  string description;
  if (!memcmp(descript_buff, "\xFF\xFE", 2)) {
    description = as::convert_wchar((wchar_t*)(descript_buff + 2));
  } else {
    description = (char*)descript_buff;
  }

  as::split_strings_t lines = as::splitstr(description, "\r\n");

  string reference_file;
  for (uint32_t i = 0; i < lines.size(); i++) {
    if (lines[i] == "#reference-file") {
      reference_file = lines[i + 1];
      break;
    }
  }

  if (reference_file.empty()) {
    return 0;
  }

  string base_filename = as::get_file_path(eri_filename) + as::get_file_prefix(reference_file) + ".png";
  string part_filename = as::get_file_prefix(eri_filename) + ".png";
  string out_filename  = "merged/" + as::get_file_prefix(eri_filename) + ".bmp";

  as::image_t base(base_filename);
  as::image_t part(part_filename);

  as::make_path(out_filename);

  base.blend_noresize(part, as::BLEND_BMP_DELTA_ADD);
  base.write(out_filename);

  close(fd);

  return 0;
}
