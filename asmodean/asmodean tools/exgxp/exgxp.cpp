// exgxp.cpp, v1.01 2012/04/27
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts GXP (*.gxp) archives

#include "as-util.h"

#pragma pack(1)
struct GXPHDR {
  unsigned char      signature[4]; // "GXP"
  unsigned long      unknown1;
  unsigned long      unknown2;
  unsigned long      unknown3;
  unsigned long      unknown4;
  unsigned long      unknown5;
  unsigned long      entry_count;
  unsigned long      toc_length;
  unsigned long      unknown6;
  unsigned long      unknown7;
  unsigned long long data_base; 
};

struct GXPENTRY {
  unsigned long      entry_length;
  unsigned long      length;
  unsigned long      unknown1;
  unsigned long      filename_length;
  unsigned long      unknown2;
  unsigned long      unknown3;
  unsigned long long offset;
  wchar_t            filename[1]; // wchar_t filename[filename_length];
};
#pragma pack()

void unobfuscate(unsigned char* buff, unsigned long len) {
  static const unsigned char KEY[] = { 0x40, 0x21, 0x28, 0x38, 0xA6, 0x6E, 0x43, 0xA5,
                                       0x40, 0x21, 0x28, 0x38, 0xA6, 0x43, 0xA5, 0x64, 
                                       0x3E, 0x65, 0x24, 0x20, 0x46, 0x6E, 0x74, };

  for (unsigned long i = 0; i < len; i++) {
    buff[i] ^= KEY[i % sizeof(KEY)] ^ i;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exgxp v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.gxp>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  GXPHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = hdr.toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);

  unsigned char* p = toc_buff;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    GXPENTRY* entry = (GXPENTRY*) p;

    unobfuscate(p, 4);
    unobfuscate(p, entry->entry_length);
    unobfuscate(p, 4);

    unsigned long  len  = entry->length;
    unsigned char* buff = new unsigned char[len];
    _lseeki64(fd, hdr.data_base + entry->offset, SEEK_SET);
    read(fd, buff, len);
    unobfuscate(buff, len);

    string filename = as::convert_wchar(entry->filename);

    as::make_path(filename);
    as::write_file(filename, buff, len);

    delete [] buff;

    p += entry->entry_length;
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}
