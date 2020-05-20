// exedendp5.cpp, v1.0 2011/06/10
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts DataPack5 (*.dat) archives used by キミとボクとエデンの林檎
// and possibly others.

#include "as-util.h"
#include "as-lzss.h"

struct DP5HDR {
  unsigned char signature[16];  // "DataPack5"
  unsigned char signature2[32]; // "EDEN", etc
  unsigned long unknown1;
  unsigned long toc_length;
  unsigned long flags;
  unsigned long entry_count;
  unsigned long data_base;
  unsigned long toc_offset;
};

static const unsigned long DP5HDR_FLAG_COMPRESSED_TOC = 0x00000001;
static const unsigned long DP5HDR_FLAG_OBFUSCATED     = 0x00000002;

struct DP5ENTRY {
  char          filename[64];
  unsigned long offset;
  unsigned long length;
  unsigned long unknown1; // 1
  unsigned long unknown2; // 1
  unsigned long unknown3[6];
};

void unobfuscate_data(char*          name,
                      unsigned char* buff,
                      unsigned long  len)
{
  unsigned long key = 0;

  for (unsigned long i = 0; name[i]; i++) {
    key *= 0x25;
    key += name[i] | 0x20;
  }

  unsigned long* p   = (unsigned long*) buff;
  unsigned long* end = p + len / 4;

  while (p < end) {
    *p++ ^= key;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exedendp5 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n", argv[0]);
    return -1;
  }

  string filename(argv[1]);

  int fd = as::open_or_die(filename, O_RDONLY | O_BINARY);

  DP5HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = hdr.toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];
  lseek(fd, hdr.toc_offset, SEEK_SET);
  read(fd, toc_buff, toc_len);

  unsigned long entries_len = sizeof(DP5ENTRY) * hdr.entry_count;
  DP5ENTRY*     entries     = new DP5ENTRY[hdr.entry_count];

  if (hdr.flags & DP5HDR_FLAG_COMPRESSED_TOC) {
    for (unsigned long i = 0; i < toc_len; i++) {
      toc_buff[i] ^= (unsigned char) i;
    }

    as::unlzss(toc_buff, toc_len, (unsigned char*) entries, entries_len);
  } else {
    memcpy(entries, toc_buff, entries_len);
  }

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[(len + 3) & ~3];
    lseek(fd, hdr.data_base + entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (hdr.flags & DP5HDR_FLAG_OBFUSCATED) {
      unobfuscate_data(entries[i].filename, buff, len);
    }

    as::write_file(entries[i].filename + as::guess_file_extension(buff, len),
                   buff,
                   len);

    delete [] buff;
  }

  close(fd);
  
  return 0;
}
