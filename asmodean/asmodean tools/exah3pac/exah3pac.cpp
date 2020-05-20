// exah3pac.cpp, v1.01 2012/07/26
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts images from FPAC (*.pac) archives used 
// by Arcana Heart 3 (XBOX360) and Persona 4 Arena (XBOX360).

// Decompress data with xbdecompress from the XDK first...

#include "as-util.h"

struct FPACHDR {
  unsigned char signature[4]; // "FPAC"
  unsigned long data_base;
  unsigned long file_length;
  unsigned long entry_count;
  unsigned long unknown1;
  unsigned long filename_length;
  unsigned long unknown3;
  unsigned long unknown4;

  void flip_endian(void) {
    as::flip_endian_multi(&data_base,
                          &file_length,
                          &entry_count,
                          &filename_length,
                          NULL);
  }
};

struct FPACENTRY {
  // char filename[hdr.filename_length];
  unsigned long index;
  unsigned long offset;
  unsigned long length;

  void flip_endian(void) {
    as::flip_endian_multi(&index,
                          &offset,
                          &length,
                          NULL);
  }
};

struct HIPHDR {
  unsigned char signature[4]; // "HIP"
  unsigned long unknown1;
  unsigned long file_length;
  unsigned long unknown2;
  unsigned long width;
  unsigned long height;
  unsigned long flags;
  unsigned long unknown4;

  void flip_endian(void) {
    as::flip_endian_multi(&file_length,
                          &width,
                          &height,
                          &flags,
                          NULL);
  }
};

// These are probably bitfields but I'm lazy to handle them individually
static const unsigned long HIPHDR_TYPE1                          = 0x0000;
static const unsigned long HIPHDR_TYPE2                          = 0x2001;
static const unsigned long HIPHDR_COMPRESSION_TYPE_8BIT          = 0x0001;
static const unsigned long HIPHDR_COMPRESSION_TYPE_8BIT_RLE      = 0x0101;
static const unsigned long HIPHDR_COMPRESSION_TYPE_GREYSCALE_RLE = 0x0104;
static const unsigned long HIPHDR_COMPRESSION_TYPE_32BIT_RLE     = 0x0110;
static const unsigned long HIPHDR_COMPRESSION_TYPE_32BIT_LONGRLE = 0x1010;
static const unsigned long HIPHDR_COMPRESSION_TYPE_LZ            = 0x0210;

struct HIPHDR2 {
  unsigned long width;
  unsigned long height;
  unsigned long offset_x;
  unsigned long offset_y;
  unsigned long unknown5;
  unsigned long unknown6;
  unsigned long unknown7;
  unsigned long unknown8;

  void flip_endian(void) {
    as::flip_endian_multi(&width,
                          &height,
                          &offset_x,
                          &offset_y,
                          &unknown5,
                          &unknown6,
                          &unknown7,
                          &unknown8,
                          NULL);
  }

};

void process_file(const string& prefix, unsigned char* buff, unsigned long len);

bool process_fpac(const string& prefix, unsigned char* buff, unsigned long len) {
  if (len < 4 || memcmp(buff, "FPAC", 4)) {
    return false;
  }

  FPACHDR* hdr = (FPACHDR*) buff;
  hdr->flip_endian();

  unsigned char* p         = (unsigned char*) (hdr + 1);
  unsigned char* data_buff = buff + hdr->data_base;

  for (unsigned long i = 0; i < hdr->entry_count; i++) {
    char filename[1024] = { 0 };
    memcpy(filename, p, hdr->filename_length);
    p += hdr->filename_length;

    FPACENTRY* entry = (FPACENTRY*) p;
    p += sizeof(*entry);
    entry->flip_endian();

    unsigned long align = (hdr->filename_length + sizeof(*entry)) % 16;
    if (align) {
      p += 16 - align;
    }

    process_file(prefix + "+" + filename, data_buff + entry->offset, entry->length);
  }

  return true;
}

