// exbkarc.cpp, v1.3 2012/05/30
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts images from Biniku no Kaori's *.arc achives with
// the alpha channel...

#include "as-util.h"
#include "as-lzss.h"

struct ARCHDR {
  unsigned long entry_count;
};

// numerics are big-endian for some reason
struct ARCENTRY {
  char          filename[260];
  unsigned long length;
  unsigned long original_length; // ?
  unsigned long offset;
};

struct RMTHDR {
  unsigned char signature[4]; // "RMT "
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long width;
  unsigned long height;
};

struct AKBHDR {
  unsigned char  signature[4]; // "AKB "
  unsigned short width;
  unsigned short height;
  unsigned short type;
  unsigned short flags; // ??
  unsigned char  bg_red;
  unsigned char  bg_green;
  unsigned char  bg_blue;
  unsigned char  bg_alpha;  
  unsigned long  left;
  unsigned long  top;
  unsigned long  right;
  unsigned long  bottom;
};

struct AKBPHDR 
  : public AKBHDR
{
  char base_name[32];
};

static const unsigned short AKB_TYPE_32BIT = 0x0000;
static const unsigned short AKB_TYPE_24BIT = 0x00FF;

void unobfuscate(ARCENTRY& entry) {
  size_t n = strlen(entry.filename);

  for (unsigned long i = 0; i < n; i++) {
    entry.filename[i] -= (char) (n - i + 1);
  }

  entry.length          = as::flip_endian(entry.length);
  entry.original_length = as::flip_endian(entry.original_length);
  entry.offset          = as::flip_endian(entry.offset);
}

void undeltafilter_rmt(unsigned char* buff, 
                       unsigned long  width, 
                       unsigned long  height, 
                       unsigned long  pixel_bytes = 4)
{
  unsigned char* src    = buff;
  unsigned char* dst    = buff + pixel_bytes;
  unsigned long  stride = width * pixel_bytes;

  for (unsigned long x = pixel_bytes; x < stride; x++) {
    *dst++ += *src++;
  }

  for (unsigned long y = 1; y < height; y++) {
    dst = buff + y       * stride;
    src = buff + (y - 1) * stride;

    for (unsigned long x = 0; x < stride; x++) {
      *dst++ += *src++;
    }
  }
}

