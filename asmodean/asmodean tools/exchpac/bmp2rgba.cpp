// bmp2rgba.cpp, v1.01 2007/03/24
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts a 24-bit bitmap plus a separate alpha channel into
// a single 32-bit RGBA bitmap.  Can be used with GIGA's *.fil alpha data
// among many others.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>

using std::string;

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(0);
  }

  return fd;
}

int main(int argc, char** argv) {
  if (argc < 4) {
    fprintf(stderr, "bmp2rgba, v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bmp> <input alpha file> <output.bmp> [-flip_alpha]\n\n", argv[0]);
    return -1;
  }

  string rgb_filename(argv[1]);
  string alp_filename(argv[2]);
  string rgba_filename(argv[3]);
  bool   flip_alpha = argc > 4 && !strcmp(argv[4], "-flip_alpha");
  
  int fd = open_or_die(rgb_filename, O_RDONLY | O_BINARY);

  BITMAPFILEHEADER bmf;
  read(fd, &bmf, sizeof(bmf));

  BITMAPINFOHEADER bmi;
  read(fd, &bmi, sizeof(bmi));

  if (bmi.biBitCount == 32) {
    fprintf(stderr, "%s: already 32-bit\n", rgb_filename.c_str());
    return 0;
  }

  RGBQUAD pal[256] = { 0 };

  if (bmi.biBitCount == 8) {        
    unsigned long pal_entries = bmi.biClrUsed == 0 ? 256 : bmi.biClrUsed;
    read(fd, pal, sizeof(RGBQUAD) * pal_entries);
  }

  unsigned long  stride   = (bmi.biWidth * bmi.biBitCount / 8 + 3) & ~3;
  unsigned long  rgb_len  = bmi.biHeight * stride;
  unsigned char* rgb_buff = new unsigned char[rgb_len];
  read(fd, rgb_buff, rgb_len);
  close(fd);

  unsigned long  alp_len    = bmi.biWidth * bmi.biHeight;
  unsigned char* alp_buff   = new unsigned char[alp_len + 1];

  fd = open_or_die(alp_filename, O_RDONLY | O_BINARY);
  unsigned long  alp_actual = read(fd, alp_buff, alp_len + 1);
  close(fd);

  if (alp_actual != alp_len) {
    fprintf(stderr, "%s: warning, alpha length mismatch (wanted %d, got %d)\n", 
            alp_filename.c_str(), alp_len, alp_actual);
    return 0;
  }

  unsigned long  rgba_len    = bmi.biWidth * bmi.biHeight * 4;
  unsigned char* rgba_buff   = new unsigned char[rgba_len];

  for (unsigned long y = 0; y < bmi.biHeight; y++) {
    unsigned char* rgb_line  = rgb_buff  + y * stride;
    unsigned char* alp_line  = NULL;
    unsigned char* rgba_line = rgba_buff + y * bmi.biWidth * 4;

    if (flip_alpha) {
      alp_line = alp_buff + (bmi.biHeight - y - 1) * bmi.biWidth;
    } else {
      alp_line = alp_buff + y * bmi.biWidth;
    }

    for (unsigned long x = 0; x < bmi.biWidth; x++) {
      if (bmi.biBitCount == 8) {
        rgba_line[x * 4 + 0] = pal[rgb_line[x]].rgbBlue;
        rgba_line[x * 4 + 1] = pal[rgb_line[x]].rgbGreen;
        rgba_line[x * 4 + 2] = pal[rgb_line[x]].rgbRed;
      } else {
        rgba_line[x * 4 + 0] = rgb_line[x * 3 + 0];
        rgba_line[x * 4 + 1] = rgb_line[x * 3 + 1];
        rgba_line[x * 4 + 2] = rgb_line[x * 3 + 2];
      }
      rgba_line[x * 4 + 3] = alp_line[x];
      //rgba_line[x * 4 + 3] = alp_line[x] == 32 ? 255 : alp_line[x] * 8;
    }
  }

  bmf.bfSize         = sizeof(bmf) + sizeof(bmi) + rgba_len;
  bmf.bfOffBits      = sizeof(bmf) + sizeof(bmi);
  bmi.biBitCount     = 32;
  bmi.biClrUsed      = 0;
  bmi.biClrImportant = 0;

  fd = open_or_die(rgba_filename,
                   O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                   S_IREAD | S_IWRITE);
  write(fd, &bmf, sizeof(bmf));
  write(fd, &bmi, sizeof(bmi));
  write(fd, rgba_buff, rgba_len);
  close(fd);

  delete [] rgba_buff;
  delete [] alp_buff;
  delete [] rgb_buff;

  return 0;
}
