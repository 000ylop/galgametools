// fixtmmevt.cpp, v1.01 2008/05/29
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool fixes chopped up event images in Tamamo Studio's event images.

#include <windows.h>
#include "as-util.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "fixtmmevt v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bmp> [output.bmp]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename = in_filename;

  if (argc > 2) out_filename = argv[2];

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  BITMAPFILEHEADER bmf;
  read(fd, &bmf, sizeof(bmf));

  if (bmf.bfType != 0x4D42) {
    fprintf(stderr, "%s: not a bitmap (yes, I'm lazy to process PNGs...)\n", in_filename.c_str());
    return -1;
  }

  BITMAPINFOHEADER bmi;
  read(fd, &bmi, sizeof(bmi));

  if (bmi.biWidth != 1024 || bmi.biHeight != 512) {
    fprintf(stderr, "%s: not a 1024x512 texture, may not need fixed.\n", in_filename.c_str());
    return 0;
  }

  unsigned long  len  = bmi.biWidth * bmi.biHeight * 3;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  // Could compute from the actual dimensions, but not worth the effort.
  unsigned long base_width  = 800;
  unsigned long base_height = 512;  

  unsigned long frag_width  = 88;
  unsigned long frag_height = 400;

  unsigned long out_width   = 800;
  unsigned long out_height  = 600;

  unsigned long  out_len  = out_width * out_height * 3;
  unsigned char* out_buff = new unsigned char[out_len];
  memset(out_buff, 0, out_len);

  unsigned long src_stride = bmi.biWidth * 3;
  unsigned long dst_stride = out_width * 3;

  for (unsigned long y = 0; y < base_height; y++) {
    unsigned char* src = buff     + y * src_stride;
    unsigned char* dst = out_buff + (y + frag_width) * dst_stride;

    for (unsigned long x = 0; x < base_width; x++) {
      dst[x * 3 + 0] = src[x * 3 + 0];
      dst[x * 3 + 1] = src[x * 3 + 1];
      dst[x * 3 + 2] = src[x * 3 + 2];
    }
  }

  for (unsigned long n = 0; n < 2; n++) {
    for (unsigned long y = 0; y < frag_height; y++) {
      unsigned char* src = buff + (bmi.biHeight - y - 1) * src_stride + (base_width * 3) + (frag_width * 3 * n);

      for (unsigned long x = 0; x < frag_width; x++) {
        unsigned char* dst = out_buff + x * dst_stride + (frag_height * 3 * n);

        dst[y * 3 + 0] = src[x * 3 + 0];
        dst[y * 3 + 1] = src[x * 3 + 1];
        dst[y * 3 + 2] = src[x * 3 + 2];
      }
    }
  }

  as::write_bmp(out_filename, out_buff, out_len, out_width, out_height, 3);

  delete [] out_buff;
  delete [] buff;

  return 0;
}
