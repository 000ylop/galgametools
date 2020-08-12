// expatbin.cpp, v1.0 2006/12/23
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from BIN/LST archives used by Patissier.
// Normally the *.lst indexes are stored in lists.bin/lst.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <string>
#include "zlib.h"

struct BINHDR {
  unsigned char signature[4]; // 0x4F5A0001
  unsigned char signature2[4]; // "OFST"
  unsigned long toc_length;
};

struct DFLTHDR {
  unsigned char signature[4]; // "DFLT"
  unsigned long length;
  unsigned long original_length;
};

struct DATAHDR {
  unsigned char signature[4]; // "DATA"
  unsigned long length;
};

unsigned long inflate_buff(unsigned char* in_buff, 
                           unsigned long  in_len, 
                           unsigned char* out_buff, 
                           unsigned long  out_len) {
  z_stream d_stream;

  d_stream.zalloc    = NULL;
  d_stream.zfree     = NULL;
  d_stream.opaque    = NULL;
  d_stream.next_in   = in_buff;
  d_stream.avail_in  = in_len;
  d_stream.next_out  = out_buff;
  d_stream.avail_out = out_len; 

  unsigned long actual_out = 0;
     
  if (inflateInit(&d_stream) >= 0) {  
    inflate(&d_stream, Z_FINISH);
    inflateEnd(&d_stream);
    actual_out = d_stream.total_out;
  }

  return actual_out;
}

using std::string;

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "expatbin v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bin> <input.lst>\n", argv[0]);
    return -1;
  }

  char* bin_filename  = argv[1];
  char* lst_filename  = argv[2];
  
  int bin_fd = open(bin_filename, O_RDONLY | O_BINARY);

  if (bin_fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", bin_filename, strerror(errno));
    return -1;
  }

  int lst_fd = open(lst_filename, O_RDONLY | O_BINARY);

  if (lst_fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", lst_filename, strerror(errno));
    return -1;
  }

  unsigned long lst_len;
  {
    struct stat file_stat;
    fstat(lst_fd, &file_stat);
    lst_len = file_stat.st_size;
  }

  unsigned char* lst_buff = new unsigned char[lst_len + 1];
  memset(lst_buff, 0, lst_len + 1);
  read(lst_fd, lst_buff, lst_len);
  close(lst_fd);

  for (unsigned long i = 0; i < lst_len; i++) {
    if (lst_buff[i] == 0x0A || lst_buff[i] == 0x0D || lst_buff[i] == 0x09) {
      lst_buff[i] = 0x00;
    }

    if (lst_buff[i] == ':') {
      lst_buff[i] = '+';
    }
  }

  char* filename = (char*) lst_buff;

  BINHDR hdr;
  
  read(bin_fd, &hdr, sizeof(hdr));

  unsigned long  entry_count  = hdr.toc_length / sizeof(unsigned long);
  unsigned long* offset_table = new unsigned long[entry_count];

  read(bin_fd, offset_table, hdr.toc_length);

  for (i = 0; i < entry_count; i++) {
    // There might be multiple nulls from CRLFs..
    while (!filename[0]) {
      filename++;
    }

    unsigned char sigcheck[4];

    lseek(bin_fd, offset_table[i], SEEK_SET);
    read(bin_fd, sigcheck, sizeof(sigcheck));
    lseek(bin_fd, offset_table[i], SEEK_SET);

    unsigned long  out_len  = 0;
    unsigned char* out_buff = NULL;
    string         ext;

    if (!memcmp(sigcheck, "DFLT", 4)) {
      ext = ".lst";

      DFLTHDR entryhdr;
      read(bin_fd, &entryhdr, sizeof(entryhdr));

      unsigned long  len  = entryhdr.length;
      unsigned char* buff = new unsigned char[len];

      read(bin_fd, buff, len);

      out_len  = entryhdr.original_length;
      out_buff = new unsigned char[out_len];

      inflate_buff(buff, len, out_buff, out_len);

      delete [] buff;
    } else if (!memcmp(sigcheck, "DATA", 4)) {
      DATAHDR entryhdr;
      read(bin_fd, &entryhdr, sizeof(entryhdr));

      out_len  = entryhdr.length;
      out_buff = new unsigned char[out_len];

      read(bin_fd, out_buff, out_len);

      if (!memcmp(out_buff, "\x89PNG", 4)) {
        ext = ".png";
      } else if (!memcmp(out_buff, "OggS", 4)) {
        ext = ".ogg";
      }
    } else {
      fprintf(stderr, "%s: unknown entry type\n", filename);
    }

    char out_filename[4096] = { 0 };
    sprintf(out_filename, "%s%s", filename, ext.c_str());

    int out_fd = open(out_filename, O_CREAT | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE);

    if (out_fd == -1) {
      fprintf(stderr, "Could not open %s (%s)\n", out_filename, strerror(errno));
      return -1;
    }

    write(out_fd, out_buff, out_len);

    delete [] out_buff;

    filename += strlen(filename) + 1;   
  }

  delete [] lst_buff;

  close(bin_fd);

  return 0;
}
