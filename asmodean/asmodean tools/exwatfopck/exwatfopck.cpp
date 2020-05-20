// exwatfopck.cpp, v1.0 2010/10/14
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.pck archives used by WHITE ALBUMÅ|í‘ÇÁÇÍÇÈì~ÇÃëzÇ¢èoÅ| (PS3).

#include "as-util.h"

struct SECTHDR {
  unsigned char signature[8]; // "Filename", "Pack    "
  unsigned long length;
};

struct FNENTRY {
  unsigned long offset;
};

struct TOCHDR {
  unsigned long entry_count;
};

struct TOCENTRY {  
  unsigned long offset;
  unsigned long length;
};

struct TEXHDR {
  unsigned long unknown1;
  unsigned long data_length;
  unsigned long width;
  unsigned long height;
  unsigned long width2;
  unsigned long height2;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exwatfopck v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pck>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  SECTHDR secthdr;
  read(fd, &secthdr, sizeof(secthdr));

  unsigned long  fn_len  = ((as::flip_endian(secthdr.length) - sizeof(secthdr)) + 3 & ~3);
  unsigned char* fn_buff = new unsigned char[fn_len];
  read(fd, fn_buff, fn_len);

  FNENTRY* fn_entries = (FNENTRY*) fn_buff;

  read(fd, &secthdr, sizeof(secthdr));
  unsigned long  toc_len  = as::flip_endian(secthdr.length) - sizeof(secthdr);
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);
  
  TOCHDR*   toc_hdr     = (TOCHDR*) toc_buff;
  TOCENTRY* toc_entries = (TOCENTRY*) (toc_hdr + 1);

  unsigned long entry_count = as::flip_endian(toc_hdr->entry_count);

  for (unsigned long i = 0; i < entry_count; i++) {
    char* filename = (char*) (fn_buff + as::flip_endian(fn_entries[i].offset));

    unsigned long  offset = as::flip_endian(toc_entries[i].offset);
    unsigned long  len    = as::flip_endian(toc_entries[i].length);
    unsigned char* buff   = new unsigned char[len];
    lseek(fd, offset, SEEK_SET);
    read(fd, buff, len);

    TEXHDR* texhdr = NULL;

    if (len >= 8 && !memcmp(buff, "Texture ", 8)) {
      texhdr = (TEXHDR*) (buff + sizeof(SECTHDR));
    } else if (len > sizeof(TEXHDR) && as::stringtol(filename).find(".tex") != string::npos) {
      texhdr = (TEXHDR*) buff;
    }

    // Other types are MotionPortrait data... we could extract the PNG texture but it's useless
    // garbage until I reverse the MP format.
    if (texhdr && as::flip_endian(texhdr->unknown1) == 1) {   
      unsigned long  data_len  = as::flip_endian(texhdr->data_length);
      unsigned char* data_buff = (unsigned char*) (texhdr + 1);

      as::write_file(as::get_file_prefix(filename) + as::guess_file_extension(data_buff, data_len), data_buff, data_len);
    } else {
      as::write_file(filename, buff, len);
    }

    delete [] buff;
  }

  delete [] toc_buff;
  delete [] fn_entries;

  close(fd);

  return 0;
}
