// agf2png.cpp, v1.01 2008/06/13
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts AGF images to bitmaps.

#include <windows.h>
#include "as-util.h"
#include "as-lzss.h"

struct AGFHDR {
  unsigned char signature[4]; // "", "ACGF"
  unsigned long type;
  unsigned long unknown;
};

struct ACIFHDR {
  unsigned char signature[4]; // "ACIF"
  unsigned long type;
  unsigned long unknown;
  unsigned long original_length;
  unsigned long width;
  unsigned long height;
};

static const unsigned long AGF_TYPE_24BIT = 1;
static const unsigned long AGF_TYPE_32BIT = 2;

struct AGFSECTHDR {
  unsigned long original_length;
  unsigned long original_length2; // why?
  unsigned long length;
};

void read_sect(int fd, unsigned char*& out_buff, unsigned long& out_len) {
  AGFSECTHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = hdr.length;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  out_len  = hdr.original_length;
  out_buff = new unsigned char[out_len];

  if (len == out_len) {
    memcpy(out_buff, buff, out_len);
  } else {
    as::unlzss(buff, len, out_buff, out_len);
  }

  delete [] buff;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "agf2png v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.agf> [output.bmp]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename = as::get_file_prefix(in_filename) + ".bmp";

  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  AGFHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  if (hdr.type != AGF_TYPE_24BIT && hdr.type != AGF_TYPE_32BIT) {
    fprintf(stderr, "%s: unsupported type (might be MPEG)\n", in_filename.c_str());
    return 0;
  }

  unsigned long  bmphdr_len  = 0;
  unsigned char* bmphdr_buff = NULL;
  read_sect(fd, bmphdr_buff, bmphdr_len);

  unsigned long  len  = 0;
  unsigned char* buff = NULL;
  read_sect(fd, buff, len);

  // Notice there's a gap of 2 bytes between these... alignment I guess.
  BITMAPFILEHEADER* bmf = (BITMAPFILEHEADER*) bmphdr_buff;
  BITMAPINFOHEADER* bmi = (BITMAPINFOHEADER*) (bmphdr_buff + 16);
  RGBQUAD*          pal = (RGBQUAD*)          (bmi + 1);

  if (hdr.type == AGF_TYPE_32BIT) {
    ACIFHDR acifhdr;
    read(fd, &acifhdr, sizeof(acifhdr));

    unsigned long  alpha_len  = 0;
    unsigned char* alpha_buff = NULL;  
    read_sect(fd, alpha_buff, alpha_len);

    unsigned long  rgba_len   = bmi->biWidth * bmi->biHeight * 4;
    unsigned char* rgba_buff  = new unsigned char[rgba_len];

    unsigned long  rgb_stride = (bmi->biWidth * bmi->biBitCount / 8 + 3) & ~3;

    for (long y = 0; y < bmi->biHeight; y++) {
      unsigned char* rgb_line  = buff       + y * rgb_stride;
      unsigned char* alp_line  = alpha_buff + (bmi->biHeight - y - 1) * bmi->biWidth;
      unsigned char* rgba_line = rgba_buff  + y * bmi->biWidth * 4;

      for (long x = 0; x < bmi->biWidth; x++) {
        if (bmi->biBitCount == 8) {
          rgba_line[x * 4 + 0] = pal[rgb_line[x]].rgbBlue;
          rgba_line[x * 4 + 1] = pal[rgb_line[x]].rgbGreen;
          rgba_line[x * 4 + 2] = pal[rgb_line[x]].rgbRed;
        } else {
          rgba_line[x * 4 + 0] = rgb_line[x * 3 + 0];
          rgba_line[x * 4 + 1] = rgb_line[x * 3 + 1];
          rgba_line[x * 4 + 2] = rgb_line[x * 3 + 2];
        }
        rgba_line[x * 4 + 3] = alp_line[x];
      }
    }

    as::write_bmp(out_filename,
                  rgba_buff,
                  rgba_len,
                  bmi->biWidth,
                  bmi->biHeight,
                  4);

    delete [] rgba_buff;
    delete [] alpha_buff;
  } else {
    as::write_bmp_ex(out_filename,
                     buff,
                     len,
                     bmi->biWidth,
                     bmi->biHeight,
                     bmi->biBitCount / 8,
                     bmi->biClrUsed,
                     pal);
  }

  delete [] buff;
  delete [] bmphdr_buff;

  close(fd);

  return 0;
}
