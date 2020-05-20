// exbbbin.cpp, v1.1 2008/06/06
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool cuts LZS compressed files from BunBun's BIN archives.
// I suppose there might be an actual TOC within the executable
// somewhere...

#include "as-util.h"
#include "lzss.h"

using std::string;

struct LZSHDR {
  unsigned char signature[4]; // "LZS\0"
  unsigned long original_length;
};

string guess_extension(unsigned char* buff, unsigned long len) {
  if (len >= 2 && !memcmp(buff, "TX", 2)) {
    return ".tx";
  }

  return as::guess_file_extension(buff, len);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exbbbin v1.1, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bin>\n", argv[0]);
    return -1;
  }

  string in_filename = argv[1];
  string prefix      = as::get_file_prefix(in_filename, true) + "_";

  unsigned char* base = NULL;
  unsigned long  len  = 0;
  as::memory_map_file(in_filename, base, len);

  unsigned char* p   = base;
  unsigned char* end = base + len;
  unsigned long  n   = 0;

  while (p + sizeof(LZSHDR) < end) {
    LZSHDR*        hdr  = (LZSHDR*) p;
    unsigned char* data = p + sizeof(*hdr);

    if (!memcmp(hdr->signature, "LZS\0", 4)) {
      unsigned long  out_len  = hdr->original_length;
      unsigned char* out_buff = new unsigned char[out_len];

      p = unlzss(data, out_buff, out_len);

      as::write_file(prefix + as::stringf("%05d", n++) + guess_extension(out_buff, out_len),
                     out_buff,
                     out_len);

      delete [] out_buff;
    } else {
      p++;
    }
  }

  return 0;
}