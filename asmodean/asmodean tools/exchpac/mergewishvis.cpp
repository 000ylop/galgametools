// mergehimevis.cpp, v1.0 2007/10/03
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges the event images delta's from GIGA's game Hime...
// Extract visual.dat from config.pac.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>

using std::string;

struct DATENTRY1 {
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long unknown5;
  unsigned long unknown6;
  unsigned long unknown7;
  unsigned long unknown8;
};

struct DATENTRY2 {
  unsigned long offset_x;
  unsigned long offset_y;
};

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

unsigned long get_file_size(int fd) {
  struct stat file_stat;
  fstat(fd, &file_stat);
  return file_stat.st_size;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "mergehimevis, v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <visual.dat>\n", argv[0]);
    return -1;
  }

  string dat_filename(argv[1]);
  
  int fd = open_or_die(dat_filename, O_RDONLY | O_BINARY);

  unsigned long  entries_len = get_file_size(fd);
  unsigned char* entries     = new unsigned char[entries_len];
  read(fd, entries, entries_len);
  close(fd);

  unsigned char* p   = entries;
  unsigned char* end = p + entries_len;

  unsigned long* unknown_count = (unsigned long*) p;
  p += sizeof(unsigned long) * (*unknown_count + 1);

  while (p < end) {
    DATENTRY1* entry1 = (DATENTRY1*) p;
    p += sizeof(*entry1);

    char* base_filename = (char*) p;
    p += strlen(base_filename) + 1;

    char* delta_filename = (char*) p;
    p += strlen(delta_filename) + 1;

    DATENTRY2* entry2 = (DATENTRY2*) p;
    p += sizeof(*entry2);

    if (!strlen(delta_filename)) {
      continue;
    }

    fd = open(base_filename, O_RDONLY | O_BINARY);

    if (fd == -1) {
      printf("could not open base %s\n", base_filename);
      continue;
    }
    
    BITMAPFILEHEADER base_bmf;
    read(fd, &base_bmf, sizeof(base_bmf));

    BITMAPINFOHEADER base_bmi;
    read(fd, &base_bmi, sizeof(base_bmi));
    
    unsigned long  base_stride = (base_bmi.biWidth * base_bmi.biBitCount / 8 + 3) & ~3;
    unsigned long  base_len    = base_bmi.biHeight * base_stride;
    unsigned char* base_buff   = new unsigned char[base_len];
    read(fd, base_buff, base_len);
    close(fd);

    fd = open_or_die(delta_filename, O_RDONLY | O_BINARY);

    if (fd == -1) {
      printf("could not open delta %s\n", delta_filename);
      delete [] base_buff;
      continue;
    }

    BITMAPFILEHEADER delta_bmf;
    read(fd, &delta_bmf, sizeof(delta_bmf));

    BITMAPINFOHEADER delta_bmi;
    read(fd, &delta_bmi, sizeof(delta_bmi));
    
    unsigned long  delta_stride = (delta_bmi.biWidth * delta_bmi.biBitCount / 8 + 3) & ~3;
    unsigned long  delta_len    = delta_bmi.biHeight * delta_stride;
    unsigned char* delta_buff   = new unsigned char[delta_len];
    read(fd, delta_buff, delta_len);
    close(fd);

    for (long y = 0; y < delta_bmi.biHeight; y++) {
      unsigned long vdelt = base_bmi.biHeight - (delta_bmi.biHeight + entry2->offset_y);
      unsigned char* dst = base_buff  + (vdelt + y) * base_stride;
      unsigned char* src = delta_buff + y * delta_stride;

      for (long x = 0; x < delta_bmi.biWidth; x++) {
        if (src[x * 3 + 0] != 0x00 ||
            src[x * 3 + 1] != 0x00 ||
            src[x * 3 + 2] != 0x00) {
          dst[(x + entry2->offset_x) * 3 + 0] = src[x * 3 + 0];
          dst[(x + entry2->offset_x) * 3 + 1] = src[x * 3 + 1];
          dst[(x + entry2->offset_x) * 3 + 2] = src[x * 3 + 2];
        }
      }
    }

    fd = open_or_die(delta_filename,
                     O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                     S_IREAD | S_IWRITE);
    write(fd, &base_bmf, sizeof(base_bmf));
    write(fd, &base_bmi, sizeof(base_bmi));
    write(fd, base_buff, base_len);
    close(fd);

    delete [] delta_buff;
    delete [] base_buff;
  }

  delete [] entries;

  return 0;
}
