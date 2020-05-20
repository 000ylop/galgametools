// exescarc.cpp, v1.1 2008/01/27
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from ESC-ARC1 or ESC-ARC2 (*.bin) archives.

#include "as-util.h"

//#define ESCARC_VERSION 1
#define ESCARC_VERSION 2

struct BINHDR {
  unsigned char signature[8]; // "ESC-ARC1" or "ESC-ARC2"
  unsigned long seed;  
  unsigned long entry_count;
#if ESCARC_VERSION == 2
  unsigned long filenames_length;
#endif
};

struct BINENTRY {
#if ESCARC_VERSION == 2
  unsigned long filename_offset;
#else
  char          filename[128];
#endif
  unsigned long offset;
  unsigned long length; 
};

struct ACPHDR {
  unsigned char signature[4];    // "acp"
  unsigned long original_length; // big endian
};

unsigned long unobfuscate_long(unsigned long& value, unsigned long seed) {
  unsigned long mutator;
  unsigned long key;

  seed     ^= 0x65AC9365;
  mutator   = seed + seed;
  mutator  ^= seed;
  key       = seed;
  key     >>= 1; 
  mutator  += mutator;
  key      ^= seed;
  mutator  += mutator;
  key     >>= 3;
  mutator  += mutator;
  key      ^= seed;
  key      ^= mutator;
  value    ^= key;
  
  return key;
}

void unobfuscate(unsigned char* buff, unsigned long len, unsigned long seed) {
  unsigned long* p   = (unsigned long*) buff;
  unsigned long* end = (unsigned long*) (buff + len);  

  while (p < end) {
    seed = unobfuscate_long(*p++, seed);
  }
}

class lzw_t {
public:
  lzw_t(unsigned char* buff, unsigned long len) 
    : buff(buff),
      len(len),
      saved_count(0),
      saved_bits(0),
      want_bits(9),
      dict_index(0x103)
  {}

  unsigned long uncompress(unsigned char* out_buff, unsigned long out_len) {
    unsigned char temp_buff[65535] = { 0 };
    unsigned long temp_len         = 0;

    unsigned char* end     = buff + len;
    unsigned char* out_end = out_buff + out_len;

    while (buff < end && out_buff < out_end) {
      clear_dict();

      unsigned long index = get_bits(want_bits);
      unsigned long value = index;
      *out_buff++ = (unsigned char) value;

      unsigned long prev_index = index;
      unsigned long prev_value = value;

      while (buff < end) {
        index = get_bits(want_bits);
        value = index;

        // end of stream marker
        if (index == 0x100) {
          return out_len - (out_end - out_buff);
        }

        // extend index size marker
        if (index == 0x101) {
          want_bits++;
          continue;
        }

        // reset dictionary marker
        if (index == 0x102) {
          break;
        }

        if (value >= dict_index) {
          temp_buff[0] = (unsigned char) prev_value;
          temp_len     = 1;
          value        = prev_index;
        } else {
          temp_len = 0;
        }

        while (value > 0xFF) {
          temp_buff[temp_len++] = dict[value].value;
          value                 = dict[value].child;
        }

        temp_buff[temp_len++] = (unsigned char) value;

        while (temp_len) {
          *out_buff++ = temp_buff[temp_len-- - 1];
        }

        dict[dict_index].child = prev_index;
        dict[dict_index].value = (unsigned char) value;
        dict_index++;

        prev_index = index;
        prev_value = value;
      }
    }

    return out_len - (out_end - out_buff);
  }

private:
  unsigned long get_bits(unsigned long bits) {
    while (bits > saved_count) {
      saved_bits   = (saved_bits << 8) | *buff++;
      saved_count += 8;
    }

    unsigned long extra_bits = saved_count - bits;
    unsigned long mask       = 0xFFFFFFFF << extra_bits;
    unsigned long val        = (saved_bits & mask) >> extra_bits;

    saved_bits  &= ~mask;
    saved_count -= bits;

    return val;
  }

  void clear_dict(void) {
    want_bits  = 9;
    dict_index = 0x103;

    for (unsigned long i = 0; i < sizeof(dict) / sizeof(dict[0]); i++) {
      dict[i].child = -1;
      dict[i].value = 0;
    }
  }

  struct dict_entry_t {
    unsigned long child;
    unsigned char value;
  };

  // Don't know how many dictionary entries we need to support, so pick
  // a random "very large" number :)  Maybe only need 35023?
  dict_entry_t dict[51200];

  unsigned long want_bits;
  unsigned long dict_index;

  unsigned char* buff;
  unsigned long  len;
  unsigned long  saved_count;
  unsigned long  saved_bits;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exescarc v1.1 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bin>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  BINHDR hdr;
  read(fd, &hdr, sizeof(hdr));  
  unsigned long seed = unobfuscate_long(hdr.entry_count, hdr.seed);  

#if ESCARC_VERSION == 2
  seed               = unobfuscate_long(hdr.filenames_length, seed);  
#endif

  BINENTRY*      entries     = new BINENTRY[hdr.entry_count];
  unsigned long  entries_len = sizeof(BINENTRY) * hdr.entry_count;
  read(fd, entries, entries_len);
  unobfuscate((unsigned char*) entries, entries_len, seed);

#if ESCARC_VERSION == 2
  unsigned long filenames_len  = hdr.filenames_length;
  char*         filenames_buff = new char[filenames_len];
  read(fd, filenames_buff, filenames_len);
#endif

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

#if ESCARC_VERSION == 2
    char* filename = filenames_buff + entries[i].filename_offset;
#else
    char* filename = entries[i].filename;
#endif

    ACPHDR* acphdr = (ACPHDR*) buff;

    if (!memcmp(acphdr->signature, "acp\0", 4)) {
      unsigned long  out_len  = as::flip_endian(acphdr->original_length);
      unsigned char* out_buff = new unsigned char[out_len];

      lzw_t lzw(buff + sizeof(*acphdr), len - sizeof(*acphdr));
      unsigned long out_actual = lzw.uncompress(out_buff, out_len);

      delete [] buff;

      len  = out_len;
      buff = out_buff;
    }

    as::make_path(filename);

    int out_fd = as::open_or_die(filename,
                                 O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                                 S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

#if ESCARC_VERSION == 2
  delete [] filenames_buff;
#endif
  delete [] entries;

  close(fd);

  return 0;
}
