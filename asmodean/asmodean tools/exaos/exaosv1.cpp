// exaosv1.cpp, v1.0 2006/12/01
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)
//   icq:   55244079

// This tool extracts data from version 1 (pre BLUE BOX) AOS archives.
// Use abm2bmp for the 32-bit graphics.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

struct AOSV1ENTRY {
  char          filename[16];
  unsigned long offset; // From current position
  unsigned long length;
  unsigned char unknown[8];
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exaosv1 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <grp.aos>\n", argv[0]);
    return -1;
  }

  char* in_filename = argv[1];
  
  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  for (unsigned long toc_offset = 0; true; toc_offset += sizeof(AOSV1ENTRY)) {
    AOSV1ENTRY entry;

    lseek(fd, toc_offset, SEEK_SET);
    read(fd, &entry, sizeof(entry));

    if (entry.filename[0] == '\0') {
      break;
    }

    unsigned long   len  = entry.length;
    unsigned char*  buff = new unsigned char[len];

    lseek(fd, entry.offset, SEEK_CUR);
    read(fd, buff, len);

    int out_fd = open(entry.filename, 
                      O_CREAT | O_WRONLY | O_BINARY,
                      S_IREAD | S_IWRITE);

    if (out_fd == -1) {
      fprintf(stderr, "Could not open %s (%s)\n", entry.filename, strerror(errno));
      return -1;
    }

    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;    
  }
  
  close(fd); 

  return 0;
}
