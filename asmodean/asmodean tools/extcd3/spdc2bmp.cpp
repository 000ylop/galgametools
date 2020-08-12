// spdc2bmp.cpp, v1.04 2010/08/01
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts SPDC images to bitmaps.

#include "as-util.h"

struct SPDCHDR {
  unsigned char  signature[4]; // "SPDC"
  unsigned short type;
  unsigned short depth;
  unsigned long  width;
  unsigned long  height;
  unsigned long  original_length;
};

// This is actually multiple flags but I don't feel like fixing it
static const unsigned long SPDC_TYPE_LZRLE0   = 0x000;
static const unsigned long SPDC_TYPE_LZ       = 0x001;
static const unsigned long SPDC_TYPE_LZRLE2   = 0x002;
static const unsigned long SPDC_TYPE_LZRLE0_2 = 0x100;
static const unsigned long SPDC_TYPE_WTF      = 0x101;
static const unsigned long SPDC_TYPE_LZRLE2_2 = 0x102;
static const unsigned long SPDC_TYPE_JPEG     = 0x103;

unsigned long DELTA_FLAG_TABLE[] = { 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
                                     0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 
                                     0x6, 0x6, 0x6, 0x6, 0x7, 0x7, 0x7, 0x7, 
                                     0xA, 0xB, 0xA, 0xB };

