// exaqugsp.cpp, v1.0 2008/04/29
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool decompresses data from Aquanaut's *.gsp archives...

#include "as-util.h"
#include "zlib.h"

struct GSPHDR {
  unsigned long entry_count;
};

struct GSPENTRY {
  unsigned long offset;
  unsigned long length;
  char          filename[56];
};

struct ZLC3HDR {
  unsigned char signature[4]; // "ZLC3"
  unsigned long original_length;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exaqugsp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.gsp>\n", argv[0]);   
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  GSPHDR hdr;
  read(fd, &hdr, sizeof(hdr));
  
  GSPENTRY* entries = new GSPENTRY[hdr.entry_count];
  read(fd, entries, sizeof(GSPENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    string filename = entries[i].filename;

    ZLC3HDR* zlc3hdr = (ZLC3HDR*) buff;    

    if (!memcmp(zlc3hdr->signature, "ZLC3", 4)) {
      unsigned long  out_len  = zlc3hdr->original_length;
      unsigned char* out_buff = new unsigned char[out_len];
      uncompress(out_buff, &out_len, buff + sizeof(*zlc3hdr), len - sizeof(*zlc3hdr));

      delete [] buff;

      filename = as::get_file_prefix(filename) + as::guess_file_extension(out_buff, out_len);
      len      = out_len;
      buff     = out_buff;
    }

    int out_fd = as::open_or_die(filename,
                                 O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                                 S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}

