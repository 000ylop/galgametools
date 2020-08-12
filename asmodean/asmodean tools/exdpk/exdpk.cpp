// exdpk.cpp, v1.05 2012/09/03
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts DPK (*.dpk) archives.

#include "as-util.h"

struct game_info_t {
  uint32_t magic1;
  uint32_t magic2;
  string   name;
};

std::array<game_info_t, 6> GAME_INFO = {{ 
  { 0x0FF98, 0x43E78A5C, "install" },
  { 0x0946E, 0xB1956783, "îíã‚ÇÃÉJÉãÇ∆ëìãÛÇÃèóâ§" },
  { 0x11EAF, 0xB9976112, "ó∏îõÅ`îwìøÇÃÉAÉgÉäÉGÅ`" },
  { 0x0583F, 0xB81031D7, "ñ≤Ç›ÇÈåéÇÃÉãÉiÉãÉeÉBÉA" },
  { 0x0527F, 0x339B266F, "ó∏èQÅ`èóäwê∂í≤ã≥Å`" },
  { 0x0BB51, 0x891F52A3, "îíê_éq Å`ÇµÇÎÇ›Ç±Å`" },
}};

static const auto GAME_CHOICES = as::choices_by_index_t<game_info_t>::init(GAME_INFO);

struct DPKHDR {
  uint8_t  signature[4]; // "DPK"
  uint32_t unknown1;
  uint32_t toc_length;   // including header
  uint32_t unknown3;
  uint32_t entry_count;
};

struct DPKENTRY1 {
  uint32_t entry_offset;
};

struct DPKENTRY2 {
  uint32_t offset;
  uint32_t length;
  uint32_t unknown1;
  char     filename[1];
};

void unobfuscate_toc(uint8_t* buff, uint32_t len) {
  uint8_t  last = 0;
  uint32_t i    = 8;

  for (; i < 16 && i < len; i++) {
    last = buff[i];
    buff[i] ^= i - 16;
  }

  for (; i < len; i++) {
    int32_t t = i + last;
    last = buff[i];
    buff[i] ^= t;
  }
}

void unobfuscate_data(const string&      filename, 
                      const game_info_t& info,
                      uint8_t*           buff, 
                      uint32_t           len)
{
  string name_only = as::get_filename(as::stringtol(filename));

  uint32_t seed = 0;

  // Assume trailing z already trimmed
  for (int32_t i = name_only.length() - 1; i >= 0; i--) {
    seed += info.magic1 + info.magic2 * (len + name_only[i]);
  }

  uint32_t key = info.magic1;

  for (uint32_t i = 0; i < len; i++) {
    buff[i] ^= key + (uint8_t)(key >> 8);
    buff[i] -= (uint8_t) seed;

    key += info.magic2;
  }
}

struct DGCHDR {
  uint8_t  signature[4]; // "DGC"
  uint32_t  max_dict_length_flags;
  uint16_t width;
  uint16_t height;
};

static const uint32_t DGC_FLAG_ALPHA = 0x00000004;
static const uint32_t DGC_FLAG_DICT  = 0x00000002;

void uncompress_line16(uint8_t* buff,
                       uint32_t len,
                       as::RGB* out,
                       as::RGB* dict)
{
  auto end = buff + len;

  while (buff < end) {
    uint16_t c = *(uint16_t*) buff;
    buff += 2;

    if (c & 0x8000) {
      uint32_t n = (c & 0x3F) + 2; 
      int16_t  p = (int16_t) c;
      p >>= 6;

      while (n--) {
        *out = *(out + p);
        out++;
      }
    } else {
      uint32_t n = c & 0x1FFF;
      
      if (c & 0x4000) {
        uint32_t index = 0;

        if (c & 0x2000) {
          index = *buff++;
        } else {
          index = *(uint16_t*) buff;
          buff += 2;
        }

        while (n--) {
          *out++ = dict[index];
        }
      } else {
        while (n--) {
          uint32_t index = 0;

          if (c & 0x2000) {
            index = *buff++;
          } else {
            index = *(uint16_t*) buff;
            buff += 2;
          }

          *out++ = dict[index];
        }
      }
    }
  }
}

void uncompress_line8(uint8_t* buff,
                      uint32_t len,
                      as::RGB* out,
                      as::RGB* dict)
{
  auto end = buff + len;

  while (buff < end) {
    uint8_t c = *buff++;

    if (c) {
      uint32_t index = *buff++;

      while (c--) {
        *out++ = dict[index];
      }
    } else {
      c = *buff++;

      if ((int8_t)c >= 0) {
        uint32_t n = c + 2;

        while (n--) {
          *out++ = dict[*buff++];
        }
      } else {
        int16_t  p = (int16_t) ((c << 8) | *buff++);
        uint32_t n = (p & 0x3F) + 4;

        p >>= 6;

        while (n--) {
          *out = *(out + p);
          out++;
        }
      }
    }
  }
}

