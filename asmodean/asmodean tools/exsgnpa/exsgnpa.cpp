// exsgnpa.cpp, v1.0 2010/08/28
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.npa archives used by STEINS;GATE PC.

#include "as-util.h"

struct NPAHDR1 {
  unsigned long toc_length;
};

struct NPAHDR2 {
  unsigned long entry_count;
};

struct NPAENTRY1 {
  unsigned long filename_length;
  // unsigned char filename[filename_length];
};

struct NPAENTRY2 {
  unsigned long length;
  unsigned long offset;
  unsigned long unknown1;
};

void unobfuscate(unsigned char* buff, unsigned long len) {
  unsigned char KEY[] = { 0xBD, 0xAA, 0xBC, 0xB4, 0xAB, 0xB6, 0xBC, 0xB4 };

  for (unsigned long i = 0; i < len; i++) {
    buff[i] ^= KEY[i % sizeof(KEY)];
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exsgnpa v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.npa>\n\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  NPAHDR1 hdr1;
  read(fd, &hdr1, sizeof(hdr1));

  unsigned long  toc_len  = hdr1.toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);
  unobfuscate(toc_buff, toc_len);

  NPAHDR2*       hdr2 = (NPAHDR2*) toc_buff;
  unsigned char* p    = (unsigned char*) (hdr2 + 1);

  for (unsigned long i = 0; i < hdr2->entry_count; i++) {
    NPAENTRY1* entry1 = (NPAENTRY1*) p;
    p += sizeof(*entry1);

    string filename((char*) p, entry1->filename_length);
    p += entry1->filename_length;

    NPAENTRY2* entry2 = (NPAENTRY2*) p;
    p += sizeof(*entry2);

    unsigned long  len  = entry2->length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entry2->offset, SEEK_SET);
    read(fd, buff, len);
    unobfuscate(buff, len);

    as::make_path(filename);
    as::write_file(filename, buff, len);

    delete [] buff;
  }


  delete [] toc_buff;

  close(fd);

  return 0;
}