bool process_hip(const string& prefix, unsigned char* buff, unsigned long len) {
  if (len < 3 || memcmp(buff, "HIP", 3)) {
    return false;
  }

  HIPHDR* hdr = (HIPHDR*) buff;

  // Some are randomly little endian...
  bool big_endian = hdr->unknown1 != 0x0125;

  if (big_endian) hdr->flip_endian();

  unsigned char* data = (unsigned char*) (hdr + 1);

  string         out_filename = prefix;
  unsigned long  width        = hdr->width;
  unsigned long  height       = hdr->height;
  unsigned long  depth        = 0;
  unsigned long  out_len      = 0;
  unsigned char* out_buff     = NULL;
  unsigned char* pal          = NULL;

  unsigned long type             = hdr->flags >> 16;
  unsigned long compression_type = hdr->flags & 0xFFFF;

  switch (type) {
    case HIPHDR_TYPE1:
      break;

    case HIPHDR_TYPE2:
      {
        HIPHDR2* hdr2 = (HIPHDR2*) data;
        hdr2->flip_endian();
        data += sizeof(*hdr2);    

        width  = hdr2->width;
        height = hdr2->height;
        out_filename += as::stringf("+x%dy%d", hdr2->offset_x, hdr2->offset_y);

        if (big_endian) hdr2->flip_endian();
      };
      break;

    default:
      if (big_endian) hdr->flip_endian();
      return false;
  }

  switch (compression_type) {
    case HIPHDR_COMPRESSION_TYPE_LZ:
      {
        struct lzhdr_t {
          unsigned char control;
          unsigned char depth;
          unsigned char bg[4];
        };

        lzhdr_t* lzhdr = (lzhdr_t*) data;
        data += sizeof(*lzhdr);

        depth    = lzhdr->depth;
        out_len  = width * height * depth;
        out_buff = new unsigned char[out_len];

        unsigned char* out_p    = out_buff;
        unsigned char* out_end  = out_p + out_len;

        while (out_p < out_end) {
          unsigned char c = *data++;

          if (c == lzhdr->control && *data != lzhdr->control) {
            unsigned long p = *data++;
            unsigned long n = *data++;

            if (p == 0xFF) {
              p = lzhdr->control;
            }

            p++;
            p *= 4;

            while (n--) {
              for (unsigned long i = 0; i < lzhdr->depth; i++) {
                if (out_p - p < out_buff) {
                  *out_p = lzhdr->bg[i];
                } else {
                  *out_p = *(out_p - p);
                }

                out_p++;
              }
            }
          } else {
            if (c == lzhdr->control) {
              c = *data++;
            }

            *out_p++ = c;

            for (unsigned long i = 1; i < lzhdr->depth; i++) {
              *out_p++ = *data++;
            }
          }
        }
      }
      break;

    case HIPHDR_COMPRESSION_TYPE_8BIT:
      pal = data;
      data += 1024;

      depth    = 1;
      out_len  = width * height * depth;
      out_buff = new unsigned char[out_len];

      memcpy(out_buff, data, out_len);
      break;

    case HIPHDR_COMPRESSION_TYPE_GREYSCALE_RLE:
      {
        depth    = 4;
        out_len  = width * height * depth;
        out_buff = new unsigned char[out_len];

        unsigned char* out_p    = out_buff;
        unsigned char* out_end  = out_p + out_len;

        while (out_p < out_end) {
          unsigned char n = data[2];

          while (n--) {
            *out_p++ = data[1];
            *out_p++ = data[0];
            *out_p++ = data[0];
            *out_p++ = data[0];
          }

          data += 3;
        }
      }
      break;

    case HIPHDR_COMPRESSION_TYPE_32BIT_LONGRLE:
      {
        depth    = 4;
        out_len  = width * height * depth;
        out_buff = new unsigned char[out_len];

        unsigned char* out_p    = out_buff;
        unsigned char* out_end  = out_p + out_len;

        while (out_p < out_end) {
          unsigned long n = as::flip_endian(*(unsigned long*) data);
          data += 4;

          if (n & 0x80000000) {
            n &= 0x7FFFFFFF;

            while (n--) {
              for (unsigned long i = 0; i < depth; i++) {
                *out_p++ = *data++;
              }
            }
          } else {
            while (n--) {
              for (unsigned long i = 0; i < depth; i++) {
                *out_p++ = data[i];
              }
            }

            data += depth;
          }
        }
      }
      break;

    case HIPHDR_COMPRESSION_TYPE_8BIT_RLE:
    case HIPHDR_COMPRESSION_TYPE_32BIT_RLE:
      {
        if (compression_type == HIPHDR_COMPRESSION_TYPE_8BIT_RLE) {
          pal = data;
          data += 1024;

          depth = 1;
        } else {
          depth = 4;
        }

        out_len  = width * height * depth;
        out_buff = new unsigned char[out_len];

        unsigned char* out_p    = out_buff;
        unsigned char* out_end  = out_p + out_len;

        while (out_p < out_end) {
          unsigned char n = data[depth];

          while (n--) {
            for (unsigned long i = 0; i < depth; i++) {
              *out_p++ = data[i];
            }
          }

          data += depth + 1;
        }
      }
      break;

    default:
      if (big_endian) hdr->flip_endian();
      return false;
  }

  unsigned long options = as::WRITE_BMP_FLIP | as::WRITE_BMP_ALIGN;

  if (big_endian) {
    options |= as::WRITE_BMP_BIGENDIAN;
  }

  as::write_bmp_ex(out_filename + ".bmp",
                   out_buff,
                   out_len,
                   width,
                   height,
                   depth,
                   256,
                   pal,
                   options);

  delete [] out_buff;

  return true;
}

void process_file(const string& filename, unsigned char* buff, unsigned long len) {
  if (!process_fpac(as::get_file_prefix(filename), buff, len) &&
      !process_hip(as::get_file_prefix(filename), buff, len))
  {
    string out_filename = filename;

    if (len >= 4 && !memcmp(buff, "XPR2", 4)) {
      out_filename += ".xpr";
    }

    as::write_file(out_filename, buff, len);
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exah3pac v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pac>\n", argv[0]);
    return -1;
  }

  string in_filename  = argv[1];

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY); 

  unsigned long  len  = as::get_file_size(fd);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  if (memcmp(buff, "FPAC", 4)) {
    fprintf(stderr, "%s: not an FPAC archive? (Use xbdecompress from the xdk first.)\n", in_filename.c_str());
    return 0;
  }

  process_file(in_filename, buff, len);

  delete [] buff;

  close(fd);

  return 0;
}