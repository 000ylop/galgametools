// excdt.cpp, v1.01 2009/10/19
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from RK1 (*.cdt) archives.

#include <windows.h>
#include "as-util.h"
#include "as-lzss.h"

struct CDTTRL {
  unsigned char signature[4]; // "RK1\0"
  unsigned long entry_count;
  unsigned long toc_offset;
};

struct CDTENTRY {
  char          filename[16];
  unsigned long length;
  unsigned long original_length;
  unsigned long unknown;
  unsigned long offset;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "excdt v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.cdt>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  CDTTRL trl;
  lseek(fd, -(long)sizeof(trl), SEEK_END);
  read(fd, &trl, sizeof(trl));

  CDTENTRY* entries = new CDTENTRY[trl.entry_count];
  lseek(fd, trl.toc_offset, SEEK_SET);
  read(fd, entries, sizeof(CDTENTRY) * trl.entry_count);

  for (unsigned long i = 0; i < trl.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (entries[i].length != entries[i].original_length) {
      unsigned long  temp_len  = entries[i].original_length;
      unsigned char* temp_buff = new unsigned char[temp_len];
      as::unlzss(buff, len, temp_buff, temp_len);

      delete [] buff;

      len  = temp_len;
      buff = temp_buff;
    }

    BITMAPFILEHEADER* bmf = (BITMAPFILEHEADER*) buff;    

    // Some bitmaps are malformed
    if (bmf->bfType == 0x4D42 && bmf->bfSize != len) { 
      BITMAPINFOHEADER* bmi = (BITMAPINFOHEADER*) (bmf + 1);

      as::write_bmp(entries[i].filename,
                    buff + 66,
                    len  - 66,
                    bmi->biWidth,
                    bmi->biHeight,
                    bmi->biBitCount / 8);
    } else {
      as::write_file(entries[i].filename, buff, len);
    }

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}

