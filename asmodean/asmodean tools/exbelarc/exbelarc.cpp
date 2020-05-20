// exbelarc.cpp, v1.01 2009/12/12
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from *.arc archives used by しろくまベルスターズ♪.
// Compile with -DARC_VERSION=1 for older games.

#include "as-util.h"

//#define ARC_VERSION 1
#define ARC_VERSION 2

#pragma pack(1)

struct ARCHDR {
  unsigned long section_count;
};

struct ARCSECTHDR {
  char          type[4]; // "PNG", "MOS", "OGG"
  unsigned long entry_count;
  unsigned long toc_offset;
};

struct ARCENTRY {
#if ARC_VERSION >= 2
  char          filename[13];
#else
  char          filename[9];
#endif
  unsigned long length;
  unsigned long offset;
};

struct WIPFHDR {
  unsigned char  signature[4]; // "WIPF";
  unsigned short entry_count;
  unsigned short depth;
};

struct WIPFENTRY {
  unsigned long  width;
  unsigned long  height;
  unsigned long  offset_x;
  unsigned long  offset_y;
  unsigned long  unknown1; // layer?
  unsigned long  length;
};

#pragma pack()

unsigned long unwipf(unsigned char* buff, 
                     unsigned long  len,
                     unsigned char* out_buff, 
                     unsigned long  out_len) 
{
  unsigned long  ring_len   = 4096;
  unsigned char* ring       = new unsigned char[ring_len];
  unsigned long  ring_index = 1;
  unsigned char* end        = buff + len;
  unsigned char* out_end    = out_buff + out_len;

  memset(ring, 0, ring_len);

  while (buff < end && out_buff < out_end) {
    unsigned char flags = *buff++;

    for (int i = 0; i < 8 && buff < end && out_buff < out_end; i++) {
      if (flags & 0x01) {
        *out_buff++ = ring[ring_index++ % ring_len] = *buff++;
      } else {
        if (end - buff < 2)
          break;

        unsigned long p = *buff++;
        unsigned long n = *buff++;

        p = (p << 4) | (n >> 4);
        n  = (n & 0x0F) + 2;

        for (unsigned long j = 0; j < n && out_buff < out_end; j++) {
          *out_buff++ = ring[ring_index++ % ring_len] = ring[p++ % ring_len];
        }
      }

      flags >>= 1;
    }
  }

  delete [] ring;

  return out_len - (out_end - out_buff);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exbelarc v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.arc>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  
  ARCHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  ARCSECTHDR* sections = new ARCSECTHDR[hdr.section_count];
  read(fd, sections, sizeof(ARCSECTHDR) * hdr.section_count);

  for (unsigned long i = 0; i < hdr.section_count; i++) {
    ARCENTRY* entries = new ARCENTRY[sections[i].entry_count];
    lseek(fd, sections[i].toc_offset, SEEK_SET);
    read(fd, entries, sizeof(ARCENTRY) * sections[i].entry_count);

    string suffix = string("_") + sections[i].type;

    for (unsigned long j = 0; j < sections[i].entry_count; j++) {
      string filename = entries[j].filename + suffix;

      unsigned long  len  = entries[j].length;
      unsigned char* buff = new unsigned char[len];
      lseek(fd, entries[j].offset, SEEK_SET);
      read(fd, buff, len);

      if (!memcmp(buff, "WIPF", 4)) {
        WIPFHDR*       wipfhdr     = (WIPFHDR*) buff;
        WIPFENTRY*     wipfentries = (WIPFENTRY*) (wipfhdr + 1);
        unsigned char* data_buff   = (unsigned char*) (wipfentries + wipfhdr->entry_count);

        for (unsigned long k = 0; k < wipfhdr->entry_count; k++) {
          unsigned char* pal_buff  = NULL;

          if (wipfhdr->depth == 8) {
            pal_buff   = data_buff;
            data_buff += 1024;
          }

          unsigned long  out_depth  = wipfhdr->depth / 8;
          unsigned long  out_stride = (wipfentries[k].width * out_depth + 3) & ~3;
          unsigned long  out_len    = wipfentries[k].height * out_stride;
          unsigned char* out_buff   = new unsigned char[out_len];
          unwipf(data_buff, wipfentries[k].length, out_buff, out_len);
          data_buff += wipfentries[k].length;

          string out_filename = filename;
          if (wipfhdr->entry_count > 1) {
            out_filename += as::stringf("+%03d+%dx%dy", 
                                        k,
                                        wipfentries[k].offset_x,
                                        wipfentries[k].offset_y);
          }

          if (wipfhdr->depth == 24) {
            unsigned char* temp_buff = new unsigned char[out_len];

            unsigned long color_stride = wipfentries[k].width;
            unsigned long color_len    = wipfentries[k].height * color_stride;

            for (unsigned long y = 0; y < wipfentries[k].height; y++) {
              as::RGB*       rgb_line = (as::RGB*) (temp_buff + y * out_stride);
              unsigned char* r_line   = out_buff + y * color_stride;
              unsigned char* g_line   = r_line + color_len;
              unsigned char* b_line   = g_line + color_len;

              for (unsigned long x = 0; x < wipfentries[k].width; x++) {
                rgb_line[x].r = r_line[x];
                rgb_line[x].g = g_line[x];
                rgb_line[x].b = b_line[x];
              }
            }

            delete [] out_buff;
            out_buff = temp_buff;
          }

          as::write_bmp_ex(out_filename + ".bmp", 
                           out_buff,
                           out_len,
                           wipfentries[k].width,
                           wipfentries[k].height,
                           out_depth,
                           256,
                           pal_buff,
                           as::WRITE_BMP_FLIP);

          delete [] out_buff;
        }
      } else {
        as::write_file(filename + as::guess_file_extension(buff, len), buff, len);
      }

      delete [] buff;
    }

    delete [] entries;
  }

  delete sections;

  close(fd);

  return 0;
}
