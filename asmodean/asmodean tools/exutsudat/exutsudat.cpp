// exutsudat.cpp, v1.0 2007/10/27
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from CARMINE's ãÛê‰Ç…êGÇÍÇÈÇ‡ÇÃ.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>
#include "zlib.h"

using std::string;

static const unsigned long TOC_OFFSET = 0x79221CB;

struct DATHDR {
  unsigned long section_count;
  unsigned long total_entries;
};

struct DATSECT {
  unsigned long offset;
  unsigned long entry_count;
};

struct DATENTRY {
  unsigned long length;
};

struct DATDATAHDR {
  unsigned long type;
};

struct CXHDR {
  unsigned char  signature[4]; // "CF" or "CX"
  unsigned long  unknown;
  unsigned short width;
  unsigned short height;
  unsigned long  length;
};

struct CPHDR {
  unsigned char  signature[2]; // "CP"
  unsigned short depth_bytes;
  unsigned long  unknown1;
  unsigned long  unknown2;
  unsigned long  width;
  unsigned long  height;
  unsigned long  frame_count;
};

unsigned long uncx(unsigned char* buff,
                   unsigned long  len,
                   unsigned char* out_buff,
                   unsigned long  out_len,
                   unsigned long  pixel_bytes,
                   unsigned long  stride)
{
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;

  unsigned long last_cmd = 0;

  while (out_buff < out_end) {
    unsigned long  n   = 0;
    unsigned long  p   = 0;
    unsigned long  c   = *buff++;
    unsigned long  cmd = c >> 6;
    unsigned char* src = NULL;

    switch (cmd) {
    // literals
    case 0:
      n = (c & 0x3F) + 1;
      while (n-- && out_buff < out_end) *out_buff++ = *buff++;
      break;

    // RLE
    case 1:
      n = (c & 0x3F) + 2;
      while (n-- && out_buff < out_end) *out_buff++ = *buff;
      buff++;
      break;

    // extended command
    case 2:
      cmd = (c >> 4) & 0x3;
      n   = ((c & 0x0F) << 8) | *buff++;

      switch (cmd) {
      // extended RLE
      case 0:
        n += 2;
        while (n-- && out_buff < out_end) *out_buff++ = *buff;
        buff++;
        break;

      // line backreference (1, 2, 4 lines)
      default:
        n++;
        p   = stride << (cmd - 1);
        src = out_buff - p;
        while (n-- && out_buff < out_end) *out_buff++ = *src++;
      }
      break;

    // block backreference or block RLE
    case 3:
      cmd = (c >> 4) & 0x3;      

      switch (cmd) {
      // RLE 3 or 6 byte block
      case 0:
        {
          n                        = ((c & 0x07) << 8) + *buff++ + 2;
          unsigned long block_size = c & 0x08 ? (pixel_bytes * 2) : pixel_bytes;

          while (n--) {
            for (unsigned long i = 0; i < block_size && out_buff < out_end; i++) {
              *out_buff++ = buff[i];
            }
          }

          buff += block_size;
        }
        break;

      // backreference (3 byte block)
      case 1:
        p   = ((c & 0x0F) << 8) + *buff++ + 1;
        p  *= pixel_bytes;

        n   = *buff++ + 1;
        n  *= pixel_bytes;

        src = out_buff - p;
        while (n-- && out_buff < out_end) *out_buff++ = *src++;
        break;

      // backreference (1 byte block)
      case 2:
        p   = ((c & 0x0F) << 8) + *buff++ + 1;
        n   = *buff++ + 1;
        src = out_buff - p;
        while (n-- && out_buff < out_end) *out_buff++ = *src++;
        break;
      case 4:
        printf("unexpected code, probably unsupported compression!\n");
        return 0;
        break;
      }
    }
  }

  return out_len - (out_end - out_buff);
}

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