unsigned long DELTA_BITS_TABLE[] = { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                     5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                                     5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                                     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

unsigned char DELTA_TABLE[] = { 0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
                                0xF7, 0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1, 0xF0,
                                0x08, 0x08, 0x07, 0x07, 0x06, 0x06, 0x05, 0x05,
                                0xFB, 0xFB, 0xFA, 0xFA, 0xF9, 0xF9, 0xF8, 0xF8,
                                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                                0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD,
                                0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
                                0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
                                0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
                                0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE };

// Anybody recognize this funky algorithm?
class wtf_t {
public:
  wtf_t(unsigned char* buff, 
        unsigned long  len,
        unsigned long  width,
        unsigned long  pixel_bytes = 3)
    : buff(buff),
      len(len),
      saved_count(0),
      saved_bits(0),
      width(width),
      pixel_bytes(pixel_bytes)
  {
    long up_pixel = -1 * width * pixel_bytes;

    for (unsigned long i =  0; i < 16; i++) dist_table[i] = -1 * pixel_bytes;
    for (unsigned long i = 16; i < 24; i++) dist_table[i] = up_pixel;
    for (unsigned long i = 24; i < 26; i++) dist_table[i] = up_pixel - pixel_bytes;
    for (unsigned long i = 26; i < 28; i++) dist_table[i] = up_pixel + pixel_bytes;
  }

  void uncompress(unsigned char* out_buff, unsigned long out_len) {
    unsigned char* out_end = out_buff + out_len;

    out_buff[0] = (unsigned char) get_bits(8);
    out_buff[1] = (unsigned char) get_bits(8);
    out_buff[2] = (unsigned char) get_bits(8);
    out_buff += pixel_bytes;

    while (out_buff < out_end) {
      unsigned long val = get_bits(5, false);

      if (val > 0x1B) {
        get_bits(3);
        out_buff[2] = (unsigned char) get_bits(8);
        out_buff[1] = (unsigned char) get_bits(8);
        out_buff[0] = (unsigned char) get_bits(8);
      } else {
        unsigned char* src = out_buff + dist_table[val];

        out_buff[0] = src[0];
        out_buff[1] = src[1];
        out_buff[2] = src[2];

        get_bits(DELTA_FLAG_TABLE[val] >> 1);

        if (DELTA_FLAG_TABLE[val] & 0x01) {
          unsigned long index1 = get_bits(8, false);
          unsigned long index2 = get_bits(8 + DELTA_BITS_TABLE[index1], false) & 0xFF;
          unsigned long index3 = get_bits(8 + DELTA_BITS_TABLE[index1] + DELTA_BITS_TABLE[index2], false) & 0xFF;          

          out_buff[0] += DELTA_TABLE[index1];
          out_buff[1] += DELTA_TABLE[index1] + DELTA_TABLE[index2];
          out_buff[2] += DELTA_TABLE[index1] + DELTA_TABLE[index3];

          get_bits(DELTA_BITS_TABLE[index1] + DELTA_BITS_TABLE[index2] + DELTA_BITS_TABLE[index3]);
        }
      }

      out_buff += pixel_bytes;
    }
  }

private:
  unsigned long get_bits(unsigned long bits, bool eat_bits = true) {
    while (bits > saved_count) {
      saved_bits   = (saved_bits << 8) | *buff++;
      saved_count += 8;
    }

    unsigned long extra_bits = saved_count - bits;
    unsigned long mask       = 0xFFFFFFFF << extra_bits;
    unsigned long val        = (saved_bits & mask) >> extra_bits;

    if (eat_bits) {
      saved_bits  &= ~mask;
      saved_count -= bits;
    }

    return val;
  }

  unsigned char* buff;
  unsigned long  len;
  unsigned long  saved_count;
  unsigned long  saved_bits;

  unsigned long  width;
  long           dist_table[28];
  // don't really support anything but 3-byte pixels though...
  unsigned long  pixel_bytes;
};

unsigned long unlz(unsigned char* buff,
                   unsigned long  len,
                   unsigned char* out_buff, 
                   unsigned long  out_len)
{
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;

  memset(out_buff, 0, out_len);

  while (buff < end && out_buff < out_end) {
    unsigned char flags = *buff++;

    for (int i = 0; i < 8 && buff < end && out_buff < out_end; i++) {
      if (flags & 0x01) {
        *out_buff++ = *buff++;
      } else {
        if (end - buff < 2)
          break;
        
        unsigned long n = *buff++;
        unsigned long p = *buff++ << 4;
                                
        p |= (n & 0xF0) >> 4;
        n  = (n & 0x0F) + 3;

        unsigned char* src = out_buff - p;

        for (unsigned long j = 0; j < n && out_buff < out_end; j++) {
          *out_buff++ = *src++;
        }
      }

      flags >>= 1;
    }
  }

  return out_len - (out_end - out_buff);
}

void unrle2(unsigned char* buff,
           unsigned long  len,
           unsigned char* out_buff,
           unsigned long  out_len)
{
  struct RLEHDR {
    unsigned long rgb_offset;
    unsigned long pixel_count;
  };

  RLEHDR* hdr = (RLEHDR*) buff;

  unsigned char* cmd_buff = buff + sizeof(*hdr);
  unsigned char* rgb_buff = buff + hdr->rgb_offset;

  unsigned char* out_end = out_buff + out_len;

  memset(out_buff, 0, out_len);

  while (out_buff < out_end) {
    unsigned short n = *(unsigned short*) cmd_buff;
    cmd_buff += 2;

    unsigned char type = n >> 14;

    n &= 0x3FFF;

    while (n--) {
      switch (type) {
        case 0:
          out_buff += 4;
          break;

        case 1:
          *out_buff++ = *rgb_buff++;
          *out_buff++ = *rgb_buff++;
          *out_buff++ = *rgb_buff++;
          *out_buff++ = 0xFF;
          break;

        default:
          *out_buff++ = *rgb_buff++;
          *out_buff++ = *rgb_buff++;
          *out_buff++ = *rgb_buff++;
          *out_buff++ = *cmd_buff++;
      }
    }
  }
}

void unrle0(unsigned char* buff,
           unsigned long  len,
           unsigned char* out_buff,
           unsigned long  out_len)
{
  struct RLEHDR {
    unsigned long rgb_offset;
    unsigned long pixel_count; // ??
    unsigned long initial_skip;
  };

  RLEHDR* hdr = (RLEHDR*) buff;

  unsigned char* cmd_buff = buff + sizeof(*hdr);
  unsigned char* rgb_buff = buff + hdr->rgb_offset;
  bool           skip     = hdr->initial_skip == 0;

  unsigned char* out_end = out_buff + out_len;

  memset(out_buff, 0, out_len);

  while (out_buff < out_end) {
    unsigned long n = *(unsigned long*) cmd_buff;
    cmd_buff += 4;

    while (n--) {
      if (skip) {
        out_buff += 4;
      } else {
        *out_buff++ = *rgb_buff++;
        *out_buff++ = *rgb_buff++;
        *out_buff++ = *rgb_buff++;
        *out_buff++ = 0xFF;        
      }
    }

    skip = !skip;
  }
}

void unobfuscate(SPDCHDR& hdr) {
  if (memcmp(hdr.signature, "SPDC", 4)) {
    for (unsigned long i = 0; i < sizeof(hdr.signature); i++) {
      hdr.signature[i] -= (unsigned char) ((hdr.original_length      ) & 0xFF);
      hdr.signature[i] -= (unsigned char) ((hdr.original_length >> 16) & 0xFF);
    }
  }

  unsigned long* p = (unsigned long*) &hdr;

  p[1] -= (hdr.original_length << 4) & 0xFFFF;
  p[2] -= (hdr.original_length << 2) & 0x137F;
  p[3] -= (hdr.original_length >> 2) & 0xF731;
}

void unobfuscate_jpeg(unsigned char* buff, unsigned long len) {
  unsigned long* words = (unsigned long*) buff;

  for (unsigned long i = 0; i < 16; i++) {
    words[i] -= 0x5769E10F;
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "spdc2bmp v1.04 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input> [output.bmp]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename = as::get_file_prefix(in_filename) + ".bmp";

  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  SPDCHDR hdr;
  read(fd, &hdr, sizeof(hdr));  
  unobfuscate(hdr);

  if (memcmp(hdr.signature, "SPDC", 4)) {
    fprintf(stderr, "%s: unrecognized type\n", in_filename.c_str());
    return -1;
  }

  unsigned long  len  = as::get_file_size(fd) - sizeof(hdr);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  unsigned long  out_len  = hdr.original_length;
  unsigned char* out_buff = new unsigned char[out_len];
  memset(out_buff, 0, out_len);

  switch (hdr.type) {    
    case SPDC_TYPE_LZ:
      unlz(buff, len, out_buff, out_len);

      as::write_bmp(out_filename,
                    out_buff,
                    out_len,
                    hdr.width,
                    hdr.height,
                    3,
                    as::WRITE_BMP_FLIP | as::WRITE_BMP_ALIGN);
      break;

    case SPDC_TYPE_LZRLE0:
    case SPDC_TYPE_LZRLE0_2:
    case SPDC_TYPE_LZRLE2:
    case SPDC_TYPE_LZRLE2_2:
      {
        unlz(buff, len, out_buff, out_len);

        unsigned long  rgb_len  = hdr.width * hdr.height * 4;
        unsigned char* rgb_buff = new unsigned char[rgb_len];

        if (hdr.type == SPDC_TYPE_LZRLE0 || hdr.type == SPDC_TYPE_LZRLE0_2) {
          unrle0(out_buff, out_len, rgb_buff, rgb_len);
        } else {
          unrle2(out_buff, out_len, rgb_buff, rgb_len);
        }

        as::write_bmp(out_filename,
                      rgb_buff,
                      rgb_len,
                      hdr.width,
                      hdr.height,
                      4,
                      as::WRITE_BMP_FLIP | as::WRITE_BMP_ALIGN);

        delete [] rgb_buff;
      }
      break;

    case SPDC_TYPE_WTF:
      {
        wtf_t wtf(buff, len, hdr.width, hdr.depth / 8);
        wtf.uncompress(out_buff, out_len);

        as::write_bmp(out_filename,
                      out_buff,
                      out_len,
                      hdr.width,
                      hdr.height,
                      hdr.depth / 8,
                      as::WRITE_BMP_FLIP | as::WRITE_BMP_ALIGN);
      }
      break;

    case SPDC_TYPE_JPEG:
      unobfuscate_jpeg(buff, len);

      // First word is the length, seems useless (no multipart files)
      as::write_file(as::get_file_prefix(out_filename) + ".jpg",
                     buff + 4,
                     len - 4);
      break;

    default:
      printf("%s: unsupported type %d\n", in_filename.c_str(), hdr.type);
  }

  delete [] out_buff;
  delete [] buff;

  return 0;
}
