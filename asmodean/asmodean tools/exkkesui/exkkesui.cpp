// exkkesui.cpp, v1.0 2009/11/18
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from ‚©‚Ì‚±‚ñ ‚¦‚·‚¢[.

#include "as-util.h"

struct DATENTRY {
  unsigned long offset;
  unsigned long length;
};

static const unsigned long DATENTRY_COUNT = 4096;

struct DATHDR {
  DATENTRY entries[DATENTRY_COUNT];
};

struct CPSHDR {
  unsigned char  signature[4]; // CPS
  unsigned long  length;
  unsigned long  unknown1;
  unsigned long  original_length;
  unsigned long  unknown2;
  unsigned short width;
  unsigned short height;
  unsigned long  depth;
  unsigned long  unknown3;

  unsigned long  unknown4;
  unsigned long  unknown5;
};

// Don't ask
unsigned long get_pal_index(unsigned long n) {  
  if ((n & 31) >= 8) {
    if ((n & 31) < 16) {
      n += 8; // +8 - 15 to +16 - 23
    } else if ((n & 31) < 24) {
      n -= 8; // +16 - 23 to +8 - 15
    }
  }

  return n;
}

void process_cps(const string&  filename,
                 unsigned char* buff, 
                 unsigned long len)
{
  static const unsigned long KEYS[]     = { 0x2623A189, 0x146FD8D7, 0x8E6F55FF, 0x1F497BCD, 
                                            0x1BB74F41, 0x0EB731D1, 0x5C031379, 0x64350881 };
  static const unsigned long KEYS_COUNT = sizeof(KEYS) / sizeof(KEYS[0]);

  CPSHDR* hdr = (CPSHDR*) buff;

  unsigned long seed_offset = *(unsigned long*) (buff + hdr->length - 4);
  seed_offset -= 0x7534682;

  unsigned long* words      = (unsigned long*) buff;  
  unsigned long  seed_index = seed_offset / 4;
  unsigned long  seed       = words[seed_index] + seed_offset + 0x3786425;
  unsigned long  n          = std::min(1023UL, len / 4);

  for (unsigned long i = 8, j = 0; i < n; i++, j++) {
    if (i != seed_index) {
      words[i] -= KEYS[j % KEYS_COUNT];
      words[i] -= seed;
      words[i] -= hdr->length;
    }

    seed *= 0x41C64E6D;
    seed += 0x9B06;
  }

  unsigned long  out_len  = hdr->original_length;
  unsigned char* out_buff = new unsigned char[out_len];

  unsigned char* in      = buff + sizeof(*hdr);
  unsigned char* end     = buff + hdr->length;

  unsigned char* out     = out_buff;
  unsigned char* out_end = out_buff + out_len;  

  while (out < out_end) {    
    unsigned long c = *in++;

    if (c & 0x80) {
      if (c & 0x40) {
        unsigned long n = (c & 0x1F) + 2;

        if (c & 0x20) {
          n += *in++ << 5;
        }

        while (n--) {
          *out++ = *in;
        }

        in++;
      } else {
        unsigned long p = ((c & 3) << 8) | *in++;
        unsigned long n = ((c >> 2) & 0xF) + 2;

        while (n--) {
          *out = *(out - p - 1);
          out++;
        }
      }
    } else {
      if (c & 0x40) {
        unsigned long r = *in++ + 1;
        unsigned long n = (c & 0x3F) + 2;

        while (r--) {
          memcpy(out, in, n);
          out += n;
        }

        in += n;
      } else {
        unsigned long n = (c & 0x1F) + 1;

        if (c & 0x20) {
          n += *in++ << 5;
        }

        while (n--) {
          *out++ = *in++;
        }
      }
    }
  }

  if (hdr->depth == 8) {
    as::RGBA* pal = (as::RGBA*) out_buff;

    as::RGBA fixed_pal[256];
    for (unsigned long i = 0; i < 256; i++) {
      fixed_pal[i] = pal[get_pal_index(i)];
    }

    unsigned long  temp_len  = hdr->width * hdr->height * 4;
    unsigned char* temp_buff = new unsigned char[temp_len];

    for (unsigned long y = 0; y < hdr->height; y++) {
      unsigned char* src_line = out_buff + 1024 + (y * hdr->width);
      as::RGBA*      dst_line = (as::RGBA*) (temp_buff + y * hdr->width * 4);

      for (unsigned long x = 0; x < hdr->width; x++) {
        dst_line[x] = fixed_pal[src_line[x]];
      }
    }
    
    delete [] out_buff;

    out_len  = temp_len;
    out_buff = temp_buff;
  }

  for (unsigned long y = 0; y < hdr->height; y++) {
    as::RGBA* line = (as::RGBA*) (out_buff + y * hdr->width * 4);

    for (unsigned long x = 0; x < hdr->width; x++) {
      if (line[x].a < 0x80) {
        line[x].a *= 2;
      } else {
        line[x].a = 0xFF;
      }
    }
  }
  
  as::write_bmp(filename,
                 out_buff,
                 out_len,
                 hdr->width,
                 hdr->height,
                 4,
                 as::WRITE_BMP_FLIP | as::WRITE_BMP_BGR);

  delete [] out_buff;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exkkesui v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  
  string prefix(as::get_file_prefix(in_filename, true));

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  DATHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  for (unsigned long i = 0; i < DATENTRY_COUNT; i++) {
    if (hdr.entries[i].length == 0) {
      continue;
    }

    unsigned long  offset = sizeof(hdr) + (hdr.entries[i].offset * 2048);
    unsigned long  len    = hdr.entries[i].length * 1024;
    unsigned char* buff   = new unsigned char[len];
    lseek(fd, offset, SEEK_SET);
    read(fd, buff, len);

    string filename = as::stringf("%s+%05d", prefix.c_str(), i);

    if (!memcmp(buff, "CPS", 3)) {
      process_cps(filename + ".bmp", buff, len);
    } else {
      as::write_file(filename, buff, len);
    }

    delete [] buff;
  }

  return 0;
}
