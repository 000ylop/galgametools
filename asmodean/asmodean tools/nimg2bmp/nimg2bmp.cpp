// nimg2bmp.cpp, v1.0 2007/01/06
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts ”LŠÛ“°'s *.img graphics to bitmaps.
// All the examples I've seen are either PNG (use sigren) or 640x480x24 images
// compatible with this tool.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "nimg2bmp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.img> <output.bmp> [width] [height]\n", argv[0]);
    return -1;
  }

  char* in_filename    = argv[1];
  char* out_filename   = argv[2];

  unsigned long width  = 640;
  unsigned long height = 480;

  if (argc > 3) {
    width = atol(argv[3]);
  }

  if (argc > 4) {
    height = atol(argv[4]);
  }

  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  unsigned long  len  = width * height * 3;
  unsigned char* buff = new unsigned char[len];

  unsigned long read_bytes = read(fd, buff, len);
  close(fd);

  if (read_bytes == len) {
    fd = open(out_filename, O_CREAT | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE);

    if (fd == -1) {
      fprintf(stderr, "Could not open %s (%s)\n", out_filename, strerror(errno));
      return -1;
    }

    {
      BITMAPFILEHEADER bmf;
      BITMAPINFOHEADER bmi;

      memset(&bmf, 0, sizeof(bmf));
      memset(&bmi, 0, sizeof(bmi));

      bmf.bfType     = 0x4D42;
      bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + len;
      bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi);

      bmi.biSize     = sizeof(bmi);
      bmi.biWidth    = width;
      bmi.biHeight   = height;
      bmi.biPlanes   = 1;
      bmi.biBitCount = 24;
    
      write(fd, &bmf, sizeof(bmf));
      write(fd, &bmi, sizeof(bmi));
    }

    write(fd, buff, len);
    close(fd);
  } else {
    fprintf(stderr, 
            "%s: not enough data -- probably a PNG (use sigren)\n",
            in_filename);
  }

  delete [] buff;

  return 0;
}
