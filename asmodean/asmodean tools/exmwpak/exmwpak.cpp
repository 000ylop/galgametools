// exmwpak.cpp, v1.1 2011/02/17
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts magi white's *.PAK archives.

#include "as-util.h"
#include "zlib.h"

#define PAK_VERSION 2

struct PAKHDR {
  unsigned long unknown1;
  unsigned long entry_count;
  unsigned long original_toc_length;
  unsigned long toc_length;
  unsigned char unknown2[264];
};

struct PAKENTRY {
#if PAK_VERSION >= 2
  unsigned long unknown1;
#endif
  unsigned long offset;
  unsigned long length;
  unsigned long unknown2;
  unsigned char pad[8];
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmwpak, v1.1 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PAKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = hdr.toc_length;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  unsigned long  toc_len  = hdr.original_toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len]; 
  uncompress(toc_buff, &toc_len, buff, len);

  string         dir;
  unsigned long  data_base = tell(fd);
  unsigned char* p         = toc_buff;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long filename_len = *(unsigned long*) p;
    p += 4;

    string filename((char*)p, filename_len);
    p += filename_len;

    PAKENTRY* entry = (PAKENTRY*) p;
    p += sizeof(*entry);

    if (!entry->length) {
      dir = filename;
      continue;
    }

    string out_filename = dir + "/" + filename;

    as::make_path(out_filename);
    as::write_slice(fd, out_filename, data_base + entry->offset, entry->length);
  }

  delete [] toc_buff;
  delete [] buff;


  return 0;
}
