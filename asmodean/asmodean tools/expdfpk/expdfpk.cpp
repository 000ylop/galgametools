// expdfpk.cpp, v1.18 2013/05/04
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from FPK 0100 (*.fpk) archives used in Pretty Devil.

#include "as-util.h"

struct FPKHDR {
	uint8_t  signature[8]; // "FPK\x000100"
  uint32_t toc_offset;
  uint32_t entry_count;
  uint32_t unknown1;
  // char     directory_name[?]; // don't care
};

struct FPKENTRY {
  uint32_t obfuscated;
  uint32_t offset;
  uint32_t length;
  char     filename[12];
};

struct FBXHDR {
	uint8_t  signature[4]; // "FBX\x1"
	uint8_t  extension[3];
	uint8_t  hdr_length;
	uint32_t length;
	uint32_t original_length;
};

void unobfuscate(uint8_t* buff, 
                 uint32_t len,
                 uint32_t seed)
{
  auto p = (uint32_t*) buff;
  p += (len / 4) - 1;
  
  uint32_t k1 = seed + len - 4;
  uint32_t k2 = seed;

  while (p >= (uint32_t*) buff) {
    *p = (*p - k1) ^ k2;
    
    k2 = ((k2 - *p) >> 7) ^ ((k1 + k2) << 7);
    k1 -= 3;

    p--;
  }
}

uint32_t guess_unobfuscate(uint8_t* buff,
                           uint32_t len)
{
  static const uint32_t SEEDS[]    = { 0x00000000, 0x392B2A31, 0x962C0093, 0xD308A71F, 
                                       0xE09A774B, 0x430A939D, 0xCF09A4E1, 0xB071CC95, 
                                       0xE293F307, 0x26E183B5, 0x0527CC63, 0x7A0F44E3,
                                       0x902ACC72, 0xC34A7BB7, 0x12CA06E3, 0xDF16A057,
                                       0x250EB69A, 0xF244B07E, 0x7EF244B0, 0x3E14884A,
                                       0x7B2A67E1, 0x2F0598B4, 0x0F24A3A9, 0x192A3B4C };

  static const uint32_t SEED_COUNT = sizeof(SEEDS) / sizeof(SEEDS[0]);

  struct TRL {
    uint32_t length;
    uint32_t unknown;
  };

  static uint32_t seed = -1;

  if (seed == -1) {
    auto temp_buff = new uint8_t[len];
    auto trl       = (TRL*) (temp_buff + len - sizeof(TRL));

    for (uint32_t i = 0; i < SEED_COUNT && seed == -1; i++) {
      memcpy(temp_buff, buff, len);
      unobfuscate(temp_buff, len, SEEDS[i]);

      if (((trl->length + 3) & ~3) == len - 8) {
        seed = SEEDS[i];
        break;
      }
    }

    delete [] temp_buff;
  }

  if (seed != -1) {
    unobfuscate(buff, len, seed);

    auto trl = (TRL*) (buff + len - sizeof(TRL));

    len = trl->length;
  }

  return len;
}

void unfbx(uint8_t* buff,
           uint32_t len,
           uint8_t* out_buff,
           uint32_t out_len)
{
  auto end     = buff + len;
  auto out_end = out_buff + out_len;

  while (out_buff < out_end) {
    uint8_t  flags = *buff++;
    uint32_t p     = 0;
    uint32_t n     = 0;

    for (uint32_t i = 0; i < 4 && out_buff < out_end; i++) {
      switch (flags & 3) {
      case 0:
        *out_buff++ = *buff++;
        break;

      case 1:
        n = *buff++ + 2;

        while (n--) {
          *out_buff++ = *buff++;
        }
        break;

      case 2:
        p   = *buff++ << 8;
        p  |= *buff++;
        n   = (p & 0x1F) + 4;
        p >>= 5;

        while (n-- && out_buff < out_end) {
          *out_buff = *(out_buff - p - 1);
          out_buff++;
        }
        break;

      case 3:
        {
          uint8_t c = *buff++;

          n = c & 0x3F;

          switch (c >> 6) {
          case 0:
            n = (n << 8) + *buff++ + 0x102;

            while (n-- && out_buff < out_end) {
              *out_buff++ = *buff++;
            }
            break;

          case 1:
            p   = *buff++ << 8;
            p  |= *buff++;

            n   = (n << 5) | (p & 0x1F);
            n  += 0x24;

            p >>= 5;

            while (n-- && out_buff < out_end) {

              *out_buff = *(out_buff - p - 1);
              out_buff++;
            }
            break;

          case 2:
            break;

          // What the hell is this for
          case 3:
            buff += n;
            i = 4;
            break;
          }
        }
        break;
      }

      flags >>= 2;
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "expdfpk v1.18 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.fpk>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string in_filename_upper = as::stringtou(in_filename);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  FPKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  auto entries = new FPKENTRY[hdr.entry_count];
  lseek(fd, hdr.toc_offset, SEEK_SET);
  read(fd, entries, sizeof(FPKENTRY) * hdr.entry_count);

  for (uint32_t i = 0; i < hdr.entry_count; i++) {
    uint32_t len  = entries[i].length;
    auto     buff = new uint8_t[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (entries[i].obfuscated) {      
      len = guess_unobfuscate(buff, len);
    }

    auto fbxhdr = (FBXHDR*) buff;

    char filename[sizeof(entries[i].filename) + 1] = { 0 };
    memcpy(filename, entries[i].filename, sizeof(entries[i].filename));

    if (!memcmp(fbxhdr->signature, "FBX\x01", 4)) {
      uint32_t out_len  = fbxhdr->original_length;
      auto     out_buff = new uint8_t[out_len];
      unfbx(buff + sizeof(*fbxhdr), len - sizeof(*fbxhdr), out_buff, out_len);

      char ext[sizeof(fbxhdr->extension) + 1] = { 0 };
      memcpy(ext, fbxhdr->extension, sizeof(fbxhdr->extension));

      as::write_file(as::stringf("%s.%s", as::get_file_prefix(filename).c_str(), ext),
                     out_buff,
                     out_len);

      delete [] out_buff;
    } else {
      as::write_file(filename, buff, len);
    }

    delete [] buff;
  }

  close(fd);

  return 0;
}
