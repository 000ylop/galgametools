// lzss.cpp, v1.0 2006/11/01
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)
//   icq:   55244079

// Mini library to decompress LZSS

#include "lzss.h"
#include <cstring>

unsigned long unlzss(unsigned char* buff, 
                     unsigned long  len,
                     unsigned char* out_buff, 
                     unsigned long  out_len, 
                     unsigned long  ring_len,
                     unsigned long  forward_len,
                     unsigned long  length_offset) 
{
  unsigned char* ring       = new unsigned char[ring_len];
  unsigned long  ring_index = ring_len - forward_len; // why?
  unsigned char* end        = buff + len;
  unsigned char* out_end    = out_buff + out_len;

  memset(ring, 0, ring_len);

  while (buff < end && out_buff < out_end) {
    unsigned char flags = *buff++;

    for (int i = 0; i < 8 && buff < end && out_buff < out_end; i++) {
      if (flags & 0x01) {
        *out_buff++ = ring[ring_index++ % ring_len] = *buff++;
      } else {
        if (end - buff < 2)
          break;

        unsigned long p = *buff++;
        unsigned long n = *buff++;
                                
        p |= (n & 0xF0) << 4;
        n  = (n & 0x0F) + length_offset;

        n = n;

        for (unsigned long j = 0; j < n && out_buff < out_end; j++) {
          *out_buff++ = ring[ring_index++ % ring_len] = ring[p++ % ring_len];
        }
      }

      flags >>= 1;
    }
  }

  delete [] ring;

  return out_len - (out_end - out_buff);
}
