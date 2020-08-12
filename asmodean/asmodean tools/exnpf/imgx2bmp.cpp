// imgx2bmp.cpp, v1.0 2008/01/10
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts IMGX (*.img) images to bitmaps.

#include <windows.h>
#include "as-util.h"

struct IMGXHDR {
  unsigned char signature[4];
  unsigned long original_length;
};

struct IMGXHDR2 {
  unsigned long  header_length;
  unsigned long  width;
  unsigned long  height;
  unsigned short unknown1;
  unsigned short depth; // hopefully :)
  unsigned long  unknown3;
  unsigned long  data_length;
  unsigned long  unknown4;
  unsigned long  unknown5;  
  unsigned long  color_count;
  unsigned long  unknown6;
};

// bits from subsequent bytes are in an unintuitive order...
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

      while (buff < end && out_buff < out_end) {
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

        while (temp_len && out_buff < out_end) {
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
      saved_bits   = (*buff++ << saved_count) | saved_bits;
      saved_count += 8;
    }

    unsigned long val = saved_bits & ~(0xFFFFFFFF << bits);
    saved_bits  >>= bits;
    saved_count  -= bits;

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
  // a random "very large" number :)
  dict_entry_t dict[51200];

  unsigned long want_bits;
  unsigned long dict_index;

  unsigned char* buff;
  unsigned long  len;
  unsigned long  saved_count;
  unsigned long  saved_bits;
};

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "imgx2bmp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.img> [output.bmp]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename(as::get_file_prefix(in_filename) + ".bmp");

  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  IMGXHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = as::get_file_size(fd) - sizeof(hdr);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  unsigned long  out_len  = ~((hdr.original_length << 16) | (hdr.original_length >> 16));
  unsigned char* out_buff = new unsigned char[out_len];

  lzw_t lzw(buff, len);
  lzw.uncompress(out_buff, out_len);

  IMGXHDR2*      hdr2      = (IMGXHDR2*) out_buff;
  
  unsigned long  pal_len   = hdr2->color_count * 4;
  unsigned char* pal_buff  = (unsigned char*) (hdr2 + 1);

  // Sometimes hdr2->data_length is bad...
  unsigned long  data_len  = out_len - pal_len - sizeof(*hdr2);
  unsigned char* data_buff = pal_buff + pal_len;

  as::write_bmp_ex(out_filename, 
                   data_buff,
                   data_len,
                   hdr2->width,
                   hdr2->height,
                   hdr2->depth / 8,
                   hdr2->color_count, 
                   pal_buff, 
                   as::WRITE_BMP_FLIP);

  delete [] out_buff;
  delete [] buff;

  return 0;
}
