// excfarc.cpp, v1.31 2009/03/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from ARC archives used by CROSS FIRE, îNè„Lesson,
// ä≠êı2 and others.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <algorithm>

#include "as-util.h"
#include "as-lzss.h"

struct ARCHDR {
  unsigned long entry_count;
  unsigned long unknown;
};

struct ARCENTRY {
  char          filename[32];
  unsigned long original_length;
  unsigned long length;
  unsigned long offset;
};

void unobfuscate(unsigned char* buff, unsigned long len, char* key) {
  size_t key_len = strlen(key);

  for (unsigned int i = 0; i < len; i++) {
    buff[i] += (unsigned char) key[i % key_len];
  }
}

bool find_key(unsigned char* sample, unsigned long sample_len, char* out_key) {
  unsigned long  key_len = 2;
  unsigned char* key     = sample + sample_len - key_len;

  while (key_len * 2 < sample_len && memcmp(key, key - key_len, key_len)) {
    key_len++;
    key--;
  }

  if (key_len * 2 < sample_len) {
    unsigned long offset = key_len - sample_len % key_len;

    for (unsigned long i = 0; i < key_len; i++) {
      out_key[i] = (char) (0 - key[(i + offset) % key_len]);
    }

    return true;
  }

  return false;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "excfarc v1.31 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.arc>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  ARCHDR hdr; 
  read(fd, &hdr, sizeof(hdr));

  ARCENTRY*     entries     = new ARCENTRY[hdr.entry_count];
  unsigned long entries_len = sizeof(ARCENTRY) * hdr.entry_count;
  read(fd, entries, entries_len);

  char key[1024] = { 0 };
  bool key_found = find_key((unsigned char*) entries[0].filename, 
                            sizeof(entries[0].filename),
                            key);

  if (!key_found) {
    fprintf(stderr, "Could not guess key.\n");
    return -1;
  }

  fprintf(stderr, "Guessed key: \"%s\"\n", key);

  unobfuscate((unsigned char*)entries, entries_len, key);

  for (unsigned long int i = 0; i < hdr.entry_count; i++) {    
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

    // Some bitmaps need fixed
    if (!memcmp(buff, "BM", 2)) {
      BITMAPFILEHEADER* bmf = (BITMAPFILEHEADER*) buff;

      if (bmf->bfSize != len) {
        BITMAPINFOHEADER* bmi = (BITMAPINFOHEADER*) (bmf + 1);

        // Let's assume we don't have any 8-bit bitmaps with alpha
        unsigned long  rgb_stride  = (bmi->biWidth * 3 + 3) & ~3;
        unsigned char* rgb_buff    = (unsigned char*) (bmi + 1);

        unsigned long  msk_stride  = bmi->biWidth;
        unsigned char* msk_buff    = buff + bmf->bfSize;

        unsigned long  rgba_stride = bmi->biWidth * 4;
        unsigned long  temp_len    = sizeof(*bmf) + sizeof(*bmi) + bmi->biHeight * rgba_stride;
        unsigned char* temp_buff   = new unsigned char[temp_len];
        unsigned char* rgba_buff   = temp_buff + sizeof(*bmf) + sizeof(*bmi);

        BITMAPFILEHEADER* out_bmf = (BITMAPFILEHEADER*) temp_buff;
        BITMAPINFOHEADER* out_bmi = (BITMAPINFOHEADER*) (out_bmf + 1);

        *out_bmf = *bmf;
        *out_bmi = *bmi;

        out_bmf->bfSize      = temp_len;
        out_bmi->biBitCount  = 32;
        out_bmi->biSizeImage = 0;

        for (int y = 0; y < bmi->biHeight; y++) {
          unsigned char* rgb_line  = rgb_buff  + y * rgb_stride;
          unsigned char* msk_line  = msk_buff  + y * msk_stride;
          unsigned char* rgba_line = rgba_buff + y * rgba_stride;

          for (int x = 0; x < bmi->biWidth; x++) {
            rgba_line[x * 4 + 0] = rgb_line[x * 3 + 0];
            rgba_line[x * 4 + 1] = rgb_line[x * 3 + 1];
            rgba_line[x * 4 + 2] = rgb_line[x * 3 + 2];
            rgba_line[x * 4 + 3] = msk_line[x];
          }
        }

        std::swap(len, temp_len);
        std::swap(buff, temp_buff);

        delete [] temp_buff;
      }
    }

    char filename[sizeof(entries[i].filename) + 1] = { 0 };
    memcpy(filename, entries[i].filename, sizeof(entries[i].filename));

    as::write_file(filename, buff, len);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
