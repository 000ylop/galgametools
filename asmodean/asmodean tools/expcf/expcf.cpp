// expcf.cpp, v1.04 2012/05/31
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from encrypted PackCode (*.pcf) archives.

#include "as-util.h"

//#define VERSION 1
#define VERSION 2

// Crypto++
#include "modes.h"
#include "rc6.h"
#include "aes.h"

// Karl Malbrain's implementation with tweaked shifts
#include "sha256_tweak.h"

// Hobocrypt
#include "hobocrypt.h"

struct PCFHDR {
  unsigned char signature[8]; // "PackCode"
  unsigned long entry_count;
  unsigned long unknown1;
  unsigned long data_length;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long unknown5;
  unsigned long unknown6;
  unsigned long toc_offset;
  unsigned long unknown7;
  unsigned long toc_length;
  unsigned long unknown8;
  unsigned long flags;
  unsigned long note_length;
  unsigned long seeds[16];
#if VERSION >= 2
  char          note[64];
#else
  char          note[32];
#endif
};

struct PCFENTRY {
  char          filename[80];
  unsigned long offset; // from start of data
  unsigned      unknown1;
  unsigned long original_length;
  unsigned long unknown2;
  unsigned long length;
  unsigned long unknown3;
  unsigned long flags;
  unsigned long unknown4;
  unsigned long seeds[4];
};

struct GBCFHDR {
  unsigned char  signature[4];
  unsigned long  length; // not including trailing note
  unsigned long  width;
  unsigned long  height;
  unsigned short depth;
  unsigned short flags;
  unsigned long  unknown2;
  unsigned long  unknown3;
  unsigned long  note_length;
  unsigned long  unknown5;
  unsigned long  unknown6;
  unsigned long  unknown7;
  unsigned long  unknown8;
};

unsigned long unrangecode(unsigned char* buff,
                          unsigned long  len,
                          unsigned char* out_buff,
                          unsigned long  out_len)
{
  unsigned long  total_len = 0;
  unsigned char* end       = buff + len;

  while (true) {
    unsigned long chunk_len = *(unsigned long*) buff;
    buff += 4;

    total_len += chunk_len;

    unsigned char  type       = *buff++;
    unsigned short table[256] = { 0 };    

    switch (type & 0x1F) {
      case 1:
        {
          unsigned long n = *buff++;

          while (n--) {
            unsigned char i = *buff++;
            unsigned char c = *buff++;

            if (c & 0x80) {
              table[i] = c & 0x7F;
            } else {
              table[i] = (*buff++ << 7) | c;
            }
          }
        }
        break;

      case 2:
        for (unsigned long i = 0; i < 256; i++) {
          unsigned char c = *buff++;

          if (c & 0x80) {
            table[i] = c & 0x7F;
          } else {
            table[i] = (*buff++ << 7) | c;
          }
        }
        break;
    }

    unsigned char* table2      = new unsigned char[0xFFFF00];
    unsigned long  table3[256] = { 0 };
    unsigned long  table4[256] = { 0 };
    unsigned long  offset      = 0;

    for (unsigned long i = 0; i < 256; i++) {
      table3[i] = offset;
      table4[i] = table[i];

      memset(table2 + offset, (unsigned char) i, table[i]);
      offset += table[i];
    }

    unsigned long divisor  = 0xC0000000;
    unsigned long dividend = 0;

    for (unsigned long i = 0; i < 4; i++) {
      dividend = (dividend << 8) | *buff++;
    }

    for (unsigned long i = 0; i < chunk_len; i++) {
      unsigned long index = dividend / (divisor >> 12);
      unsigned char c     = table2[index];

      *out_buff++ = c;

      dividend -= (divisor >> 12) * table3[c];
      divisor   = (divisor >> 12) * table4[c];

      while (!(divisor & 0xFF000000)) {
        dividend = (dividend << 8) | *buff++;
        divisor <<= 8;
      }
    }

    delete [] table2;

    if (!(type & 0x80)) {
      break;
    }
  }

  return total_len;
}

void unrle(unsigned char* buff,
           unsigned long  len,
           unsigned char* out_buff,
           unsigned long  out_len)
{
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;

  unsigned char last_c = *buff++;
  unsigned char c      = last_c;

  while (out_buff < out_end - 1) {
    c = *buff++;

    if (c == last_c) {
      unsigned long n = *buff++ + 2;

      while (n--) {
        *out_buff++ = c;
      }

      c = *buff++;
    } else {
      *out_buff++ = last_c;      
    }

    last_c = c;
  }

  // this might be wrong
  if (out_buff < out_end) {
    *out_buff++ = last_c;
  }
}

