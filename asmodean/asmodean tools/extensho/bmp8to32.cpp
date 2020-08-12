// bmp8to32.cpp, v1.0 2007/01/20
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts 8-bit bitmaps to 32-bit.  The intention is to be used
// with images that have 32-bit palette entries (console games typically).

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "bmp8to32, v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bmp> <output.bmp> [-fixalpha]\n\n", argv[0]);
    fprintf(stderr, "\t-fixalpha = multiply alpha value by 2 (commonly needed)\n\n");
    return -1;
  }

  char* in_filename  = argv[1];
  char* out_filename = argv[2];
  bool  fix_alpha    = argc > 3 && !strcmp(argv[3], "-fixalpha");

  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  BITMAPFILEHEADER bmf;
  read(fd, &bmf, sizeof(bmf));

  BITMAPINFOHEADER bmi;
  read(fd, &bmi, sizeof(bmi));

  if (bmi.biBitCount != 8) {
    fprintf(stderr, "%s: not an 8-bit bitmap.\n", in_filename);
    return -1;
  }

  if (bmi.biClrUsed == 0)
    bmi.biClrUsed = 256;

  RGBQUAD pal[256];
  read(fd, pal, sizeof(RGBQUAD) * bmi.biClrUsed);

  unsigned long  stride = (bmi.biWidth + 3) & ~3;
  unsigned long  len    = bmi.biHeight * stride;
  unsigned char* buff   = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  unsigned long  out_len  = bmi.biWidth * bmi.biHeight * 4;
  unsigned char* out_buff = new unsigned char[out_len];

  long solid_count = 0;

  for (int y = 0; y < bmi.biHeight; y++) {
    unsigned char* src_line = buff                 + y * stride;
    RGBQUAD*       dst_line = (RGBQUAD*) (out_buff + y * bmi.biWidth * 4);

    for (int x = 0; x < bmi.biWidth; x++) {
      dst_line[x] = pal[src_line[x]];

      if (fix_alpha) {
        dst_line[x].rgbReserved = dst_line[x].rgbReserved == 128 ? 0xFF : (dst_line[x].rgbReserved * 2);
      }

      if (dst_line[x].rgbReserved == 0xFF) {
        solid_count++;
      }
    }
  }

  if (solid_count == bmi.biWidth * bmi.biHeight) {
    fprintf(stderr, "%s: all pixels are solid (not converting)\n", in_filename);
    return -1;
  }

  fd = open(out_filename, 
            O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
            S_IREAD | S_IWRITE);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", out_filename, strerror(errno));
    return -1;
  }

  bmf.bfSize         = sizeof(bmf) + sizeof(bmi) + out_len;
  bmf.bfOffBits      = sizeof(bmf) + sizeof(bmi);
  bmi.biSizeImage    = 0;
  bmi.biBitCount     = 32;
  bmi.biClrUsed      = 0;
  bmi.biClrImportant = 0;

  write(fd, &bmf, sizeof(bmf));
  write(fd, &bmi, sizeof(bmi));
  write(fd, out_buff, out_len);
  close(fd);

  delete [] out_buff;
  delete [] buff;

  return 0;
}
