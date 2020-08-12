// xm2bmp.cpp, v1.01 2007/03/31
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts CM and AM graphics to 24-bit and 32-bit bitmaps
// respectively.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <algorithm>

#pragma pack(1)
struct AMHDR {
  unsigned char  signature[2]; // "AM"
	unsigned long  length;
	unsigned short width;
	unsigned short height;
	unsigned short width2;
	unsigned short height2;
	unsigned char  unknown[4];
  unsigned short pal_length;
	unsigned char  depth;
	unsigned char  unknown2;
  unsigned char  alpha_type; // 1 = 0-15, 2 = 0-255
  unsigned char  unknown3;
	unsigned char  unknown4[2];
	unsigned long  rgb_offset;
	unsigned long  rgb_length;
	unsigned long  msk_offset;
	unsigned long  msk_length;
	unsigned char  unknown5[6];
};

static const unsigned long ALPHA_TYPE_SCALED  = 1;
static const unsigned long ALPHA_TYPE_LITERAL = 2;

struct CMHDR {
  unsigned char  signature[2]; // "CM"
  unsigned long  length;
  unsigned short width;
  unsigned short height;
  unsigned short pal_length;
  unsigned char  depth;
  unsigned char  unknown2[3];
  unsigned long  rgb_offset;
};

struct PALENTRY {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};
#pragma pack()

