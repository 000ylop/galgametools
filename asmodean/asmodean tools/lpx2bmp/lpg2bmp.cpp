// lpg2bmp.cpp, v1.0 2006/12/27
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts LPG images to 8-bit or 32-bit bitmaps.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

#pragma pack(1)
struct LPGHDR {
  unsigned long unknown1;
  unsigned long depth;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long width;
  unsigned long height;
  unsigned long length;
};
#pragma pack()

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "lpg2bmp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.lpg> <output.bmp>\n", argv[0]);
    return -1;
  }

  char* in_filename  = argv[1];
  char* out_filename = argv[2];

  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  LPGHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  RGBQUAD pal[256];
  read(fd, &pal, sizeof(pal));

  unsigned long  len  = hdr.length;
  unsigned char* buff = new unsigned char[len];

  read(fd, buff, len);
  close(fd);

  unsigned long pal_len = sizeof(pal);

  if (hdr.depth == 32) {
    unsigned long  temp_len  = hdr.width * hdr.height * hdr.depth / 8;
    unsigned char* temp_buff = new unsigned char[temp_len];
    RGBQUAD*       out_pixel = (RGBQUAD*) temp_buff;

    for (unsigned long i = 0; i < len; i += 2, out_pixel++) {
      *out_pixel             = pal[buff[i]];
      out_pixel->rgbReserved = buff[i + 1];
    }

    delete [] buff;

    len  = temp_len;
    buff = temp_buff;
  } else {
    // Not sure why they claim 24-bit for 8-bit images
    hdr.depth = 8;
  }

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
    bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi) + pal_len;

    bmi.biSize     = sizeof(bmi);
    bmi.biWidth    = hdr.width;
    bmi.biHeight   = hdr.height;
    bmi.biPlanes   = 1;
    bmi.biBitCount = (unsigned short) hdr.depth;

    write(fd, &bmf, sizeof(bmf));
    write(fd, &bmi, sizeof(bmi));
    write(fd, &pal, pal_len);
  }

  write(fd, buff, len);
  close(fd);

  delete [] buff;

  return 0;
}
