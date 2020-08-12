// lzss.h, v1.02 2006/11/17
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)
//   icq:   55244079

// Mini library to decompress LZSS

#ifndef LZSS_H
#define LZSS_H

unsigned long unlzss(unsigned char* buff, 
                     unsigned long  len,
                     unsigned char* out_buff, 
                     unsigned long  out_len, 
                     unsigned long  ring_len      = 4096,
                     unsigned long  forward_len   = 18,
                     unsigned long  length_offset = 3);

#endif /* LZSS_H  */