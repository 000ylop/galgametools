// extskfdat, v1.0 2008/12/22
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts tskforce (*.dat) archives.

#include "as-util.h"
#include "as-lzss.h"

struct TSKFHDR {
  unsigned char signature[8]; // "tskforce"
  unsigned long entry_count;
};

struct TSKFENTRY {
  char          filename[256];
  unsigned long offset;
  unsigned long original_length;
  unsigned long length;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "extskfdat v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n", argv[0]);
    return -1;
  }

  string filename(argv[1]);
  
  int fd = as::open_or_die(filename, O_RDONLY | O_BINARY);

  TSKFHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  TSKFENTRY* entries = new TSKFENTRY[hdr.entry_count];
  read(fd, entries, sizeof(TSKFENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (entries[i].length != entries[i].original_length) {
      unsigned long  temp_len  = entries[i].original_length;
      unsigned char* temp_buff = new unsigned char[temp_len];
      as::unlzss(buff, len, temp_buff, temp_len);

      delete [] buff;

      len  = temp_len;
      buff = temp_buff;
    }

    as::make_path(entries[i].filename);
    as::write_file(entries[i].filename, buff, len);

    delete [] buff;
  }
  
  delete [] entries;

  return 0;
}
