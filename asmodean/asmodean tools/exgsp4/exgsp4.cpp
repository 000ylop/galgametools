// exgsp4, v1.0 2007/05/26
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts graphics from GsPack4, DataPack5, etc (*.pak) archives.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>
#include "lzss.h"

using std::string;

struct GSPACK4HDR {
  unsigned char signature[16];  // "GsPack4", "DataPack5", etc
  unsigned char signature2[16]; // "GsPackFile4", "MOMO", etc
  unsigned char unknown[28];
  unsigned long entry_count;
  unsigned char unknown2[1984];
};

struct GSPACK4BMPHDR {
  unsigned long unknown1;
  unsigned long length;
  unsigned long original_length;
  unsigned long header_length;
  unsigned long unknown2;
  unsigned long width;
  unsigned long height;
  unsigned long depth;
  unsigned char unknown3[84];
};

string get_file_prefix(const std::string& filename) {
  string temp(filename);

  string::size_type pos = temp.find_last_of(".");

  if (pos != string::npos) {
    temp = temp.substr(0, pos);
  }

  pos = temp.find_last_of("/\\");

  if (pos != string::npos) {
    temp = temp.substr(pos + 1);
  }

  return temp;
}

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exgsp4 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix(get_file_prefix(in_filename));

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  GSPACK4HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    GSPACK4BMPHDR bmphdr;
    read(fd, &bmphdr, sizeof(bmphdr));

    unsigned long  len  = bmphdr.length;
    unsigned char* buff = new unsigned char[len];
    read(fd, buff, len);

    if (bmphdr.length % 4) {
      lseek(fd, 4 - (bmphdr.length % 4), SEEK_CUR);
    }

    unsigned long  out_len  = bmphdr.original_length;
    unsigned char* out_buff = new unsigned char[out_len];
    unlzss(buff, len, out_buff, out_len);

    unsigned char* bmp_buff = new unsigned char[out_len];
    unsigned long  stride   = (bmphdr.width * bmphdr.depth / 8 + 3) & ~3;

    for (unsigned long y = 0; y < bmphdr.height; y++) {
      memcpy(bmp_buff + (bmphdr.height - y - 1) * stride,
             out_buff + y * stride,
             stride);
    }

    char filename[4096] = { 0 };
    sprintf(filename, "%s_%05d.bmp", prefix.c_str(), i);

    int out_fd = open_or_die(filename,
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);

    {
      BITMAPFILEHEADER bmf;
      BITMAPINFOHEADER bmi;

      memset(&bmf, 0, sizeof(bmf));
      memset(&bmi, 0, sizeof(bmi));

      bmf.bfType     = 0x4D42;
      bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + out_len;
      bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi);

      bmi.biSize     = sizeof(bmi);
      bmi.biWidth    = bmphdr.width;
      bmi.biHeight   = bmphdr.height;
      bmi.biPlanes   = 1;
      bmi.biBitCount = (WORD) bmphdr.depth;
    
      write(out_fd, &bmf, sizeof(bmf));
      write(out_fd, &bmi, sizeof(bmi));
    }

    write(out_fd, bmp_buff, out_len);
    close(out_fd);

    delete [] bmp_buff;
    delete [] out_buff;
    delete [] buff;
  }

  close(fd);

  return 0;
}
