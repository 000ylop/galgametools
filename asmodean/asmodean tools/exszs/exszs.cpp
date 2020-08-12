// exszs.cpp, v1.01 2012/05/29
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts SZS (*.szs) archives.

#include "as-util.h"

struct SZSHDR {
  unsigned char signature[8]; // "SZS100__", "SZS120__"
  unsigned long unknown1;
  unsigned long entry_count;
};

struct SZSENTRY {
  char          filename[256];
  unsigned long long offset;
  unsigned long long length;
};

void unobfuscate(unsigned char* buff, unsigned long len) {
  unsigned char key = 0x90;

  unsigned char* end = buff + len;

  while (buff < end) {
    *buff++ ^= key;
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "exszs v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.szs> [-prefix]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix    = as::get_file_prefix(in_filename);
  bool   do_prefix = false;

  if (argc > 2) {
    do_prefix = !strcmp(argv[2], "-prefix");
  }

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  SZSHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  if (memcmp(hdr.signature, "SZS", 3)) {
    fprintf(stderr, "%s: not an SZS archive (might be mpeg).\n", in_filename.c_str());
    return 0;
  }

  SZSENTRY* entries = new SZSENTRY[hdr.entry_count];
  read(fd, entries, sizeof(SZSENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = (unsigned long)entries[i].length;
    unsigned char* buff = new unsigned char[len];
    _lseeki64(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);
    unobfuscate(buff, len);

    string out_filename = entries[i].filename;

    if (do_prefix) {
      out_filename = prefix + "+" + entries[i].filename;
    }

    as::write_file(out_filename, buff, len);

    delete [] buff;   
  }

  delete [] entries;

  close(fd);

  return 0;
}
