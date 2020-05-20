// mag2bmp, v1.0 2006/12/25
// coded by asmodean

// contact: 
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)
//   icq:   55244079

// This tool converts BANANA shu-shu *.mag graphics to 32-bit bitmaps.
// I had to write this because Sage won't support alpha data :(

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include "lzss.h"

struct MAGHDR {
  unsigned long x;
  unsigned long y;
  unsigned long cx;
  unsigned long cy;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long total_width;
  unsigned long total_height;
  unsigned long mask_offset;
};

// I don't even know what to call this.  Is it supposed to make the data
// compress better, or just obfuscation?
void unwtf(unsigned char* buff, unsigned long width, unsigned long height) {
  for (unsigned long y = 0; y < height; y++) {
    if (y == 0) {
      for (unsigned long x = 1; x < width; x++) {
        buff[x * 3 + 0] += buff[(x - 1) * 3 + 0];
        buff[x * 3 + 1] += buff[(x - 1) * 3 + 1];
        buff[x * 3 + 2] += buff[(x - 1) * 3 + 2];
      }
    } else {
      unsigned char* line      = buff + y * width * 3;
      unsigned char* prev_line = buff + (y - 1) * width * 3;

      for (unsigned long x = 0; x < width; x++) {
        line[x * 3 + 0] += prev_line[x * 3 + 0];
        line[x * 3 + 1] += prev_line[x * 3 + 1];
        line[x * 3 + 2] += prev_line[x * 3 + 2];
      }
    }    
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "mag2bmp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.mag> <output.bmp>\n\n", argv[0]);
    return -1;
  }

  char* in_filename  = argv[1];
  char* out_filename = argv[2];

  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  MAGHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long width  = hdr.cx - hdr.x;
  unsigned long height = hdr.cy - hdr.y;

  unsigned long data_len;
  {
    struct stat file_stat;
    fstat(fd, &file_stat);
    data_len = file_stat.st_size - sizeof(hdr);
  }

  unsigned char* buff = new unsigned char[data_len];

  read(fd, buff, data_len);
  close(fd);

  unsigned long  out_len  = width * height * 3;
  unsigned char* out_buff = new unsigned char[out_len];
  memset(out_buff, 0, out_len);

  unsigned long rgb_len = hdr.mask_offset ? hdr.mask_offset : data_len;

  unsigned long actual_out = unlzss(buff, rgb_len, out_buff, out_len);

  unwtf(out_buff, width, height);

  unsigned long  msk_len  = 0;
  unsigned char* msk_buff = NULL;

  if (hdr.mask_offset) {
    msk_len  = hdr.total_width * hdr.total_height;
    msk_buff = new unsigned char[msk_len];
    memset(msk_buff, 0, msk_len);

    unsigned long actual_out = unlzss(buff + hdr.mask_offset, 
                                      data_len - hdr.mask_offset, 
                                      msk_buff,
                                      msk_len);
  }
 
  unsigned long  rgba_len  = hdr.total_width * hdr.total_height * 4;
  unsigned char* rgba_buff = new unsigned char[rgba_len];
  memset(rgba_buff, 0, rgba_len);

  // Bitmap is bottom-up, so have to compute the offset backwards
  unsigned long y_skip = hdr.total_height - hdr.cy;

  for (unsigned long y = 0; y < hdr.total_height; y++) {
    unsigned char* rgb_line  = out_buff + (y - y_skip) * width * 3;
    unsigned char* msk_line  = msk_buff + y * hdr.total_width;
    unsigned char* rgba_line = rgba_buff + y * hdr.total_width * 4;

    for (unsigned long x = 0; x < hdr.total_width; x++) {
      if (y >= y_skip && (y - y_skip) < height && x >= hdr.x && x < hdr.cx) {
        rgba_line[x * 4 + 0] = rgb_line[(x - hdr.x) * 3 + 0];
        rgba_line[x * 4 + 1] = rgb_line[(x - hdr.x) * 3 + 1];
        rgba_line[x * 4 + 2] = rgb_line[(x - hdr.x) * 3 + 2];
      }

      if (msk_buff) {
        rgba_line[x * 4 + 3] = msk_line[x];
      } else {
        rgba_line[x * 4 + 3] = 0xFF;
      }
    }
  }

  fd = open(out_filename, O_CREAT | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  {
    BITMAPFILEHEADER bmf;
    BITMAPINFOHEADER bmi;

    memset(&bmf, 0, sizeof(bmf));
    memset(&bmi, 0, sizeof(bmi));

    bmf.bfType     = 0x4D42;
    bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + rgba_len;
    bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi);

    bmi.biSize     = sizeof(bmi);
    bmi.biWidth    = hdr.total_width;
    bmi.biHeight   = hdr.total_height;
    bmi.biPlanes   = 1;
    bmi.biBitCount = 32;
    
    write(fd, &bmf, sizeof(bmf));
    write(fd, &bmi, sizeof(bmi));
  }

  write(fd, rgba_buff, rgba_len);
  close(fd);

  delete [] rgba_buff;
  delete [] msk_buff;
  delete [] out_buff;
  delete [] buff;

  return 0;
}
