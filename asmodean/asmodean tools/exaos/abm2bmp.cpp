// abm2bmp.cpp, v1.0 2006/12/01
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)
//   icq:   55244079

// This tool decompresses 32-bit ABM graphics.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

void unrle(unsigned char* buff, 
           unsigned long  len, 
           unsigned char* out_buff, 
           unsigned long  out_len,
           char*          filename) 
{
  unsigned char* end = buff + len;

  memset(out_buff, 0, out_len);

  while (buff < end) {
    unsigned char c = *buff++;

    switch (c) {
    case 0x00:
      {
        unsigned char n = *buff++;
        out_buff += (n / 3) * 4;
      }
      break;

    case 0xFF:
      {
        unsigned char n = *buff++;

        n /= 3;
        
        while (n--) {
          *out_buff++ = *buff++;
          *out_buff++ = *buff++;
          *out_buff++ = *buff++;
          *out_buff++ = 0xFF;
        }
      }
      break;

    default: 
      {
        *out_buff++          = *buff++;
        unsigned char alpha2 = *buff++;
        *out_buff++          = *buff++;
        unsigned char alpha3 = *buff++;
        *out_buff++          = *buff++;
        *out_buff++          = c;

        // Not sure why there's an alpha value for each color value in this
        // case.  What would it mean if they were different?
        if (alpha2 != c || alpha3 != c) {
          fprintf(stderr, "%s: warning alpha values differ on a pixel (%d, %d, %d)\n", 
                  filename, c, alpha2, alpha3);
        }
      }
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "abm2bmp, v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.abm> <output.bmp>\n\n", argv[0]);
    return -1;
  }

  char* in_filename  = argv[1];
  char* out_filename = argv[2];

  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  BITMAPFILEHEADER bmf;

  read(fd, &bmf, sizeof(bmf));

  BITMAPINFOHEADER bmi;

  read(fd, &bmi, sizeof(bmi));

  unsigned long len;
  {
    struct stat file_stat;
    fstat(fd, &file_stat);
    len = file_stat.st_size - sizeof(bmf) - sizeof(bmi);
  }

  unsigned char* buff = new unsigned char[len];

  read(fd, buff, len);

  close(fd);

  unsigned long  out_stride = bmi.biWidth * 4;
  unsigned long  out_len    = bmi.biHeight * out_stride;
  unsigned char* out_buff   = new unsigned char[out_len];

  unrle(buff, len, out_buff, out_len, in_filename);

  unsigned char* flipped_buff = new unsigned char[out_len];

  for (int y = 0; y < bmi.biHeight; y++) {
    memcpy(flipped_buff + (bmi.biHeight - y - 1) * out_stride,
           out_buff + y * out_stride,
           out_stride);
  }

  fd = open(out_filename, O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", out_filename, strerror(errno));
    return -1;
  }

  // Why is this set?
  bmi.biClrUsed = 0;

  write(fd, &bmf, sizeof(bmf));
  write(fd, &bmi, sizeof(bmi));
  write(fd, flipped_buff, out_len);
  close(fd);

  delete [] flipped_buff;
  delete [] out_buff;
  delete [] buff;

  return 0;
}
