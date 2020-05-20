// exk5.cpp, v1.0 2010/12/05
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts K5 (*.K5) archives.

#include "as-util.h"

struct K5HDR {
  unsigned char  signature[2]; // "K5"
  unsigned short version;
  unsigned long  entry_count;
  unsigned long  toc_offset;
  unsigned short unknown1;
  unsigned char  unknown2;
  unsigned char  depth;
};

struct K5ENTRY {
  unsigned char unknown1[128];
  wchar_t       filename[32];
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long offset;
  unsigned long length;
  unsigned long unknown4;
  unsigned long length2;
  unsigned long unknown5;
  unsigned long unknown6[9];
};

struct K4HDR1 {
  unsigned char  signature[2]; // K4
  unsigned short version;
  unsigned short width;
  unsigned short height;
  unsigned long  unknown1;
  unsigned short type;
  unsigned char  unknown2;
  unsigned char  depth;
  unsigned long  unknown3;
  unsigned long  unknown4;
  unsigned long  length;
  unsigned long  unknown5[5];
};

struct K4HDR2 {
  unsigned short width;
  unsigned short height;
  unsigned long  unknown1;
  unsigned long  unknown2;
  unsigned short depth;
  unsigned short flags;
  unsigned long  original_length;
  unsigned long  alpha_offset;
  unsigned long  alpha_length;
  unsigned long  unknown3;
  unsigned long  unknown4;
  unsigned long  data_offset;
};

static const unsigned long K4HDR2_FLAG_DELTA = 0x0001;

bool proc_k4(const string& filename, unsigned char* buff, unsigned long len) {
  // I've seen one entirely bogus file...
  if (len < sizeof(K4HDR1) + sizeof(K4HDR2)) {
    return false;
  }

  if (memcmp(buff, "K4", 2) != 0) {
    return false;
  }

  K4HDR1* hdr1 = (K4HDR1*) buff;
  K4HDR2* hdr2 = (K4HDR2*) (hdr1 + 1);

  if (hdr1->type != 1) {
    return false;
  }

  unsigned char* flags_buff = (unsigned char*) (hdr2 + 1);
  unsigned long  flags_len  = hdr2->data_offset - 16;

  unsigned char* data_buff  = flags_buff + flags_len;
  unsigned long  data_len   = len - (data_buff - buff);

  as::bitbuff_t flags(flags_buff, flags_len);
  as::bitbuff_t data(data_buff, data_len);

  unsigned long  pixel_bytes = hdr2->depth / 8;
  unsigned long  rgb_stride  = (hdr1->width * pixel_bytes + 3) & ~3;  
  unsigned long  rgb_len     = hdr1->height * rgb_stride;
  unsigned char* rgb_buff    = new unsigned char[rgb_len];
  memset(rgb_buff, 0, rgb_len);

  unsigned char* rgb_p   = rgb_buff;
  unsigned char* rgb_end = rgb_p + rgb_len;

  bool do_delta = (hdr2->flags & K4HDR2_FLAG_DELTA) != 0;

  while (rgb_p < rgb_end && !flags.empty()) {
    if (flags.get_bits(1)) {
      if (do_delta) {
        if ((unsigned long) (rgb_p - rgb_buff) < pixel_bytes) {
          *rgb_p++ = (unsigned char) data.get_bits(9);
        } else {
          *rgb_p = (unsigned char) (*(rgb_p - pixel_bytes) + data.get_bits(9) + 1);
          rgb_p++;
        }
      } else {
        *rgb_p++ = (unsigned char) data.get_bits(8);
      }
    } else {
      unsigned long p = 0;
      unsigned long n = 0;      

      if (flags.get_bits(1)) {
        p = data.get_bits(14);
        n = data.get_bits(4) + 3;
      } else {
        p = data.get_bits(9);
        n = data.get_bits(3) + 2;
      }

      unsigned char* src = rgb_p - p - 1;

      while (n-- && rgb_p < rgb_end) {
        if (!do_delta || (unsigned long) (rgb_p - rgb_buff) < pixel_bytes) {
          *rgb_p++ = *src++;
        } else {
          *rgb_p++ = *src + *(src + p - pixel_bytes + 1) - *(src - pixel_bytes);
          src++;
        }
      }
    }
  }

  string out_filename = as::get_file_prefix(filename) + ".bmp";

  // Let's hope there's never 8-bit images with alpha...
  if (hdr2->alpha_offset) {
    unsigned char* alpha_buff  = buff + sizeof(*hdr1) + hdr2->alpha_offset;
    unsigned long* offsets     = (unsigned long*) alpha_buff;

    unsigned long  rgba_stride = hdr1->width * 4;
    unsigned long  rgba_len    = hdr1->height * rgba_stride;
    unsigned char* rgba_buff   = new unsigned char[rgba_len];

    for (unsigned long y = 0; y < hdr1->height; y++) {
      unsigned char* alpha_line = alpha_buff + offsets[hdr1->height - y - 1];

      as::RGB*  rgb_line  = (as::RGB*)  (rgb_buff  + y * rgb_stride);
      as::RGBA* rgba_line = (as::RGBA*) (rgba_buff + y * rgba_stride);

      for (unsigned long x = 0; x < hdr1->width; x++) {
        rgba_line[x] = rgb_line[x];
      }

      for (unsigned long x = 0; x < hdr1->width;) {        
        unsigned char c = *alpha_line++;
        unsigned char n = *alpha_line++;

        if (c == 0x80) {
          c = 0xFF;
        } else {
          c *= 2;
        }

        while (n-- && x < hdr1->width) {
          rgba_line[x++].a = c;
        }
      }
    }

    as::write_bmp(out_filename,
                  rgba_buff,
                  rgba_len,
                  hdr1->width,
                  hdr1->height,
                  4);

    delete [] rgba_buff;

  } else {
    as::write_bmp(out_filename,
                  rgb_buff,
                  rgb_len,
                  hdr1->width,
                  hdr1->height,
                  pixel_bytes);
  }

  delete [] rgb_buff;

  return true;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exk5 v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.K5>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  K5HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  K5ENTRY* entries = new K5ENTRY[hdr.entry_count];
  lseek(fd, hdr.toc_offset, SEEK_SET);
  read(fd, entries, sizeof(K5ENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    string filename = as::convert_wchar(entries[i].filename);

    if (!proc_k4(filename, buff, len)) {      
      as::write_file(filename, buff, len);
    }

    delete [] buff;
  }
  
  delete [] entries;

  close(fd);

  return 0;
}