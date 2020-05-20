// exvcpak.cpp, v1.0 2009/06/02
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from Valkyrie Complex's *.PAK archives.

#include "as-util.h"

struct PAKHDR {
  unsigned char game_name[24]; // "ÇuÇbêªïiî≈" (plaintext)
  unsigned long entry_count;
  unsigned long toc_len;
};

struct PAKENTRY {
  unsigned long unknown;
  unsigned long filename_length;
  unsigned long offset;
  unsigned long length;
};

struct DATAHDR {
  unsigned long original_length;
};

class bitbuff_t {
public:
  bitbuff_t(unsigned char* buff, 
            unsigned long  len)
    : buff(buff),
      len(len),
      saved_count(0),
      saved_bits(0)
  {}

  unsigned long get_bits(unsigned long bits) {
    unsigned long n = 0;

    while (bits--) {
      if (saved_count == 0) {
        saved_bits  = *buff++;
        saved_count = 8;
      }

      n = (n << 1) | (saved_bits & 1);

      saved_bits >>= 1;
      saved_count--;
    }

    return n;
  }

  unsigned char* buff;
  unsigned long  len;
  unsigned long  saved_count;
  unsigned long  saved_bits;
};

void uncps2(unsigned char* buff, 
            unsigned long  len,
            unsigned char* out_buff,
            unsigned long  out_len)
{
  struct HDR {
    unsigned short dict[16];
    unsigned char  last_byte;
    unsigned char  pad; // ?
  };

  HDR* hdr = (HDR*) buff;

  bitbuff_t bits(buff + sizeof(*hdr), len - sizeof(*hdr)); 

  unsigned short* p   = (unsigned short*) out_buff;
  unsigned short* end = p + (out_len / 2);

  out_buff[out_len - 1] = hdr->last_byte;

  while (p < end) {
    if (bits.get_bits(1)) {
      *p++ = (unsigned short) bits.get_bits(16);
    } else {
      unsigned long index = 0;

      while (index < 15 && !bits.get_bits(1)) {
        index++;
      }

      *p++ = hdr->dict[index];
    }
  }
}

void uncps3(unsigned char* buff, 
            unsigned long  len,
            unsigned char* out_buff,
            unsigned long  out_len)
{
  static const unsigned long PREFIX_LEN = 128;

  if (out_len < PREFIX_LEN) {
    memcpy(out_buff, buff, out_len);
    return;
  }

  unsigned char* out_end = out_buff + out_len;

  memcpy(out_buff, buff, PREFIX_LEN);
  buff     += PREFIX_LEN;
  out_buff += PREFIX_LEN;

  bitbuff_t bits(buff, len);

  while (out_buff < out_end) {
    if (bits.get_bits(1)) {
      unsigned long p = bits.get_bits(7) + 1;
      unsigned long n = bits.get_bits(4) + 2;

      unsigned char* src = out_buff - p;

      while (n--) {
        *out_buff++ = *src++;
      }
    } else {
      *out_buff++ = (unsigned char) bits.get_bits(8);
    }
  }
}

void unscramble(unsigned char* buff, unsigned long len) {
  if (len < 776) return; 

  buff[0] ^= buff[len - 1];

  // I don't think they intended this to be constant :P
  unsigned long z = 0xFF;
  unsigned long x = (len - 4) / 512;
  
  std::swap(buff[len - z], buff[4 + x + z]);
  std::swap(buff[len - 2 * z], buff[4 + 2 * x + z]);
}

void unobfuscate(unsigned char* buff, 
                 unsigned long  len, 
                 unsigned long  entry_length) 
{
  unsigned char* end = buff + len;
  unsigned char  key = buff[entry_length - 1];

  while (buff < end) {
    *buff++ ^= key;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exvcpak v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak>\n\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PAKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  if (!memcmp(hdr.game_name, "\x00\x00\x01\xBA", 4)) {
    fprintf(stderr, "%s: this is video data.\n", in_filename.c_str());
    return 0;
  }

  unobfuscate((unsigned char*)&hdr, sizeof(hdr), sizeof(hdr));  

  unsigned long  toc_len  = hdr.toc_len;
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);
  unobfuscate(toc_buff, toc_len, sizeof(PAKENTRY));

  PAKENTRY* entries  = (PAKENTRY*) toc_buff;
  char*     filename = (char*) (entries + hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {    
    string out_filename = filename;
    filename += entries[i].filename_length + 1;

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);
    for (unsigned long j = 0; j < len; j++) buff[j] ^= 0x24;

    if (out_filename.find(".CPS") != string::npos) {      
      bool           skip      = false;
      DATAHDR*       datahdr   = (DATAHDR*) buff;
      unsigned long  data_len  = len  - sizeof(*datahdr);
      unsigned char* data_buff = buff + sizeof(*datahdr);

      unsigned long type = (datahdr->original_length >> 28) & 3;

      unsigned long  out_len  = (datahdr->original_length ^ 0xFA415FCF) & 0xFFFFFFF;
      unsigned char* out_buff = new unsigned char[out_len];

      unscramble(data_buff, data_len);

      switch (type) {
        case 0:
          memcpy(out_buff, data_buff, out_len);
          break;

        case 2:
          uncps2(data_buff, data_len, out_buff, out_len);
          break;

        case 3:
          uncps3(data_buff, data_len, out_buff, out_len);
          break;

        default:
          skip = true;
          printf("%s: unsupported type (%d)\n", out_filename.c_str(), type);
          break;
      }

      if (!skip) {
        delete [] buff;

        len          = out_len;
        buff         = out_buff;
        out_filename = as::get_file_prefix(out_filename) + as::guess_file_extension(buff, len);
      }
    }

    as::make_path(out_filename);
    as::write_file(out_filename, buff, len);

    delete [] buff;    
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}
