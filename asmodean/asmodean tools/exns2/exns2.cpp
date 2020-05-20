// exns2.cpp, v1.0 2009/12/26
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.ns2 archives.  Use decrkansa first for encrypted
// archives.

#include "as-util.h"

struct NS2HDR {
  unsigned long data_offset;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exns2 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.ns2>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  NS2HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = hdr.data_offset - sizeof(hdr);
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);

  char* p = (char*) toc_buff;

  while (true) {
    if (*p++ != '"') break;

    char* filename = p;
    while (*p != '"') p++;
    *p++ = '\0';
    
    unsigned long  len  = *(unsigned long*) p;
    p += 4;

    unsigned char* buff = new unsigned char[len];
    read(fd, buff, len);

    as::make_path(filename);
    as::write_file(filename, buff, len);

    delete [] buff;
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}