void uncompress_line_alpha(uint8_t* buff,
                           uint32_t len,
                           uint8_t* out)
{
  auto end = buff + len;

  while (buff < end) {
    uint8_t c = *buff++;

    if (c) {
      while (c--) {
        *out++ = *buff;
      }

      buff++;
    } else {
      c = *buff++;

      if ((int8_t)c >= 0) {
        uint32_t n = c + 2;

        while (n--) {
          *out++ = *buff++;
        }
      } else {
        int16_t  p = (int16_t) ((c << 8) | *buff++);
        uint32_t n = (p & 0x3F) + 4;

        p >>= 6;

        while (n--) {
          *out = *(out + p);
          out++;
        }
      }
    }
  }
}

void uncompress_line_lz(uint8_t* buff,
                        uint32_t len,
                        as::RGB* out)
{
  auto end = buff + len;

  while (buff < end) {
    uint16_t c = *(uint16_t*) buff;
    buff += 2;

    if (c & 0x8000) {
      uint32_t n = (c & 0x3F) + 1; 
      int16_t  p = (int16_t) c;
      p >>= 6;

      while (n--) {
        *out = *(out + p);
        out++;
      }
    } else {
      uint32_t n = c & 0x1FFF;
      
      if (c & 0x4000) {
        while (n--) {
          *out++ = *(as::RGB*) buff;
        }
        buff += 3;
      } else {
        while (n--) {
          *out++ = *(as::RGB*) buff;
          buff += 3;
        }
      }
    }
  }
}

