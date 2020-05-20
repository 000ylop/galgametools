// exmaiarc.cpp, v1.0 2006/11/26
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)
//   icq:   55244079

// This tool extracts data from MAI (*.ARC) archives.
// Use xm2bmp for the graphics.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

struct MAIHDR {
  unsigned char signature[4]; // "MAI."
  unsigned long unknown1;
  unsigned long entry_count;
  unsigned long unknown2;
};

struct MAIENTRY {
  char          filename[16];
  unsigned long offset;
  unsigned long length;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmaiarc v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <graphic.arc>\n", argv[0]);
    return -1;
  }

  char* in_filename = argv[1];
  
  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  MAIHDR hdr;

  read(fd, &hdr, sizeof(hdr));

  MAIENTRY* entries = new MAIENTRY[hdr.entry_count];

  read(fd, entries, sizeof(MAIENTRY) * hdr.entry_count);

  for (unsigned int i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];

    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    char filename[1024] = { 0 };

    if (!memcmp(buff, "CM", 2)) {
      sprintf(filename, "%s.cm", entries[i].filename);
    } else if (!memcmp(buff, "AM", 2)) {
      sprintf(filename, "%s.am", entries[i].filename);
    } else {
      strcpy(filename, entries[i].filename);
    }

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
