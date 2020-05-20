// ge2bmp2.cpp, v1.0 2008/12/28
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts GE and PGD3 (*.pgd) images from M.E.s.  Use ge2bmp for
// other varieties of this format.

#include "as-util.h"

struct GEHDR {
  unsigned char  signature[2]; // "GE"
  unsigned short unknown1;
  unsigned long  unknown3;
  unsigned long  unknown4;
  unsigned long  width;
  unsigned long  height;
  unsigned long  unknown5;
  unsigned long  unknown6;
};

struct DATAHDR {
  unsigned short filter_type;
  unsigned short unknown;
  unsigned long  original_length;
  unsigned long  length;
};

struct PGAINNERHDR {
  unsigned short unknown1;
  unsigned short depth;
  unsigned short width;
  unsigned short height;
};

// Fragments
#pragma pack(1)
struct PGA3HDR {
  unsigned char  signature[4]; // "PGA3"
  unsigned short offset_x;
  unsigned short offset_y;
  unsigned short width;
  unsigned short height;
  unsigned short depth;
  char           filename[30];
};
#pragma pack()

void unge2(unsigned char* buff,
           unsigned long  len,
           unsigned char* out_buff,
           unsigned long  out_len)
{  
  unsigned char* out_start = out_buff;
  unsigned char* out_end   = out_buff + out_len;

  while (out_buff < out_end) {
    unsigned char flags = *buff++;

    for (unsigned long i = 0; i < 8 && out_buff < out_end; i++) {
      unsigned long  n   = 0;
      unsigned char* src = NULL;

      if (flags & 1) {
        unsigned long p = *(unsigned short*) buff;
        buff += 2;

        if (p & 8) {          
          n = (p & 7) + 4;
          p >>= 4;  
        } else {
          p = (p << 8) | *buff++;

          n  = ((p & 0xFFC) >> 2) + 1;
          n *= 4;
          n += p & 3;

          p >>= 12;
        }

        src = out_buff - p;
      } else {
        n = *buff++;
        n = (n >> 2) * 4 + (n & 3);

        src = buff;
        buff += n;
      }

      while (n--) {
        *out_buff++ = *src++;
      }

      flags >>= 1;
    }
  }
}

void undeltafilter(unsigned char* cmds, 
                   unsigned long  width,
                   unsigned long  height, 
                   unsigned long  pixel_bytes,
                   unsigned char* buff)
{
  unsigned long stride = width * pixel_bytes;

  for (unsigned long y = 0; y < height; y++) {
    unsigned char* prev_line = buff + (y  - 1) * stride;
    unsigned char* dst_line  = buff + y * stride;

    switch (cmds[y]) {
      case 1:
        for (unsigned long x = pixel_bytes; x < stride; x++) {
          dst_line[x] = dst_line[x - pixel_bytes] - dst_line[x];
        }
        break;

      case 2:
        for (unsigned long x = 0; x < stride; x++) {
          dst_line[x] = prev_line[x] - dst_line[x];                    
        }
        break;

      case 4:
        for (unsigned long x = pixel_bytes; x < stride; x++) {
          dst_line[x] = (prev_line[x] + dst_line[x - pixel_bytes]) / 2 - dst_line[x];          
        }
        break;
    }
  }
}

inline void clamp_color(long& v) {
  if (v <= 255) {
    if (v < 0) {
      v = 0;
    }
  } else {
    v = 255;
  }
}

inline void pack_pixel(unsigned char* p, long r, long g, long b) {
  *p++ = (unsigned char) r;
  *p++ = (unsigned char) g;
  *p++ = (unsigned char) b;
}

