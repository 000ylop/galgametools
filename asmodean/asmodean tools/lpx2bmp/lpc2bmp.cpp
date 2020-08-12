// lpc2bmp.cpp, v1.01 2006/12/27
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts LPC composite images to 8-bit or 32-bit bitmaps.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>

#pragma pack(1)
struct LPCHDR {
  unsigned long unknown1;
  unsigned long entry_count;
  unsigned long length;
  unsigned long depth;
  unsigned long unknown2;
};

struct LPCENTRY {
  unsigned long width;
  unsigned long height;
  unsigned long length;
};
#pragma pack()

unsigned long unrle(unsigned char* buff, 
                    unsigned long  len, 
                    unsigned char* out_buff, 
                    unsigned long  out_len,
                    unsigned long  pixel_bytes) {
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;

  memset(out_buff, 0, out_len);

  while (buff < end && out_buff < out_end) {
    unsigned char c = *buff++;

    switch (c) {
    case 0x00:
      {
        unsigned char n = *buff++;
        out_buff += n * pixel_bytes;
      }
      break;

    default:
      // Assume there's only 1 or 2 byte pixels..
      if (pixel_bytes > 1) {
        // alpha byte
        *out_buff++ = *buff++;
      }

      // color byte
      *out_buff++ = c;
    }
  }

  return out_len - (out_buff - out_end);
}

using std::string;

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "lpc2bmp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.lpc>\n", argv[0]);
    return -1;
  }

  char* in_filename = argv[1];

  string prefix(in_filename);
  {
    string::size_type pos = prefix.find_last_of(".");

    if (pos) {
      prefix = prefix.substr(0, pos);
    }
  }

  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  LPCHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  RGBQUAD pal[256];
  read(fd, &pal, sizeof(pal));

  unsigned long offset_table[384];
  read(fd, offset_table, sizeof(offset_table));

  unsigned long data_base = sizeof(hdr) + sizeof(pal) + sizeof(offset_table);

  for (unsigned int i = 0; i < hdr.entry_count; i++) {
    LPCENTRY entry;
    lseek(fd, data_base + offset_table[i], SEEK_SET);
    read(fd, &entry, sizeof(entry));

    unsigned long  len  = entry.length;
    unsigned char* buff = new unsigned char[len];
    read(fd, buff, len);
    
    unsigned long  pixel_bytes = hdr.depth == 32 ? 2 : 1;
    unsigned long  out_len     = entry.width * entry.height * pixel_bytes;
    unsigned char* out_buff    = new unsigned char[out_len];

    unrle(buff, len, out_buff, out_len, pixel_bytes);

    unsigned long pal_len = sizeof(pal);

    if (hdr.depth == 32) {
      unsigned long  temp_len  = entry.width * entry.height * hdr.depth / 8;
      unsigned char* temp_buff = new unsigned char[temp_len];
      RGBQUAD*       out_pixel = (RGBQUAD*) temp_buff;      

      for (unsigned long i = 0; i < out_len; i += 2, out_pixel++) {
        *out_pixel             = pal[out_buff[i]];
        out_pixel->rgbReserved = out_buff[i + 1];
      }

      delete [] out_buff;

      out_len  = temp_len;
      out_buff = temp_buff;
    } else {
      // Not sure why they claim 24-bit for 8-bit images
      hdr.depth = 8;
    }
    
    unsigned char* flipped_buff = new unsigned char[out_len];
    unsigned long  stride       = entry.width * hdr.depth / 8;

    for (unsigned long y = 0; y < entry.height; y++) {
      memcpy(flipped_buff + (entry.height - y - 1) * stride,
             out_buff + y * stride,
             stride);
    }

    char filename[4096] = { 0 };
    sprintf(filename, "%s_%03d.bmp", prefix.c_str(), i);

    int out_fd = open(filename, O_CREAT | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE);

    if (out_fd == -1) {
      fprintf(stderr, "Could not open %s (%s)\n", filename, strerror(errno));
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
      bmi.biWidth    = entry.width;
      bmi.biHeight   = entry.height;
      bmi.biPlanes   = 1;
      bmi.biBitCount = (unsigned short) hdr.depth;

      write(out_fd, &bmf, sizeof(bmf));
      write(out_fd, &bmi, sizeof(bmi));
      write(out_fd, &pal, pal_len);
    }

    write(out_fd, flipped_buff, out_len);
    close(out_fd);

    delete [] flipped_buff;
    delete [] out_buff;
    delete [] buff;
  }

  return 0;
}
