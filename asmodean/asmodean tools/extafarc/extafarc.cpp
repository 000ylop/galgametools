// extafarc.cpp, v1.01 2008/12/26
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from TACTICS_ARC_FILE (*.arc) archives.

#include "as-util.h"
#include "as-lzss.h"

#define TAF_VERSION 2

static const unsigned char TOC_KEY[] = { 0xFF };

struct ARCHDR {
  unsigned char signature[16]; // "TACTICS_ARC_FILE"
  unsigned long toc_length;
  unsigned long original_toc_length;
  unsigned long entry_count;
  unsigned long unknown;
};

struct ARCENTRY {
  unsigned long offset;
  unsigned long length;
  unsigned long original_length;
  unsigned long filename_length;
#if TAF_VERSION >= 2
  unsigned char unknown[8]; // some kind of hash?
#endif
};

void unobfuscate(unsigned char*      buff, 
                 unsigned long       len,
                 const unsigned char* key,
                 unsigned long        key_len) 
{
  for (unsigned long i = 0; i < len; i++) {
    buff[i] ^= key[i % key_len];
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "extafarc v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.arc>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  ARCHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = hdr.toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);
  unobfuscate(toc_buff, toc_len, TOC_KEY, sizeof(TOC_KEY));

  unsigned long  out_toc_len  = hdr.original_toc_length;
  unsigned char* out_toc_buff = new unsigned char[out_toc_len];
  as::unlzss(toc_buff, toc_len, out_toc_buff, out_toc_len);  

  unsigned char* p       = out_toc_buff;
  unsigned char* key     = p;
  unsigned long  key_len = 0;

  // Not clear whether the length can vary, but we'll try to deal with it.
  while (key[key_len]) key_len++;
  p += key_len + 1;

  unsigned long data_offset = sizeof(hdr) + toc_len;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    ARCENTRY* entry = (ARCENTRY*) p;
    p += sizeof(*entry);

    string filename((char*)p, entry->filename_length);
    p += entry->filename_length;

    unsigned long  len  = entry->length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, data_offset + entry->offset, SEEK_SET);
    read(fd, buff, len);
    unobfuscate(buff, len, key, key_len);

    if (entry->original_length) {
      unsigned long  temp_len  = entry->original_length;
      unsigned char* temp_buff = new unsigned char[temp_len];
      as::unlzss(buff, len, temp_buff, temp_len);

      delete [] buff;

      len  = temp_len;
      buff = temp_buff;
    }
  
    as::make_path(filename);
    as::write_file(filename, buff, len);

    delete [] buff;
  }

  delete [] out_toc_buff;
  delete [] toc_buff;

  return 0;
}
