// exaosv2.cpp, v1.0 2006/12/01
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)
//   icq:   55244079

// This tool extracts data from version 2 (post BLUE BOX) AOS archives.
// Use abm2bmp for the 32-bit graphics.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

#pragma pack(1)
struct AOSV2HDR {
  unsigned long unknown1;
  unsigned long data_offset;
  unsigned long toc_length;
  char          archive_name[261]; // weird size
};

struct AOSV2ENTRY {
  char          filename[32];
  unsigned long offset;
  unsigned long length;
};
#pragma pack()

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exaosv2 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <grp.aos>\n", argv[0]);
    return -1;
  }

  char* in_filename = argv[1];
  
  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  AOSV2HDR hdr;

  read(fd, &hdr, sizeof(hdr));

  unsigned long entry_count = hdr.toc_length / sizeof(AOSV2ENTRY);

  AOSV2ENTRY* entries = new AOSV2ENTRY[entry_count];

  read(fd, entries, hdr.toc_length);

  unsigned long base_offset = sizeof(hdr) + hdr.toc_length;

  for (unsigned long i = 0; i < entry_count; i++) {
    unsigned long   len  = entries[i].length;
    unsigned char*  buff = new unsigned char[len];

    lseek(fd, base_offset + entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    int out_fd = open(entries[i].filename, 
                      O_CREAT | O_WRONLY | O_BINARY,
                      S_IREAD | S_IWRITE);

    if (out_fd == -1) {
      fprintf(stderr, "Could not open %s (%s)\n", entries[i].filename, strerror(errno));
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