bool proc_dgc(const string& filename,
              uint8_t*      buff,
              uint32_t      len)
{
  if (len < 3 || memcmp(buff, "DGC", 3)) {
    return false;
  }

  auto hdr = (DGCHDR*) buff;
  buff += sizeof(*hdr);

  uint32_t depth    = 3;
  uint32_t out_len  = hdr->width * hdr->height * depth;
  auto     out_buff = new uint8_t[out_len];

  uint8_t  flags        = hdr->max_dict_length_flags >> 24;
  uint32_t max_dict_len = hdr->max_dict_length_flags & 0xFFFFFF;

  if (flags & DGC_FLAG_DICT) {
    auto dict_buff = new uint8_t[max_dict_len * 3];
    auto dict      = (as::RGB*) dict_buff;
  
    if (max_dict_len > 256) {
      for (uint32_t y = 0; y < hdr->height;) {
        uint32_t dict_len = *(uint16_t*) buff + 1;
        buff += 2;

        memcpy(dict_buff, buff, dict_len * 3);
        buff += dict_len * 3;

        uint16_t y_end = *(uint16_t*) buff;
        buff += 2;

        for (; y < y_end; y++) {
          auto out_line = (as::RGB*) (out_buff + y * hdr->width * 3);

          int16_t line_len = *(int16_t*) buff;
          buff += 2;

          if (line_len) {
            if (line_len > -1) {
              if (dict_len > 256) {
                uncompress_line16(buff, line_len, out_line, dict);              
              } else {
                uncompress_line8(buff, line_len, out_line, dict);
              }
              buff += line_len;
            } else {
              auto src_line = (as::RGB*) (out_buff + (y + line_len) * hdr->width * 3);

              for (uint32_t x = 0; x < hdr->width; x++) {
                out_line[x] = src_line[x];
              }
            }
          } else {
            for (uint32_t x = 0; x < hdr->width; x++) {
              if (dict_len > 256) {
                uint16_t index = *(uint16_t*)buff;
                buff += 2;

                out_line[x] = dict[index];
              } else {
                out_line[x] = dict[*buff++];
              }
            }
          }
        }
      }
    } else {
      uint32_t dict_len = *buff++ + 1;
      memcpy(dict_buff, buff, dict_len * 3);
      buff += dict_len * 3;

      for (uint32_t y = 0; y < hdr->height; y++) {
        auto out_line = (as::RGB*) (out_buff + y * hdr->width * 3);

        int16_t line_len = *(int16_t*) buff;
        buff += 2;

        if (line_len) {
          if (line_len > -1) {
            uncompress_line8(buff, line_len, out_line, dict);
            buff += line_len;
          } else {
            auto src_line = (as::RGB*) (out_buff + (y + line_len) * hdr->width * 3);

            for (uint32_t x = 0; x < hdr->width; x++) {
              out_line[x] = src_line[x];
            }
          }
        } else {
          for (uint32_t x = 0; x < hdr->width; x++) {
            out_line[x] = dict[buff[x]];
          }

          buff += hdr->width;
        }
      }
    }

    delete [] dict_buff;
  } else {
    for (uint32_t y = 0; y < hdr->height; y++) {
      auto out_line = (as::RGB*) (out_buff + y * hdr->width * 3);

      int16_t line_len = *(int16_t*) buff;
      buff += 2;

      if (line_len) {
        if (line_len > -1) {
          uncompress_line_lz(buff, line_len, out_line);
          buff += line_len;
        } else {
          auto src_line = (as::RGB*) (out_buff + (y + line_len) * hdr->width * 3);

          for (uint32_t x = 0; x < hdr->width; x++) {
            out_line[x] = src_line[x];
          }
        }
      } else {
        auto src_line = (as::RGB*) buff;
        buff += hdr->width * 3;

        for (uint32_t x = 0; x < hdr->width; x++) {
          out_line[x] = src_line[x];
        }
      }
    }
  }

  if (flags & DGC_FLAG_ALPHA) {
    uint32_t alpha_len  = hdr->width * hdr->height;
    auto     alpha_buff = new uint8_t[alpha_len];

    for (uint32_t y = 0; y < hdr->height; y++) {
      auto out_line = alpha_buff + y * hdr->width;

      int16_t line_len = *(int16_t*) buff;
      buff += 2;

      if (line_len) {
        if (line_len > -1) {
          uncompress_line_alpha(buff, line_len, out_line);
          buff += line_len;
        } else {
          auto src_line = alpha_buff + (y + line_len) * hdr->width;

          for (uint32_t x = 0; x < hdr->width; x++) {
            out_line[x] = src_line[x];
          }
        }
      } else {
        for (uint32_t x = 0; x < hdr->width; x++) {
          out_line[x] = buff[x];
        }

        buff += hdr->width;
      }
    }

    depth = 4;
    uint32_t temp_len  = hdr->width * hdr->height * depth;
    auto     temp_buff = new uint8_t[temp_len];    

    for (uint32_t y = 0; y < hdr->height; y++) {
      auto rgb_line   = (as::RGB*) (out_buff + y * hdr->width * 3);
      auto alpha_line = alpha_buff + y * hdr->width;
      auto rgba_line  = (as::RGBA*) (temp_buff + y * hdr->width * 4);

      for (uint32_t x = 0; x < hdr->width; x++) {
        rgba_line[x] = as::RGBA(rgb_line[x], alpha_line[x]);
      }
    }

    delete [] alpha_buff;
    delete [] out_buff;

    out_len  = temp_len;
    out_buff = temp_buff;
  }

  as::write_bmp(as::get_file_prefix(filename) + ".bmp",
                out_buff,
                out_len,
                hdr->width,
                hdr->height,
                depth,
                as::WRITE_BMP_FLIP);

  delete [] out_buff;

  return true;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "exdpk v1.05 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dpk> <choice>\n", argv[0]);
    GAME_CHOICES.print();

    return -1;
  }

  string      in_filename = argv[1];
  const auto& info        = GAME_CHOICES.get(argv[2]);
  
  auto fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);  

  uint32_t toc_len  = 0;
  uint8_t* toc_buff = NULL;

  {
    DPKHDR hdr;
    read(fd, &hdr, sizeof(hdr));  
    unobfuscate_toc((uint8_t*) &hdr, sizeof(hdr));

    toc_len  = hdr.toc_length;
    toc_buff = new uint8_t[toc_len];
  }

  lseek(fd, 0, SEEK_SET);
  read(fd, toc_buff, toc_len);
  unobfuscate_toc(toc_buff, toc_len);

  auto hdr      = (DPKHDR*) toc_buff;
  auto entries  = (DPKENTRY1*) (hdr + 1);
  auto entries2 = (uint8_t*) (entries + hdr->entry_count);

  for (uint32_t i = 0; i < hdr->entry_count; i++) {
    auto   entry    = (DPKENTRY2*) (entries2 + entries[i].entry_offset);
    string filename = entry->filename;

    if (*filename.rbegin() == 'z') {
      filename = filename.substr(0, filename.length() - 1);
    }   

    uint32_t len  = entry->length;
    auto     buff = new uint8_t[len];
    lseek(fd, entry->offset + hdr->toc_length, SEEK_SET);
    read(fd, buff, len);
    unobfuscate_data(filename, info, buff, len);

    as::make_path(filename);

    if (!proc_dgc(filename, buff, len)) {
      as::write_file(filename, buff, len);
    }

    delete [] buff;
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}