void unfilter_type2(unsigned long  width,
                    unsigned long  height, 
                    unsigned char* buff,
                    unsigned char* out_buff)
{
  unsigned long out_stride  = width * 3;
  unsigned long sect_len    = width * height / 4;
  char*         p1          = (char*) buff;
  char*         p2          = p1 + sect_len;
  char*         p3          = p2 + sect_len;

  for (unsigned long y = 0; y < height / 2; y++) {
    for (unsigned long x = 0; x < width / 2; x++) {
      long p1_pix = *p1++;
      long p2_pix = *p2++;

      long p1_m         = 226 * p1_pix;
      long p1_p2_m      = -43 * p1_pix - 89 * p2_pix;
      long p2_m         = 179 * p2_pix;

      long p3_s         = (unsigned char)*p3 << 7;
      long p1_p3_ms     = p1_m + p3_s;
      long p3_p1_p2_sm  = p3_s + p1_p2_m;
      long p2_p3_mss    = (p2_m + p3_s) >> 7;
      long p1_p3_mss    = p1_p3_ms >> 7;
      long p3_p1_p2_sms = p3_p1_p2_sm >> 7;

      clamp_color(p2_p3_mss);
      clamp_color(p3_p1_p2_sms);
      clamp_color(p1_p3_mss);
      pack_pixel(out_buff, p1_p3_mss, p3_p1_p2_sms, p2_p3_mss);

      long p32_s         = (unsigned char)*(p3 + 1) << 7;
      long p32_p1_p2_sm  = p32_s + p1_p2_m;
      long p1_p32_ms     = p1_m + p32_s;
      long p2_p32_mss    = (p2_m + p32_s) >> 7;
      long p1_p32_mss    = p1_p32_ms >> 7;
      long p32_p1_p2_sms = p32_p1_p2_sm >> 7;

      clamp_color(p2_p32_mss);
      clamp_color(p32_p1_p2_sms);
      clamp_color(p1_p32_mss);
      pack_pixel(out_buff + 3, p1_p32_mss, p32_p1_p2_sms, p2_p32_mss);

      long p33_s         = (unsigned char)*(p3 + width) << 7;
      long p33_p1_p2_sm  = p33_s + p1_p2_m;
      long p1_p33_ms     = p1_m + p33_s;
      long p2_p33_mss    = (p2_m + p33_s) >> 7;
      long p1_p33_mss    = p1_p33_ms >> 7;
      long p33_p1_p2_sms = p33_p1_p2_sm >> 7;

      clamp_color(p2_p33_mss);
      clamp_color(p33_p1_p2_sms);
      clamp_color(p1_p33_mss);
      pack_pixel(out_buff + out_stride, p1_p33_mss, p33_p1_p2_sms, p2_p33_mss);

      long p34_s         = (unsigned char)*(p3 + width + 1) << 7;
      long p34_p1_p2_sm  = p34_s + p1_p2_m;
      long p1_p34_ms     = p1_m + p34_s;
      long p2_p34_mss    = (p2_m + p34_s) >> 7;
      long p1_p34_mss    = p1_p34_ms >> 7;
      long p34_p1_p2_sms = p34_p1_p2_sm >> 7;

      clamp_color(p2_p34_mss);
      clamp_color(p34_p1_p2_sms);
      clamp_color(p1_p34_mss);
      pack_pixel(out_buff + out_stride + 3, p1_p34_mss, p34_p1_p2_sms, p2_p34_mss);     

      p3       += 2;
      out_buff += 6;
    }

    p3       += width;
    out_buff += out_stride;
  }
}