void undeltafilter_akb(unsigned char* buff, 
                       unsigned long  width, 
                       unsigned long  height, 
                       unsigned long  pixel_bytes = 4)
{
  unsigned long  stride = width * pixel_bytes;
  unsigned char* src    = buff + (height - 1) * stride;
  unsigned char* dst    = src  + pixel_bytes;  

  for (unsigned long x = pixel_bytes; x < stride; x++) {
    *dst++ += *src++;
  }

  for (unsigned long y = 1; y < height; y++) {
    dst = buff + (height - y - 1) * stride;
    src = buff + (height - y)     * stride;

    for (unsigned long x = 0; x < stride; x++) {
      *dst++ += *src++;
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exbkarc v1.3 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.arc>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  ARCHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  ARCENTRY* entries = new ARCENTRY[hdr.entry_count];
  read(fd, entries, sizeof(ARCENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unobfuscate(entries[i]);

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (!memcmp(buff, "RMT", 3)) {
      RMTHDR* rmthdr = (RMTHDR*) buff;

      unsigned long  out_len  = rmthdr->width * rmthdr->height * 4;
      unsigned char* out_buff = new unsigned char[out_len];
      as::unlzss(buff + sizeof(*rmthdr), len - sizeof(*rmthdr), out_buff, out_len);

      undeltafilter_rmt(out_buff, rmthdr->width, rmthdr->height);

      as::write_bmp(as::get_file_prefix(entries[i].filename) + ".bmp",
                    out_buff,
                    out_len,
                    rmthdr->width,
                    rmthdr->height,
                    4);

      delete [] out_buff;
    } else if (!memcmp(buff, "AKB+", 4)) {
      // Process these last so (hopefully) all the bases are written
    } else if (!memcmp(buff, "AKB", 3)) {
      AKBHDR* akbhdr = (AKBHDR*) buff;

      unsigned long  pixel_bytes = akbhdr->type == AKB_TYPE_24BIT ? 3 : 4;
      unsigned long  out_stride  = akbhdr->width  * pixel_bytes;
      unsigned long  out_len     = akbhdr->height * out_stride;
      unsigned char* out_buff    = new unsigned char[out_len];

      for (unsigned long j = 0; j < out_len; j += pixel_bytes) {
        out_buff[j + 0] = akbhdr->bg_red;
        out_buff[j + 1] = akbhdr->bg_green;
        out_buff[j + 2] = akbhdr->bg_blue;

        if (pixel_bytes == 4) {
          out_buff[j + 3] = akbhdr->bg_alpha;
        }
      }
      
      unsigned long frag_width  = akbhdr->right  - akbhdr->left;
      unsigned long frag_height = akbhdr->bottom - akbhdr->top;
      unsigned long frag_stride = frag_width * pixel_bytes;
      unsigned long frag_len    = frag_height * frag_stride;

      if (frag_len) {
        unsigned char* frag_buff = new unsigned char[frag_len];
        as::unlzss(buff + sizeof(*akbhdr), len - sizeof(*akbhdr), frag_buff, frag_len);
        undeltafilter_akb(frag_buff, frag_width, frag_height, pixel_bytes);

        for (unsigned long y = 0; y < frag_height; y++) {
          memcpy(out_buff + (y + akbhdr->height - akbhdr->bottom) * out_stride + akbhdr->left * pixel_bytes,
                 frag_buff + y * frag_stride,
                 frag_stride);
        }

        delete [] frag_buff;
      }

      as::write_bmp(as::get_file_prefix(entries[i].filename) + ".bmp",
                    out_buff,
                    out_len,
                    akbhdr->width,
                    akbhdr->height,
                    pixel_bytes,
                    as::WRITE_BMP_ALIGN);

      delete [] out_buff;      
    } else {
      as::write_file(entries[i].filename, buff, len);
    }

    delete [] buff;
  }  

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (!memcmp(buff, "AKB+", 4)) {
      AKBPHDR* akbhdr = (AKBPHDR*) buff;

      unsigned long  base_len      = 0;
      unsigned char* base_buff     = NULL;
      unsigned long  base_width    = 0;
      unsigned long  base_height   = 0;
      unsigned long  base_depth    = 0;
      unsigned long  base_stride   = 0;
      as::read_bmp(akbhdr->base_name,
                   base_buff,
                   base_len,
                   base_width,
                   base_height,
                   base_depth,
                   base_stride);

      unsigned long frag_depth  = akbhdr->type == AKB_TYPE_24BIT ? 3 : 4;
      unsigned long frag_width  = akbhdr->right  - akbhdr->left;
      unsigned long frag_height = akbhdr->bottom - akbhdr->top;
      unsigned long frag_stride = frag_width * frag_depth;
      unsigned long frag_len    = frag_height * frag_stride;

      if (frag_len) {
        unsigned char* frag_buff = new unsigned char[frag_len];
        as::unlzss(buff + sizeof(*akbhdr), len - sizeof(*akbhdr), frag_buff, frag_len);
        undeltafilter_akb(frag_buff, frag_width, frag_height, frag_depth);

        for (unsigned long y = 0; y < frag_height; y++) {
          unsigned char* base_line = base_buff + (y + akbhdr->height - akbhdr->bottom) * base_stride;
          unsigned char* frag_line = frag_buff + y * frag_stride;

          for (unsigned long x = 0; x < frag_width; x++) {
            unsigned char* base_pixel = base_line + (akbhdr->left + x) * base_depth;
            unsigned char* frag_pixel = frag_line + x * frag_depth;

            if (frag_pixel[0] != 0 || frag_pixel[1] != 0xFF || frag_pixel[0] != 0) {
              memcpy(base_pixel, frag_pixel, base_depth);
            }
          }
        }

        delete [] frag_buff;
      }

      as::write_bmp(as::get_file_prefix(entries[i].filename) + ".bmp",
                    base_buff,
                    base_len,
                    base_width,
                    base_height,
                    base_depth);

      delete [] base_buff;
    }

    delete [] buff;
  }  


  delete [] entries;

  close(fd);

  return 0;
}