// Move-To-Front + Burrows-Wheeler Transform (?)
void unmtfbwt(unsigned char* buff,
              unsigned long  len,
              unsigned char* out_buff,
              unsigned long  out_len)
{
  unsigned long start_marker = *(unsigned long*) buff;
  buff += 4;
  len  -= 4;

  char table1[256] = { 0 };
  for (unsigned long i = 0; i < 256; i++) {
    table1[i] = (unsigned char) i;
  }

  for (unsigned long i = 0; i < len; i++) {
    unsigned char c    = table1[buff[i]];    
    unsigned char prev = table1[0];

    if (prev != c) {
      for (unsigned long j = 1; true; j++) {
        unsigned char t = table1[j];

        table1[j] = prev;
        prev = t;

        if (t == c) break;
      }

      table1[0] = c;
    }

    buff[i] = c;
  }

  unsigned long table2[256] = { 0 };
  for (unsigned long i = 0; i < len; i++) {
    table2[buff[i]]++;
  }

  unsigned long remain = len;
  for (unsigned long i = 0; i < 256; i++) {
    remain -= table2[255 - i];
    table2[255 - i] = remain;
  }

  unsigned long* table3 = new unsigned long[len];
  for (unsigned long i = 0; i < len; i++) {
    table3[table2[buff[i]]++] = i;
  }

  unsigned long marker = start_marker;
  for (unsigned long i = 0; i < out_len; i++) {
    marker      = table3[marker];
    out_buff[i] = buff[marker];
  }

  delete [] table3;
}

static const char KEY_LEN = 16;

void generate_key(void*          seed, 
                  unsigned long  seed_len,
                  unsigned char* key)
{
  unsigned char hash[32];

  SHA256 sha256;
  sha256_begin(&sha256);
  sha256_next(&sha256, (unsigned char*) seed, seed_len);
  sha256_finish(&sha256, hash);

  memset(key, 0, KEY_LEN);

  for (unsigned long i = 0; i < sizeof(hash); i++) {
    key[i % KEY_LEN] ^= hash[i];
  }
}

void unlz(unsigned char*  buff,
          unsigned long   len,
          unsigned char*  out_buff,
          unsigned long   out_len)
{
  unsigned short ring_len = *(unsigned short*) buff;
  buff += 2;

  ring_len = 2 << ring_len;
  
  unsigned char* ring     = new unsigned char[ring_len];
  unsigned long  ring_idx = 0;

  unsigned char* out_end  = out_buff + out_len;

  while (out_buff < out_end) {
    unsigned char flags = *buff++;

    for (unsigned long i = 0; i < 8 && out_buff < out_end; i++) {
      unsigned char c = *buff++;

      if (flags & 1) {
        ring[ring_idx++ % ring_len] = *out_buff++ = c;
      } else {
        unsigned long p = c | (*buff++ << 8);
        unsigned long n = *buff++ + 4;

        if (ring_idx < p) {
          p = ring_len + ring_idx - p;
        } else {
          p = ring_idx - p;
        }

        while (n-- && out_buff < out_end) {          
          ring[ring_idx++ % ring_len] = *out_buff++ = ring[p++ % ring_len];
        }
      }

      flags >>= 1;
    }    
  }

  delete [] ring;
}

class bitbuff_t {
public:
  bitbuff_t(unsigned char* buff, unsigned long len) 
    : buff(buff),
      len(len),
      saved_count(0),
      saved_bits(0)
  {} 

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

  short get_signed16(void) {
    short n = (short)get_bits(4);

    switch (n) {
      case 0:
        n = 0;
        break;
      
      case 1:
        n = 1;
        break;

      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
        n = (unsigned char)get_bits(n - 1) + (1 << (n - 1));
        break;

      case 8:
        n = -1;
        break;

      case 9:
        n = -2;
        break;

      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
        n = (unsigned char)get_bits(n - 9) - (2 << (n - 9));
        break;
    }

    return n;
  }

  short get_signed16_v2(unsigned long& repeat) {
    short n = (short)get_bits(4);

    repeat = 0;

    switch (n) {
      case 0:
        n = 0;

        {          
          for (repeat = 1; repeat < 16; repeat++) {
            if (!get_bits(1)) {
              break;
            }
          }

          if (repeat == 16) {
            repeat = 0;
          }
        }

        break;
      
      case 1:
        n = 1;
        break;

      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
        n = (unsigned char)get_bits(n - 1) + (1 << (n - 1));
        break;

      case 8:
        n = -1;
        break;

      case 9:
        n = -2;
        break;

      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
        n = (unsigned char)get_bits(n - 9) - (2 << (n - 9));
        break;
    }

    return n;
  }


