// exmk2.cpp, v1.01 2008/04/04
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from Maika's newer MK2.0 (*.DAT) archives.
// BPR02 graphics are decompressed and converted to bitmaps.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>
#include <algorithm>

#include "as-util.h"
#include "as-lzss.h"

using std::string;

#pragma pack(1)
struct MK2HDR {
  unsigned char  signature[8];
  unsigned short header_length;
  unsigned long  toc_length;
  unsigned long  toc_offset;
  unsigned long  entry_count;
  unsigned char  pad[6];
};

struct MK2OFFENTRY {
  unsigned short entry_offset;
  unsigned long  unknown; // subentry count?
};

struct MK2ENTRY {
  unsigned long offset;
  unsigned long length;
  unsigned char filename_length;
  char          filename[1];
};

struct LZHDR {
  unsigned short type;
  unsigned long  length;
  unsigned long  original_length;
};

#pragma pack()

unsigned long unbpr02(unsigned char* buff, 
                      unsigned long  len, 
                      unsigned char* out_buff, 
                      unsigned long  out_len) 
{
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;

  memset(out_buff, 0, out_len);

  while (buff < end && out_buff < out_end) {
    unsigned char c = *buff++;

    // Check this now or computing n might die on a page boundary :)
    if (c == 0xFF) {
      break;
    }

    unsigned long n = *(unsigned long*)buff;
    buff += 4;

    switch (c) {
    case 3:
      while (n-- && buff < end && out_buff < out_end) {
        *out_buff++ = *buff;
      }
      buff++;
      break;
    
    case 4:
      while (n-- && buff < end && out_buff < out_end) {
        *out_buff++ = *buff++;
      }
      break;

    default:
      fprintf(stderr, "unknown code: %d\n", c);
    }
  }

  return out_len - (out_end - out_buff);
}

void unobfuscate(unsigned char* buff) {
  unsigned short* type = (unsigned short*)buff;

  switch (*type) {
  case 0x3146:
    std::swap(buff[0x11], buff[0x15]);
    std::swap(buff[0x13], buff[0x16]);
    // Intentional fall through
  case 0x3143:
  case 0x3144:
    *type = 0x3142;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmk2 v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  MK2HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned char* toc_buff = new unsigned char[hdr.toc_length];
  lseek(fd, hdr.toc_offset, SEEK_SET);
  read(fd, toc_buff, hdr.toc_length);

  // For some reason the table doesn't have enough entries, so we only use
  // it to find the first one...
  MK2OFFENTRY*   entry_table = (MK2OFFENTRY*) toc_buff;
  unsigned char* entry_p     = toc_buff + entry_table[0].entry_offset;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    MK2ENTRY* entry = (MK2ENTRY*) entry_p;
    string    out_filename(entry->filename, entry->filename_length);

    unsigned long  len  = entry->length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entry->offset + hdr.header_length, SEEK_SET);
    read(fd, buff, len);

    unsigned long  out_len  = len;
    unsigned char* out_buff = buff;

    bool is_compressed = out_filename.find(".BMP") != string::npos || 
                         out_filename.find(".bmp") != string::npos;

    if (is_compressed) {
      unobfuscate(buff);

      LZHDR* lzhdr = (LZHDR*) buff;
    
      unsigned long  unlz_len    = lzhdr->original_length;
      unsigned char* unlz_buff   = new unsigned char[unlz_len];   
      unsigned long  unlz_actual = as::unlzss(buff + sizeof(*lzhdr), lzhdr->length, unlz_buff, unlz_len);      

      if (!memcmp(unlz_buff, "BPR02", 5)) {
        // Don't know how much data to expect so pick something "big enough" ...
        unsigned long out_max = 1024 * 1024 * 5;
        out_buff              = new unsigned char[out_max];
        out_len               = unbpr02(unlz_buff + 5, unlz_actual - 5, out_buff, out_max);

        delete [] unlz_buff;
        delete [] buff;
      } else {
        delete [] buff;

        out_len  = unlz_actual;
        out_buff = unlz_buff;
      }
    }

    int out_fd = as::open_or_die(out_filename, 
                                 O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
                                 S_IREAD | S_IWRITE);
    write(out_fd, out_buff, out_len);
    close(out_fd);

    delete [] out_buff;

    entry_p += sizeof(*entry) - 1 + entry->filename_length;
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}

