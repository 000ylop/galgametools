// exchpac.cpp, v1.1 2009/11/27
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from GIGA's PAC (*.pac) archives.

#include "as-util.h"
#include "as-huffman.h"
#include "as-lzss.h"
#include "zlib.h"

#pragma pack(1)
struct PACHDR {
  unsigned char signature[4]; // "PAC"
  unsigned long entry_count;
  unsigned long type;
};

enum PACTYPE {
  PACTYPE_PLAIN,
  PACTYPE_LZSS,
  PACTYPE_HUFFMAN,
  PACTYPE_DEFLATE,
  PACTYPE_DEFLATE_MAYBE
};

struct PACTRL {
  unsigned long toc_length;
};

struct PACENTRY {
  char          name[64];
  unsigned long offset;
  unsigned long original_length;
  unsigned long length;
};
#pragma pack()

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exchpac v1.1 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pac>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PACHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  PACENTRY*     entries     = new PACENTRY[hdr.entry_count];
  unsigned long entries_len = sizeof(PACENTRY) * hdr.entry_count;
  read(fd, entries, entries_len);

  if (entries[0].offset != sizeof(hdr) + entries_len) {
    PACTRL trl;
    lseek(fd, -1 * (long)sizeof(trl), SEEK_END);
    read(fd, &trl, sizeof(trl));

    unsigned long  toc_len  = trl.toc_length;
    unsigned char* toc_buff = new unsigned char[toc_len];
    lseek(fd, -1 * (long)(sizeof(trl) + toc_len), SEEK_END);
    read(fd, toc_buff, toc_len);

    for (unsigned long i = 0; i < toc_len; i++) {
      toc_buff[i] ^= 0xFF;
    }

    as::unhuffman(toc_buff, 
                  toc_len, 
                  (unsigned char*) entries, 
                  entries_len);
  }
  
  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    if (!entries[i].length) {
      continue;
    }

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    unsigned long  out_len  = entries[i].original_length;
    unsigned char* out_buff = new unsigned char[out_len];

    switch (hdr.type) {
    case PACTYPE_LZSS:
      as::unlzss(buff, len, out_buff, out_len);
      break;

    case PACTYPE_HUFFMAN:
      as::unhuffman(buff, len, out_buff, out_len);
      break;

    case PACTYPE_DEFLATE:
      uncompress(out_buff, &out_len, buff, len);
      break;

	  case PACTYPE_DEFLATE_MAYBE:
		  if (len != out_len) {
        uncompress(out_buff, &out_len, buff, len);
        break;
		  } else {
        memcpy(out_buff, buff, out_len);
		  }
		  break;

    case PACTYPE_PLAIN:
    default:
      memcpy(out_buff, buff, out_len);
      break;
    }

    as::write_file(entries[i].name, out_buff, out_len);

    delete [] out_buff;
    delete [] buff;
  }

  return 0;
}

