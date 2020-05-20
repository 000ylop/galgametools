// exmarupac.cpp, v1.01 2008/06/27
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts files from *.PAC archives used by STRIKE's É}ÉãîÈêléñïîóΩêJâ€.
// Use pgd2tga for the graphics.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>

//#define PACV2

#pragma pack(1)
struct PACHDR {
  unsigned long entry_count;
  unsigned char junk[1018]; // ??
};

struct PACENTRY {
#ifdef PACV2
  unsigned char filename[32];
#else
  unsigned char filename[16];
#endif
  unsigned long length;
  unsigned long offset;
};
#pragma pack()

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmarupac v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pac>\n", argv[0]);
    return -1;
  }

  char*  in_filename = argv[1];
  
  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  PACHDR hdr;

  read(fd, &hdr, sizeof(hdr));

  PACENTRY* entries = new PACENTRY[hdr.entry_count];

  read(fd, entries, sizeof(PACENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];

    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    char filename[1024] = { 0 };
    memcpy(filename, entries[i].filename, sizeof(entries[i].filename));

    int out_fd = open(filename, 
                      O_CREAT | O_WRONLY | O_BINARY,
                      S_IREAD | S_IWRITE);

    if (out_fd == -1) {
      fprintf(stderr, "Could not open %s (%s)\n", filename, strerror(errno));
      return -1;
    }

    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  delete [] entries;
  
  close(fd); 

  return 0;
}
