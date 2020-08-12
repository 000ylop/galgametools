// exlac.cpp, v1.01 2009/12/20
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Extracts LAC (*.PAK) archives used by Leaf.

#include "as-util.h"
#include "as-lzss.h"

struct LACHDR {
  unsigned char signature[4]; // "LAC"
  unsigned long entry_count;
};

struct LACENTRY {
  char          filename[31];
  unsigned char compressed;  
  unsigned long length;
  unsigned long offset;
};

struct LACCOMPHDR {
  unsigned long original_length;
};

struct LGFHDR {
  unsigned char  signature[3]; // "lgf"
  unsigned char  depth;
  unsigned short width;
  unsigned short height;
  unsigned long  unknown1;
};

void unobfuscate(LACENTRY& entry) {
  for (unsigned long i = 0; i < sizeof(entry.filename) && entry.filename[i]; i++) {
    entry.filename[i] ^= 0xFF;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exlac v1.01, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.lac>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  LACHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  LACENTRY* entries = new LACENTRY[hdr.entry_count];
  read(fd, entries, sizeof(LACENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unobfuscate(entries[i]);

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (entries[i].compressed) {
      LACCOMPHDR* comphdr = (LACCOMPHDR*) buff;
      
      unsigned long  temp_len  = comphdr->original_length;
      unsigned char* temp_buff = new unsigned char[temp_len];
      as::unlzss(buff + sizeof(*comphdr), len - sizeof(*comphdr), temp_buff, temp_len);

      delete [] buff;

      len  = temp_len;
      buff = temp_buff;
    }

    if (len >= 3 && !memcmp(buff, "lgf", 3)) {
      LGFHDR* lgfhdr = (LGFHDR*) buff;

      unsigned long  rgb_len  = len  - sizeof(*lgfhdr);
      unsigned char* rgb_buff = buff + sizeof(*lgfhdr);
      unsigned char* pal_buff = NULL;

      if (lgfhdr->depth == 8) {
        pal_buff = rgb_buff;

        rgb_len  -= 1024;
        rgb_buff += 1024;
      }

      as::write_bmp_ex(as::get_file_prefix(entries[i].filename) + ".bmp",
                       rgb_buff,
                       rgb_len,
                       lgfhdr->width,
                       lgfhdr->height,
                       lgfhdr->depth / 8,
                       256,
                       pal_buff,
                       as::WRITE_BMP_FLIP);
    } else {
      as::write_file(entries[i].filename, buff, len);
    }

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}