void unrle(unsigned char* data, DWORD len, unsigned char* decoded_data, int pixel_size) {
  static const unsigned char RLE_FLAG = 0x80;

	for (unsigned int i = 0, j = 0; i < len;) {
		int n = (int) (data[i] & ~RLE_FLAG);

		if (data[i] & RLE_FLAG) {
			for (int k = 0; k < n; k++) {
				memcpy(decoded_data + j, data + i + 1, pixel_size);
				j += pixel_size;
			}

			i += pixel_size + 1;
		} else {
			n *= pixel_size;

			memcpy(decoded_data + j, data + i + 1, n);
			i += n + 1;
			j += n;
		}
	}
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "xm2bmp v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.cm|input.am> <output.bmp>\n", argv[0]);
    return -1;
  }

  char* in_filename  = argv[1];
  char* out_filename = argv[2];
  
  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  unsigned long len        = 0;
  unsigned long width      = 0;
  unsigned long height     = 0;
  unsigned long depth      = 0;
  unsigned long pal_len    = 0;
  unsigned long rgb_offset = 0;
  unsigned long rgb_len    = 0;
  unsigned long msk_offset = 0;
  unsigned long msk_len    = 0;
  unsigned long alpha_type = 0;

  unsigned char sigcheck[2];
  read(fd, sigcheck, sizeof(sigcheck));
  lseek(fd, 0, SEEK_SET);

  if (!memcmp(sigcheck, "CM", 2)) {
    CMHDR hdr;
    read(fd, &hdr, sizeof(hdr));

    len        = hdr.length - sizeof(hdr);
    width      = hdr.width;
    height     = hdr.height;
    depth      = hdr.depth;
    pal_len    = hdr.pal_length;
    rgb_offset = hdr.rgb_offset - sizeof(hdr);
    rgb_len    = hdr.length - hdr.rgb_offset;
  } else if (!memcmp(sigcheck, "AM", 2)) {
    AMHDR hdr;
    read(fd, &hdr, sizeof(hdr));

    len        = hdr.length - sizeof(hdr);
    width      = hdr.width;
    height     = hdr.height;
    depth      = hdr.depth;
    alpha_type = hdr.alpha_type;
    pal_len    = hdr.pal_length;
    rgb_offset = hdr.rgb_offset - sizeof(hdr);
    rgb_len    = hdr.rgb_length;
    msk_offset = hdr.msk_offset - sizeof(hdr);
    msk_len    = hdr.msk_length;
  } else {
    fprintf(stderr, "%s: unknown type\n", in_filename);
    return -1;
  }

  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  PALENTRY*      pal      = (PALENTRY*) buff;
  unsigned char* rgb_buff = buff + rgb_offset;
  unsigned char* msk_buff = NULL;

  if (msk_len) {
    msk_buff = buff + msk_offset;
  }

  unsigned long  stride   = width * depth / 8;
  unsigned long  out_len  = height * stride;
  unsigned char* out_buff = new unsigned char[out_len];

  unrle(rgb_buff, rgb_len, out_buff, depth / 8);

  unsigned long out_depth = 0;

  if (msk_buff) {
    out_depth = 32;

    unsigned long  out_msk_len  = width * height;
    unsigned char* out_msk_buff = new unsigned char[out_msk_len];

    unrle(msk_buff, msk_len, out_msk_buff, 1);

    unsigned long  rgba_len  = width * height * 4;
    unsigned char* rgba_buff = new unsigned char[rgba_len];

    for (unsigned int y = 0; y < height; y++) {
      unsigned char* rgb_line  = out_buff     + y * stride;
      unsigned char* msk_line  = out_msk_buff + (height - y - 1) * width;
      unsigned char* rgba_line = rgba_buff    + y * width * 4;

      for (unsigned int x = 0; x < width; x++) {
        if (depth == 8) {
          rgba_line[x * 4 + 0] = pal[rgb_line[x]].red;
          rgba_line[x * 4 + 1] = pal[rgb_line[x]].green;
          rgba_line[x * 4 + 2] = pal[rgb_line[x]].blue;
        } else {
          rgba_line[x * 4 + 0] = rgb_line[x * 3 + 0];
          rgba_line[x * 4 + 1] = rgb_line[x * 3 + 1];
          rgba_line[x * 4 + 2] = rgb_line[x * 3 + 2];
        }

        if (alpha_type == ALPHA_TYPE_SCALED) {
          rgba_line[x * 4 + 3] = msk_line[x] * 16;

          if ((rgba_line[x * 4 + 0] != 0x00 ||
               rgba_line[x * 4 + 1] != 0x00 ||
               rgba_line[x * 4 + 2] != 0x33) &&
              rgba_line[x * 4 + 3] == 0x00) {
            rgba_line[x * 4 + 3] = 0xFF;
          }        
        } else {
          rgba_line[x * 4 + 3] = msk_line[x];
        }
      }
    }

    std::swap(out_buff, rgba_buff);
    std::swap(out_len, rgba_len);

    delete [] rgba_buff;
    delete [] out_msk_buff;
  } else {
    out_depth = depth;

    // Make sure the output is 4-byte aligned
    unsigned long  temp_stride = (width * depth / 8 + 3) & ~3;
    unsigned long  temp_len    = height * temp_stride;
    unsigned char* temp_buff   = new unsigned char[temp_len];

    for (unsigned long y = 0; y < height; y++) {
      memcpy(temp_buff + y * temp_stride,
             out_buff + y * stride,
             stride);
    }

    std::swap(out_buff, temp_buff);
    std::swap(out_len, temp_len);

    delete [] temp_buff;
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
    bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + out_len;
    bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi) + sizeof(RGBQUAD) * pal_len;

    bmi.biSize     = sizeof(bmi);
    bmi.biWidth    = width;
    bmi.biHeight   = height;
    bmi.biPlanes   = 1;
    bmi.biBitCount = out_depth;
    
    write(fd, &bmf, sizeof(bmf));
    write(fd, &bmi, sizeof(bmi));

    for (unsigned int i = 0; i < pal_len; i++) {
      RGBQUAD pal_entry;

      pal_entry.rgbRed      = pal[i].blue;
      pal_entry.rgbGreen    = pal[i].green;
      pal_entry.rgbBlue     = pal[i].red;
      pal_entry.rgbReserved = 0;

      write(fd, &pal_entry, sizeof(pal_entry));
    }
  }

  write(fd, out_buff, out_len);
  close(fd);

  delete [] out_buff;
  delete [] buff;
  
  return 0;
}
