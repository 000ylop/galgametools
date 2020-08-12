// exmed.cpp, v1.02 2012/09/14
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extract MD (*.med) archives.

#include "as-util.h"

struct MEDHDR {
  uint8_t  signature[2]; // "MD"
  uint16_t unknown1;
  uint16_t entry_length;
  uint16_t entry_count;
  uint32_t unknown2;     // garbage
  uint32_t unknown3;     // garbage
};

struct MEDENTRY {
  // char     filename[];
  uint32_t length;
  uint32_t offset;
};

struct YBHDR {
  uint8_t  signature[2]; // "YB"
  uint8_t  flags;
  uint8_t  depth_bytes;
  uint32_t length;
  uint32_t original_length;;
  uint16_t width;
  uint16_t height;
};

bool proc_yb(const string&  filename,
             uint8_t*       buff,
             uint32_t       len)
{
  auto hdr = (YBHDR*) buff;
  buff += sizeof(*hdr);

  if (len < sizeof(*hdr) || memcmp(hdr->signature, "YB", 2)) {
    return false;
  }

  // Bug in the data??
#if 0
  if (filename == "SYS_CLICKW" && hdr->height == 58) {
    hdr->height = 116;
  }
#endif

  uint32_t out_len = 0;

  if (hdr->flags & 0x40) {
    out_len = hdr->original_length;

    // XXX: haven't seen anything that needs this
    return false;
  } else {
    out_len = hdr->width * hdr->height * hdr->depth_bytes;
  }

  auto out_buff = new uint8_t[out_len];
  auto out_p    = out_buff;
  auto out_end  = out_buff + out_len;

  uint32_t p = 0;
  uint32_t n = 0;

  while (out_p < out_end) {
    uint8_t flags = *buff++;

    for (uint32_t i = 0; out_p < out_end && i < 8; i++) {
      if (flags & 0x80) {
        uint32_t cmd1 = *buff++;

        if (cmd1 & 0x80) {
          uint32_t cmd2 = *buff++;

          if (cmd1 & 0x40) {
            p = cmd2 + ((cmd1 & 0x3F) << 8) + 1;

            uint32_t cmd3 = *buff++;

            switch (cmd3) {
              case 0xFF:
                n = 4096;
                break;
              case 0xFE:
                n = 1024;
                break;
              case 0xFD:
                n = 256;
                break;
              default:
                n = cmd3 + 3;
                break;
            }
          } else {
            p = (cmd2 >> 4) + ((cmd1 & 0x3F) << 4) + 1;
            n = (cmd2 & 0xF) + 3;
          }

          while (out_p < out_end && n--) {
            *out_p = *(out_p - p);
            out_p++;
          }
        } else {
          if ((cmd1 & 3) == 3) {
            n = (cmd1 >> 2) + 9;

            while (out_p < out_end && n--) {
              *out_p++ = *buff++;
            }
          } else {
            p = (cmd1 >> 2) + 1;
            n = (cmd1 & 3) + 2;

            while (out_p < out_end && n--) {
              *out_p = *(out_p - p);
              out_p++;
            }
          }
        }
      } else {
        *out_p++ = *buff++;
      }

      flags <<= 1;
    }
  }

  if (hdr->flags & 0x40) {
    // XXX: haven't seen anything that needs this
  }

  if (hdr->flags & 0x80) {
    for (uint32_t i = 0; i < hdr->depth_bytes; i++) {
      for (uint32_t j = hdr->depth_bytes + i; j < out_len; j += hdr->depth_bytes) {
        out_buff[j] += *(out_buff - hdr->depth_bytes + j);
      }
    }
  }

  as::write_bmp(filename + ".bmp",
                out_buff,
                out_len,
                hdr->width,
                hdr->height,
                hdr->depth_bytes,
                as::WRITE_BMP_FLIP);

  delete [] out_buff;

  return true;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmed v1.02 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.med>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  MEDHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  if (memcmp(hdr.signature, "MD", 2)) {
    fprintf(stderr, "%s: unsupported archive\n", in_filename.c_str());
    return 1;
  }

  uint32_t toc_len  = hdr.entry_count * hdr.entry_length;
  auto     toc_buff = new uint8_t[toc_len];
  read(fd, toc_buff, toc_len);

  auto p = toc_buff;
  
  for (uint32_t i = 0; i < hdr.entry_count; i++) {
    auto filename = (char*) p;
    auto entry    = (MEDENTRY*) (p + hdr.entry_length - sizeof(MEDENTRY));
    p += hdr.entry_length;

    uint32_t len  = entry->length;
    auto     buff = new uint8_t[len];
    lseek(fd, entry->offset, SEEK_SET);
    read(fd, buff, len);

    if (!proc_yb(filename, buff, len)) {
      as::write_file(filename + as::guess_file_extension(buff, len), buff, len);
    }

    delete [] buff;
  }

  delete [] toc_buff;

  close(fd);
  
  return 0;
}
