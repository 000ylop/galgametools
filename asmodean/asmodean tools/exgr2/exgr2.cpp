// exgr2.cpp, v1.0 2009/11/28
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts PACK (*.GR2) archvies.

#include "as-util.h"

struct PACKHDR {
  unsigned char signature[4]; // "PACK"
  unsigned long entry_count;
  unsigned long toc_length;
  unsigned long unknown1;
};

struct PACKENTRY {
  char          filename[16];
  unsigned long length;
  unsigned long offset;
};

struct GR2HDR {
  unsigned char  signature[4]; // "GR2_"
  unsigned short width;
  unsigned short height;
  unsigned long  unknown1;     // 0xFFFFFFFF
  unsigned long  pixel_bytes;  // hopefully?
};

unsigned long unrle(unsigned char* buff,
                    unsigned long  len,
                    unsigned char* out_buff,
                    unsigned long  out_len)
{
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;

  while (buff < end) {
    char n = (char) *buff++;

    if (n < 0) {
      n *= -1;

      while (n--) {
        *out_buff++ = *buff++;
      }
    } else {
      while (n--) {
        *out_buff++ = *buff;
      }

      buff++;
    }
  }

  return out_len - (out_end - out_buff);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exgr2, v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.gr2>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PACKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  PACKENTRY* entries = new PACKENTRY[hdr.entry_count];
  read(fd, entries, sizeof(PACKENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    read(fd, buff, len);

    if (!memcmp(buff, "LL5", 3)) {
      unsigned long  temp_len  = 16 * 1024 * 1024; // "big enough"
      unsigned char* temp_buff = new unsigned char[temp_len];
      temp_len = unrle(buff + 4, len - 4, temp_buff, temp_len);

      delete [] buff;

      len  = temp_len;
      buff = temp_buff;
    }

    GR2HDR* gr2hdr = (GR2HDR*) buff;

    if (!memcmp(gr2hdr->signature, "GR2", 3)) {
      as::write_bmp(as::get_file_prefix(entries[i].filename) + ".bmp",
                    buff + sizeof(*gr2hdr),
                    len  - sizeof(*gr2hdr),
                    gr2hdr->width,
                    gr2hdr->height,
                    gr2hdr->pixel_bytes,
                    as::WRITE_BMP_FLIP);  
    } else {
      as::write_file(entries[i].filename, buff, len);
    }

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
