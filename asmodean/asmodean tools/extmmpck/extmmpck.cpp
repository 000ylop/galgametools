// extmmpck.cpp, v1.0 2008/05/28
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from encrypted PACK_FILE001 (*.pck) archives.

#include "as-util.h"
#include "blowfish.h"

static char* ARCHIVE_PASSWORD = "o01oa";

struct PCKHDR {
  unsigned char signature[12]; // "PACK_FILE001"
  unsigned long entry_count;
  unsigned long toc_length;
};

void read_decrypt(int            fd, 
                  unsigned char* buff, 
                  unsigned long  len, 
                  bool           is_data = true) 
{
  bool need_decrypt = true;

  if (is_data) {
    unsigned char type = 0;
    read(fd, &type, sizeof(type));

    need_decrypt = type != 0;
  }

  read(fd, buff, len);

  if (need_decrypt) {
    Blowfish bf;  
    bf.Set_Passwd(ARCHIVE_PASSWORD);
    bf.Decrypt(buff, len);
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "extmmpck v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pck>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PCKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = hdr.toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];
  read_decrypt(fd, toc_buff, toc_len, false);

  unsigned char* p = toc_buff;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long len = *(unsigned long*) p;
    p += 4;

    char* filename = (char*) p;
    p += strlen(filename) + 1;

    unsigned long padded_len = *(unsigned long*) p;
    p += 4;

    unsigned char* buff = new unsigned char[padded_len];
    read_decrypt(fd, buff, padded_len);

    as::write_file(filename, buff, len);

    delete [] buff;
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}