  unsigned char* buff;
  unsigned long  len;
  unsigned long  saved_count;
  unsigned long  saved_bits;
};

static const int zigzag[64] = {
  0x00, 0x01, 0x08, 0x10, 0x09, 0x02, 0x03, 0x0A, 
  0x11, 0x18, 0x20, 0x19, 0x12, 0x0B, 0x04, 0x05, 
  0x0C, 0x13, 0x1A, 0x21, 0x28, 0x30, 0x29, 0x22, 
  0x1B, 0x14, 0x0D, 0x06, 0x07, 0x0E, 0x15, 0x1C, 
  0x23, 0x2A, 0x31, 0x38, 0x39, 0x32, 0x2B, 0x24, 
  0x1D, 0x16, 0x0F, 0x17, 0x1E, 0x25, 0x2C, 0x33, 
  0x3A, 0x3B, 0x34, 0x2D, 0x26, 0x1F, 0x27, 0x2E, 
  0x35, 0x3C, 0x3D, 0x36, 0x2F, 0x37, 0x3E, 0x3F
};

void ungbcf_block_v2(bitbuff_t& bits, short* block) {
  unsigned long skip = 0;      
  for (unsigned long i = 0; i < 64; i++) {
    short n = bits.get_signed16_v2(skip);
    if (n) {
      block[zigzag[i]] = n;
    } else {
      if (!skip) break;
      i += skip - 1;
    }
  }

  short* line      = block;
  short* next_line = block + 8;

  for (unsigned long y = 0; y < 8; y++) {
    for (unsigned long x = 1; x < 8; x++) {
      line[x] += line[x - 1];
    }

    line += 8;
    next_line += 8;
  }

  line      = block;
  next_line = block + 8;

  for (unsigned long y = 1; y < 8; y++) {
    for (unsigned long x = 0; x < 8; x++) {
      next_line[x] += line[x];
    }

    line += 8;
    next_line += 8;
  }
}

bool ungbcf_v2(unsigned char*  buff,
               unsigned long   len,
               const string&   filename)
{
  if (len < 4 || memcmp(buff, "GBCF", 4)) {
    return false;
  }

  GBCFHDR* hdr = (GBCFHDR*) buff;
  buff += sizeof(*hdr);
  len  -= sizeof(*hdr);

  if (hdr->depth != 24 && hdr->depth != 32) {
    return false;
  }

  unsigned long  depth_bytes = hdr->depth / 8;
  unsigned long  rgb_stride  = (hdr->width * depth_bytes + 3) & ~3;
  unsigned long  rgb_len     = rgb_stride * hdr->height;
  unsigned char* rgb_buff    = new unsigned char[rgb_len];
  memset(rgb_buff, 0, rgb_len);

  unsigned long blocks_x = (hdr->width + 7) / 8;
  unsigned long blocks_y = (hdr->height + 7) / 8;

  bitbuff_t bits(buff, len);

  for (unsigned long by = 0; by < blocks_y; by++) {
    unsigned char* out_line = rgb_buff + (by * 8) * rgb_stride;

    for (unsigned long bx = 0; bx < blocks_x; bx++) {
      unsigned char* out_block = out_line + (bx * 8) * depth_bytes;
      
      short block1[64] = { 0 }; 
      short block2[64] = { 0 };
      short block3[64] = { 0 };
      short block4[64] = { 0 };

      ungbcf_block_v2(bits, block1);

      if (depth_bytes > 1) {
        ungbcf_block_v2(bits, block2);
        ungbcf_block_v2(bits, block3);
      }

      if (depth_bytes > 3) {
        ungbcf_block_v2(bits, block4);
      }

      short* p1 = block1;
      short* p2 = block2;
      short* p3 = block3;
      short* p4 = block4;

      for (unsigned long y = 0; y < 8; y++) {
        if (by == blocks_y - 1 && (by * 8 + y) >= hdr->height) break;

        unsigned char* dst = out_block + y * rgb_stride;        

        for (unsigned long x = 0; x < 8; x++) {
          if (bx == blocks_x - 1 && (bx * 8 + x) >= hdr->width) {
            *p1++;
            *p2++;
            *p3++;
            *p4++;
            continue;
          }

          if (depth_bytes > 3) {
            dst[x * 4 + 3] = (unsigned char)*p4++;
            dst[x * 4 + 2] = (unsigned char)*p1++;
            dst[x * 4 + 1] = (unsigned char)*p2++;
            dst[x * 4 + 0] = (unsigned char)*p3++;
          } else if (depth_bytes > 1) {
            dst[x * 3 + 2] = (unsigned char)*p1++;
            dst[x * 3 + 1] = (unsigned char)*p2++;
            dst[x * 3 + 0] = (unsigned char)*p3++;
          } else {
            dst[x] = (unsigned char)*p1++;
          }
        }
      }
    }
  }

  as::write_bmp(as::get_file_prefix(filename) + ".bmp", 
                rgb_buff,
                rgb_len,
                hdr->width,
                hdr->height, 
                depth_bytes,
                as::WRITE_BMP_FLIP);

  delete [] rgb_buff;

  return true;
}