void write_bmp(const    string& filename,
               unsigned char*   buff,
               unsigned long    len,
               unsigned long    width,
               unsigned long    height,
               unsigned short   depth_bytes)
{
  BITMAPFILEHEADER bmf;
  BITMAPINFOHEADER bmi;

  memset(&bmf, 0, sizeof(bmf));
  memset(&bmi, 0, sizeof(bmi));

  bmf.bfType     = 0x4D42;
  bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + len;
  bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi);

  bmi.biSize     = sizeof(bmi);
  bmi.biWidth    = width;
  bmi.biHeight   = height;
  bmi.biPlanes   = 1;
  bmi.biBitCount = depth_bytes * 8;
           
  int fd = open_or_die(filename + ".bmp", 
                       O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                       S_IREAD | S_IWRITE);
  write(fd, &bmf, sizeof(bmf));
  write(fd, &bmi, sizeof(bmi));
  write(fd, buff, len);
  close(fd);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exutsudat v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <archpac.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int toc_fd = open_or_die(in_filename, O_RDONLY | O_BINARY);
  int dat_fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  DATHDR hdr;
  lseek(toc_fd, TOC_OFFSET, SEEK_SET);
  read(toc_fd, &hdr, sizeof(hdr));

  DATSECT* sections = new DATSECT[hdr.section_count];
  read(toc_fd, sections, sizeof(DATSECT) * hdr.section_count);

  for (long i = hdr.section_count - 1; i >= 0; i--) {
    unsigned long entry_count = sections[i].entry_count + 1;

    DATENTRY* entries = new DATENTRY[entry_count];
    read(toc_fd, entries, sizeof(DATENTRY) * entry_count);

    // Only have to seek to the section to read sequentially
    lseek(dat_fd, sections[i].offset, SEEK_SET);

    for (unsigned long j = 0; j < entry_count - 1; j++) {
      char filename[4096] = { 0 };
      sprintf(filename, "%03d+%05d", i, j);

      unsigned long  len  = entries[j + 1].length - entries[j].length;

      if (len == 0) {
        continue;
      }

      unsigned char* buff = new unsigned char[len];
      read(dat_fd, buff, len);

      // Not sure how much data to expect (grr)
      unsigned long  out_len  = 1024 * 1024 * 20;
      unsigned char* out_buff = new unsigned char[out_len];

      DATDATAHDR* datahdr = (DATDATAHDR*) buff;

      if (datahdr->type == 1) {
        uncompress(out_buff, &out_len, buff + sizeof(*datahdr), len - sizeof(*datahdr));
      } else {
        printf("%s: skipping dummy(?) data\n", filename);
        delete [] out_buff;
        delete [] buff;
        continue;
      }      

      bool is_cf = !memcmp(out_buff, "CF", 2);
      bool is_cx = !memcmp(out_buff, "CX", 2);
      bool is_cp = !memcmp(out_buff, "CP", 2);

      if (is_cf || is_cx) {
        CXHDR*         cxhdr      = (CXHDR*) out_buff;
        unsigned long  cxdata_len = cxhdr->length;
        unsigned char* cxdata     = (unsigned char*) (cxhdr + 1);

        unsigned short depth      = is_cx ? 4 : 3;
        unsigned long  rgb_stride = cxhdr->width * depth;
        unsigned long  rgb_len    = cxhdr->height * rgb_stride;
        unsigned char* rgb_buff   = new unsigned char[rgb_len];

        uncx(cxdata, cxdata_len, rgb_buff, rgb_len, depth, rgb_stride);

        write_bmp(string(filename),
                  rgb_buff,
                  rgb_len,
                  cxhdr->width,
                  cxhdr->height,
                  depth);

        delete [] rgb_buff;
      } else if (is_cp) {
        CPHDR*         cphdr      = (CPHDR*) out_buff;
        unsigned char* frame_buff = (unsigned char*) (cphdr + 1);
        unsigned long  stride     = cphdr->width * cphdr->depth_bytes;
        unsigned long  frame_len  = cphdr->height * stride;
        unsigned char* flip_buff  = new unsigned char[frame_len];

        for (unsigned long k = 0; k < cphdr->frame_count; k++) {
          char frame_filename[4096] = { 0 };
          sprintf(frame_filename, "%s+%03d", filename, k);

          for (unsigned long y = 0; y < cphdr->height; y++) {
            unsigned char* src_line = frame_buff + y * stride;
            unsigned char* dst_line = flip_buff  + (cphdr->height - y - 1) * stride;
            memcpy(dst_line, src_line, stride);
          }

          write_bmp(frame_filename,
                    flip_buff,
                    frame_len,
                    cphdr->width,
                    cphdr->height,
                    (unsigned short) cphdr->depth_bytes);

          frame_buff += frame_len;
        }

        delete [] flip_buff;
      } else {
        printf("%s: skipping unsupported type %02x %02x (%d bytes)\n", filename, out_buff[0], out_buff[1], out_len);
      }

      delete [] out_buff;
      delete [] buff;
    }

    delete [] entries;
  }

  delete [] sections;

  close(toc_fd);
  close(dat_fd);

  return 0;
}