void load_pga(const string&   filename,
              unsigned long&  width,
              unsigned long&  height,
              unsigned long&  pixel_bytes,
              unsigned char*& ret_buff,
              unsigned long&  ret_len)
{
  int fd = as::open_or_die(filename, O_RDONLY | O_BINARY);

  GEHDR   gehdr   = { 0 };
  PGA3HDR pgd3hdr = { 0 };

  bool is_ge   = false;
  bool is_pgd3 = false;

  {
    unsigned char temp[4] = { 0 };
    read(fd, temp, sizeof(temp));
    lseek(fd, 0, SEEK_SET);

    is_pgd3 = !memcmp(temp, "PGD3", 4);
    is_ge   = !memcmp(temp, "GE", 2);
  }

  if (is_ge) {
    read(fd, &gehdr, sizeof(gehdr));
  } else if (is_pgd3) {
    read(fd, &pgd3hdr, sizeof(pgd3hdr));
  } else {
    fprintf(stderr, "%s: unsupported type.\n", filename.c_str());
    exit(0);
  }

  DATAHDR datahdr;
  read(fd, &datahdr, sizeof(datahdr));

  unsigned long  len  = datahdr.length;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  unsigned long  out_len  = datahdr.original_length;
  unsigned char* out_buff = new unsigned char[out_len];
  unge2(buff, len, out_buff, out_len);

  unsigned char* delta_cmds = NULL;

  if (is_ge) {
    switch (datahdr.filter_type) {
      case 2:
        width       = gehdr.width;
        height      = gehdr.height;
        pixel_bytes = 3;

        {
          unsigned long  final_len  = width * height * pixel_bytes;
          unsigned char* final_buff = new unsigned char[final_len];
          unfilter_type2(width, height, out_buff, final_buff);

          delete [] out_buff;

          out_len  = final_len;
          out_buff = final_buff;
        }
        break;

      case 3:
        {
          PGAINNERHDR* pgahdr = (PGAINNERHDR*) out_buff;

          width       = pgahdr->width;
          height      = pgahdr->height;
          pixel_bytes = pgahdr->depth / 8;
          delta_cmds  = (unsigned char*) (pgahdr + 1);
        }
        break;

      case 1:
        // There's an algorithm for type 1 but I hope nothing uses it ... :P

      default:
        fprintf(stderr, "%s: unsupported filter type %d\n", filename.c_str(), datahdr.filter_type);
        exit(0);
        break;
    }
  }

  if (is_pgd3) {
    width       = pgd3hdr.width;
    height      = pgd3hdr.height;
    pixel_bytes = pgd3hdr.depth / 8;
    delta_cmds  = out_buff;
  }

  unsigned long  pixel_len = out_len;
  unsigned char* pixels    = out_buff;

  if (delta_cmds) {
    pixel_len = width * height * pixel_bytes;
    pixels    = delta_cmds + height;

    undeltafilter(delta_cmds, width, height, pixel_bytes, pixels);
  }

  if (is_pgd3) {
    unsigned long  base_width       = 0;
    unsigned long  base_height      = 0;
    unsigned long  base_pixel_bytes = 0;
    unsigned long  base_len         = 0;
    unsigned char* base_buff        = NULL;

    // In theory recursive fragments would work, but I don't think there are any.
    load_pga(pgd3hdr.filename, 
             base_width,
             base_height,
             base_pixel_bytes,
             base_buff,
             base_len);

    unsigned long stride      = width      * pixel_bytes;
    unsigned long base_stride = base_width * base_pixel_bytes;

    for (unsigned long y = 0; y < height; y++) {
      unsigned char* src_line = pixels    + y * stride;
      unsigned char* dst_line = base_buff + (y + pgd3hdr.offset_y) * base_stride;

      for (unsigned long x = 0; x < width; x++) {
        // Yes, XOR.  WTF?
        dst_line[(pgd3hdr.offset_x + x) * base_pixel_bytes + 0] ^= src_line[x * pixel_bytes + 0];
        dst_line[(pgd3hdr.offset_x + x) * base_pixel_bytes + 1] ^= src_line[x * pixel_bytes + 1];
        dst_line[(pgd3hdr.offset_x + x) * base_pixel_bytes + 2] ^= src_line[x * pixel_bytes + 2];

        if (pixel_bytes == 4) {
          dst_line[(pgd3hdr.offset_x + x) * base_pixel_bytes + 3] ^= src_line[x * pixel_bytes + 3];
        }
      }
    }

    width       = base_width;
    height      = base_height;
    pixel_bytes = base_pixel_bytes;
    ret_len     = base_len;
    ret_buff    = base_buff;
  } else {
    ret_len  = pixel_len;
    ret_buff = new unsigned char[ret_len];
    memcpy(ret_buff, pixels, ret_len);
  }

  delete [] out_buff;
  delete [] buff;
}


int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "ge2bmp2 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pgd> [output.bmp]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename = as::get_file_prefix(in_filename) + ".bmp";

  if (argc > 2) {
    out_filename = argv[2];
  }

  unsigned long  width       = 0;
  unsigned long  height      = 0;
  unsigned long  pixel_bytes = 0;
  unsigned long  len         = 0;
  unsigned char* buff        = NULL;

  load_pga(in_filename, width, height, pixel_bytes, buff, len);

  as::write_bmp(out_filename,
                buff,
                len,
                width,
                height,
                pixel_bytes,
                as::WRITE_BMP_FLIP | as::WRITE_BMP_ALIGN);

  delete [] buff;

  return 0;
}