void undeltablock(short* block) {
  short* line      = block;
  short* next_line = block + 8;

  for (unsigned long i = 1; i < 8; i++) {
    block[i] += block[i - 1];

    *next_line += *line;

    line += 8;
    next_line += 8;
  }

  line      = block;
  next_line = block + 8;

  for (unsigned long y = 1; y < 8; y++) {
    for (unsigned long x = 1; x < 8; x++) {
      next_line[x] += line[x - 1];
    }

    line += 8;
    next_line += 8;
  }
}

bool ungbcf(unsigned char*  buff,
            unsigned long   len,
            const string&   filename)
{
  if (len < 4 || memcmp(buff, "GBCF", 4)) {
    return false;
  }

  GBCFHDR* hdr = (GBCFHDR*) buff;
  buff += sizeof(*hdr);
  len  -= sizeof(*hdr);

  unsigned long  depth_bytes = hdr->depth / 8;
  unsigned long  rgb_stride  = hdr->width * depth_bytes;
  unsigned long  rgb_len     = rgb_stride * hdr->height;
  unsigned char* rgb_buff    = new unsigned char[rgb_len];
  memset(rgb_buff, 0, rgb_len);

  unsigned long blocks_x = (hdr->width + 7) / 8;
  unsigned long blocks_y = (hdr->height + 7) / 8;

  bitbuff_t bits(buff, len);

  for (unsigned long by = 0; by < blocks_y; by++) {
    unsigned char* out_line = rgb_buff + (by * 8) * rgb_stride;

    for (unsigned long bx = 0; bx < blocks_x; bx++) {
      unsigned char* out_block = out_line + (bx * 8) * depth_bytes;
      
      short block1[64] = { 0 }; 
      short block2[64] = { 0 };
      short block3[64] = { 0 };
      short block4[64] = { 0 };

      short last = 0;      
      for (unsigned long i = 0; i < 64; i++) {
        block1[zigzag[i]] = last = last + bits.get_signed16();
      }
      
      if (depth_bytes > 1) {
        for (unsigned long i = 0; i < 64; i++) block2[zigzag[i]] = bits.get_signed16();                      
        for (unsigned long i = 0; i < 64; i++) block3[zigzag[i]] = bits.get_signed16();     
      }
            
      if (depth_bytes > 3) {
        for (unsigned long i = 0; i < 64; i++) block4[zigzag[i]] = bits.get_signed16();                        
      }

      undeltablock(block2);
      undeltablock(block3);
      undeltablock(block4);

      short* p1 = block1;
      short* p2 = block2;
      short* p3 = block3;
      short* p4 = block4;

      for (unsigned long y = 0; y < 8; y++) {
        if (by == blocks_y - 1 && (by * 8 + y) >= hdr->height) break;

        unsigned char* dst = out_block + y * rgb_stride;        

        for (unsigned long x = 0; x < 8; x++) {
          if (bx == blocks_x - 1 && (bx * 8 + x) >= hdr->width) break;

          if (depth_bytes > 3) {
            dst[x * 4 + 3] = (unsigned char)*p1++ - 128;
            //dst[x * 4 + 3] = 0xFF;
            dst[x * 4 + 2] = (unsigned char)*p2++ - 128;
            dst[x * 4 + 1] = (unsigned char)*p3++ - 128;
            dst[x * 4 + 0] = (unsigned char)*p4++ - 128;
          } else if (depth_bytes > 1) {
            dst[x * 3 + 2] = (unsigned char)*p1++ - 128;
            dst[x * 3 + 1] = (unsigned char)*p2++ - 128;
            dst[x * 3 + 0] = (unsigned char)*p3++ - 128;
          } else {
            dst[x] = (unsigned char)*p1++ - 128;
          }
        }
      }
    }
  }

  unsigned char pal[256][4] = { 0 };
  for (unsigned long i = 0; i < 256; i++) {
    pal[i][0] = pal[i][1] = pal[i][2] = (unsigned char) i;
  }

  as::write_bmp_ex(as::get_file_prefix(filename) + ".bmp", 
                   rgb_buff,
                   rgb_len,
                   hdr->width,
                   hdr->height, 
                   depth_bytes,
                   256,
                   pal,
                   as::WRITE_BMP_FLIP);

  delete [] rgb_buff;

  return true;
}

