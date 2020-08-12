// exdaf2.cpp, v1.01 2008/12/27
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Extracts DAF2 (*.DAT) archives used by CROSSNET.

#include "as-util.h"
#include "zlib.h"

#define DAF2_VERSION 2

struct DAF2HDR {
  unsigned char signature[4]; // "DAF2"
  unsigned long unknown1;
  unsigned long entry_count;
  unsigned long unknown3;
  unsigned long toc_length;
  unsigned long original_toc_length;
  unsigned long unknown4;
  unsigned long unknown5;
  unsigned long key_words[4];
};

struct DAF2ENTRY {
  unsigned long entry_length;
  unsigned long offset;
  unsigned long length;
  unsigned long original_length;
#if DAF2_VERSION == 1
  unsigned char unknown1[32]; // hash?
#endif
  unsigned long compressed;
  char          filename[1];
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exdaf2 v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <archive.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  DAF2HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long key = 0;

  key |= (hdr.key_words[0] & 0x000000FF) << 24;
  key |= (hdr.key_words[1] & 0x0000FF00) << 8;
  key |= (hdr.key_words[2] & 0x00FF0000) >> 8;
  key |= (hdr.key_words[3] & 0xFF000000) >> 24;

  hdr.entry_count         ^= key;
  hdr.toc_length          ^= key;
  hdr.original_toc_length ^= key;

  unsigned long  toc_len  = hdr.original_toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];

  {
    unsigned long  len  = hdr.toc_length;
    unsigned char* buff = new unsigned char[len];
    read(fd, buff, hdr.toc_length);
    uncompress(toc_buff, &toc_len, buff, len);

    delete [] buff;
  }

  unsigned char* p         = toc_buff;
  unsigned long  data_base = (sizeof(hdr) + hdr.toc_length + 3) & ~3;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    DAF2ENTRY* entry = (DAF2ENTRY*) p;    
   
    entry->entry_length    ^= key;
    entry->offset          ^= key;
    entry->length          ^= key;
    entry->original_length ^= key;

    p += entry->entry_length;

    unsigned long  len  = entry->length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, data_base + entry->offset, SEEK_SET);
    read(fd, buff, len);

    if (entry->compressed) {
      unsigned long  temp_len  = entry->original_length;
      unsigned char* temp_buff = new unsigned char[temp_len];
      uncompress(temp_buff, &temp_len, buff, len);

      delete [] buff;

      len  = temp_len;
      buff = temp_buff;
    }

    as::make_path(entry->filename);

    // Could be original_length < len because of padding.
    as::write_file(entry->filename, buff, entry->original_length);

    delete [] buff;
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}