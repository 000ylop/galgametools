// exyox.cpp, v1.01 2009/10/23
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tools extracts and decompresses graphics from YOX (*.DAT) archives.

#include "as-util.h"
#include "zlib.h"

#define YOX_VERSION 2

struct YOXHDR {
  unsigned char signature[4]; // "YOX\0"
  unsigned long unknown1;
  unsigned long toc_offset;
  unsigned long entry_count;
};

struct YOXENTRY {
  unsigned long offset;
  unsigned long length;
#if YOX_VERSION >= 2
  unsigned long unknown1;
  unsigned long unknown2;
#endif
};

struct YOXDATAHDR {
  unsigned char signature[4]; // "YOX\0"
  unsigned long unknown1;
  unsigned long original_length;
  unsigned long unknown2;
};

unsigned char YOXDATASIG[] = { 'Y', 'O', 'X', 0x00 };

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exyox v1.01, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <archive.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix = as::get_file_prefix(in_filename, true);

  // Process the archive
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  YOXHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  YOXENTRY* entries = new YOXENTRY[hdr.entry_count];
  lseek(fd, hdr.toc_offset, SEEK_SET);
  read(fd, entries, sizeof(YOXENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len   = entries[i].length;
    unsigned char* buff  = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    YOXDATAHDR* datahdr = (YOXDATAHDR*) buff;;

    if (!memcmp(datahdr->signature, YOXDATASIG, sizeof(YOXDATASIG))) {
      unsigned long  out_len  = datahdr->original_length;
      unsigned char* out_buff = new unsigned char[out_len];
      uncompress(out_buff, &out_len, buff + sizeof(*datahdr), len - sizeof(*datahdr));

      delete [] buff;

      len  = out_len;
      buff = out_buff;
    }

    as::write_file(as::stringf("%s_%05d", prefix.c_str(), i) + as::guess_file_extension(buff, len),
                   buff,
                   len);

    delete [] buff;   
  }

  delete [] entries;

  close(fd);

  return 0;
}