void read_decrypt(int            fd,
                  unsigned long  offset,
                  unsigned long  len,
                  void*          seed, 
                  void*          out_buff,
                  unsigned long  out_len,
                  unsigned long  flags)
{
  unsigned char key1[KEY_LEN] = { 0 };
  generate_key(seed, 8, key1);

  unsigned char key2[KEY_LEN] = { 0 };
  generate_key(key1, sizeof(key1), key2);

  unsigned char* buff = new unsigned char[len];
  lseek(fd, offset, SEEK_SET);
  read(fd, buff, len);

  unsigned char* decr_buff = new unsigned char[len];

  switch (flags & 0xF0000) {
    case 0xA0000:
      {
        CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryptor;    
        decryptor.SetKeyWithIV(key1, sizeof(key1), key2, sizeof(key2));
        decryptor.ProcessData(decr_buff, buff, len);
      }
      break;

    case 0x10000: 
      hobocrypt_decrypt(buff, decr_buff, len, key1, key2);
      break;

    case 0x20000: 
      hobocrypt256_decrypt(buff, decr_buff, len, key1, key2);
      break;

    case 0x30000:      
      hobocrypt512_decrypt(buff, decr_buff, len, key1, key2);
      break;

    case 0x80000: 
      {
        CryptoPP::CFB_Mode<CryptoPP::RC6>::Decryption decryptor;    
        decryptor.SetKeyWithIV(key1, sizeof(key1), key2, sizeof(key2));   
        decryptor.ProcessData(decr_buff, buff, len);
      }
      break;

    default:      
      memcpy(out_buff, buff, out_len);
      
      delete [] decr_buff;
      delete [] buff;
      return;
  }

  delete [] buff;
  buff = decr_buff;

  // These flag checks might not be quite right...

  if (flags & 0xFF) {
    // Don't know the size of all uncompressed chunks.  Should be less
    // than the total final size but add a fudge just in case.
    unsigned long  temp_len  = out_len + (1024 * 1024);
    unsigned char* temp_buff = new unsigned char[temp_len];    
    temp_len = unrangecode(buff, len, temp_buff, temp_len);

    delete [] buff;

    len  = temp_len;
    buff = temp_buff;
  }

  switch (flags & 0xF00) {
    default:
    case 0:
      memcpy(out_buff, buff, out_len);
      break;

    case 0x400:
      {
        unsigned long  temp_len  = *(unsigned long*) buff;
        unsigned char* temp_buff = new unsigned char[temp_len];
        unrle(buff + 4, len - 4, temp_buff, temp_len);
        unmtfbwt(temp_buff, temp_len, (unsigned char*) out_buff, out_len);
        delete [] temp_buff;
      }
      break;

    case 0x700:
      unlz(buff + 4, len - 4, (unsigned char*)out_buff, out_len);
      break;
  }

  delete [] buff;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "expcf v1.04, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pcf>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PCFHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long entries_len = sizeof(PCFENTRY) * hdr.entry_count;
  PCFENTRY*     entries     = new PCFENTRY[hdr.entry_count + 1];
  read_decrypt(fd, 
               sizeof(hdr) + hdr.toc_offset, 
               hdr.toc_length, 
               &hdr.seeds[6],
               entries, 
               entries_len, 
               hdr.flags);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].original_length;
    unsigned char* buff = new unsigned char[len];
    read_decrypt(fd, 
                 sizeof(hdr) + entries[i].offset,
                 entries[i].length,
                 &entries[i].seeds[2],
                 buff,
                 len,
                 entries[i].flags);

    as::make_path(entries[i].filename);

#if VERSION >= 2
    if (ungbcf_v2(buff, len, entries[i].filename)) {
#else
    if (ungbcf(buff, len, entries[i].filename)) {
#endif
    } else {      
      as::write_file(entries[i].filename, buff, len);
    }

    delete [] buff;
  }

  delete [] entries;

  return 0;
}