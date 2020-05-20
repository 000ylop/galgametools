// extk2fpk.cpp, v1.02 2010/02/27
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool decompresses data from *.FPK archives...

#include "as-util.h"

// Version 1 is detected by the header flag.
//#define FPK_VERSION 2
#define FPK_VERSION 3

struct FPKHDR {
  unsigned long entry_count; // high bit set for some reason
};

struct FPKTRL {
  unsigned long key;
  unsigned long toc_offset;
};

#if FPK_VERSION >= 3
struct FPKENTRY1 {
  unsigned long offset;
  unsigned long length;
  char          filename[128];
  unsigned long unknown;
};
#else
struct FPKENTRY1 {
  unsigned long offset;
  unsigned long length;
  char          filename[24];
  unsigned long unknown;
};
#endif

struct FPKENTRY2 {
  unsigned long offset;
  unsigned long length;
  char          filename[24];
};

struct ZLC2HDR {
  unsigned char signature[4]; // "ZLC2"
  unsigned long original_length;
};

// Pretty much LZSS
void unzlc2(unsigned char* buff, 
            unsigned long  len,
            unsigned char* out_buff, 
            unsigned long  out_len) 
{
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;

  while (buff < end && out_buff < out_end) {
    unsigned char flags = *buff++;

    for (int i = 0; i < 8 && buff < end && out_buff < out_end; i++) {
      if (flags & 0x80) {
        if (end - buff < 2)
          break;

        unsigned long p = *buff++;
        unsigned long n = *buff++;
                                
        p |= (n & 0xF0) << 4;
        n  = (n & 0x0F) + 3;

        if (!p) {
          p = 4096;
        }

        for (unsigned long j = 0; j < n && out_buff < out_end; j++) {
          *out_buff++ = *(out_buff - p);
        }
      } else {
        *out_buff++ = *buff++;
      }

      flags <<= 1;
    }
  }
}

void unobfuscate(unsigned char* buff,
                 unsigned long  len,
                 unsigned long  key) 
{
  unsigned long* p   = (unsigned long*) buff;
  unsigned long* end = (unsigned long*) (buff + len);

  while (p < end) {
    *p++ ^= key;
  }
}

template <class ENTRY_T>
void process_toc(int fd, unsigned char* toc_buff, unsigned long entry_count) {
  ENTRY_T* entries = (ENTRY_T*) toc_buff;

  for (unsigned long i = 0; i < entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    ZLC2HDR* zlc2hdr = (ZLC2HDR*) buff;    

    if (!memcmp(zlc2hdr->signature, "ZLC2", 4)) {
      unsigned long  out_len  = zlc2hdr->original_length;
      unsigned char* out_buff = new unsigned char[out_len];
      unzlc2(buff + sizeof(*zlc2hdr), len - sizeof(*zlc2hdr), out_buff, out_len);

      delete [] buff;

      len  = out_len;
      buff = out_buff;
    }

    int out_fd = as::open_or_die(entries[i].filename,
                                 O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                                 S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "extk2fpk v1.02 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.fpk>\n", argv[0]);   
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  FPKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  entry_count = hdr.entry_count & ~0x80000000;
  unsigned long  toc_len     = 0;
  unsigned char* toc_buff    = NULL;

  if (hdr.entry_count & 0x80000000) {
    FPKTRL trl;
    lseek(fd, -(int)sizeof(trl), SEEK_END);
    read(fd, &trl, sizeof(trl));

    toc_len  = ((sizeof(FPKENTRY1) * entry_count) + 3) & ~3;
    toc_buff = new unsigned char[toc_len];

    lseek(fd, trl.toc_offset, SEEK_SET);
    read(fd, toc_buff, toc_len);
    unobfuscate(toc_buff, toc_len, trl.key);
    process_toc<FPKENTRY1>(fd, toc_buff, entry_count);
  } else {
    toc_len  = sizeof(FPKENTRY2) * entry_count;
    toc_buff = new unsigned char[toc_len];

    read(fd, toc_buff, toc_len);
    process_toc<FPKENTRY2>(fd, toc_buff, entry_count);
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}

