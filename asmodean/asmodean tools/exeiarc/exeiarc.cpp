// exeiarc.cpp, v1.0 2007/01/11
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from Eikyuu Loop's arc.dat+idx.dat archives.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>

struct IDXENTRY {
  char          filename[56];
  unsigned long offset;
  unsigned long length;
};

void unrle(unsigned char* buff, unsigned long len, unsigned char* out_buff, unsigned long out_len) {
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;

  while (buff < end) {
    unsigned char n = *buff++;    

    while (n--) {
      unsigned long copy_len = 4;

      if (out_buff + copy_len > out_end) {
        copy_len = out_end - out_buff;
      }

      memcpy(out_buff, buff, copy_len);
      out_buff += copy_len;
    }

    buff += 4;
  }
}

unsigned long flip_endian(unsigned long x) {
  return (x >> 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x << 24);
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "exeiarc v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <idx.dat> <arc.dat>\n", argv[0]);
    return -1;
  }

  char* idx_filename = argv[1];
  char* arc_filename = argv[2];
  
  int fd = open(idx_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", idx_filename, strerror(errno));
    return -1;
  }

  unsigned long idx_len;
  {
    struct stat file_stat;
    fstat(fd, &file_stat);
    idx_len = file_stat.st_size;
  }

  unsigned long entry_count = idx_len / sizeof(IDXENTRY);

  IDXENTRY* entries = new IDXENTRY[entry_count];

  read(fd, entries, idx_len);
  close(fd);

  fd = open(arc_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", arc_filename, strerror(errno));
    return -1;
  }

  for (unsigned long i = 0; i < entry_count; i++) {
    unsigned long  len    = entries[i].length;
    unsigned char* buff   = new unsigned char[len];
    unsigned long out_len = 0;

    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, &out_len, sizeof(out_len));
    read(fd, buff, len);

    // Why is it big endian?
    out_len = flip_endian(out_len);

    unsigned char* out_buff = new unsigned char[out_len];

    unrle(buff, len, out_buff, out_len);

    int out_fd = open(entries[i].filename, 
                      O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                      S_IREAD | S_IWRITE);

    if (out_fd == -1) {
      fprintf(stderr, "Could not open %s (%s)\n", entries[i].filename, strerror(errno));
      return -1;
    }

    write(out_fd, out_buff, out_len);
    close(out_fd);

    delete [] out_buff;
    delete [] buff;
  }

  close(fd);

  return 0;
}
