// arc4common, v1.01 2007/03/13
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This is a set of common functionality used by exarc4 and fcb2bmp.

#ifndef __ARC4COMMON_H__
#define __ARC4COMMON_H__

#include <string>

//*****************************************************************************
// Generic utilities
//*****************************************************************************

//-----------------------------------------------------------------------------
// open_or_die
//-----------------------------------------------------------------------------
int open_or_die(const std::string& filename, int flags, int mode = 0);

//-----------------------------------------------------------------------------
// get_file_size
//-----------------------------------------------------------------------------
unsigned long get_file_size(int fd);

//-----------------------------------------------------------------------------
// get_file_prefix
//-----------------------------------------------------------------------------
std::string get_file_prefix(const std::string& filename);

//-----------------------------------------------------------------------------
// get_file_extension
//-----------------------------------------------------------------------------
std::string get_file_extension(const std::string& filename);

//-----------------------------------------------------------------------------
// flip_endian
//-----------------------------------------------------------------------------
unsigned long flip_endian(unsigned long x);

//*****************************************************************************
// ARC4 related functions
//*****************************************************************************

#pragma pack(1)
struct ARC4COMPHDR {
  unsigned short type; // 0x5A74
  unsigned long  length;
};

struct ARC4COMPCHUNKHDR {
  unsigned short type; // 0x745A or 0x7453
  unsigned short length;
  unsigned short original_length;
  unsigned short seed;
};
#pragma pack()

//-----------------------------------------------------------------------------
// unobfuscate
//-----------------------------------------------------------------------------
void unobfuscate(unsigned char* buff, 
                 unsigned long  len, 
                 unsigned long  seed);

//-----------------------------------------------------------------------------
// uncompress
//-----------------------------------------------------------------------------
unsigned long uncompress(unsigned char* buff, 
                         unsigned long  len,
                         unsigned char* out_buff,
                         unsigned long  out_len);

//-----------------------------------------------------------------------------
// uncompress_sequence
//-----------------------------------------------------------------------------
void uncompress_sequence(unsigned char*     buff, 
                         unsigned long      len,
                         unsigned char*&    out_buff,
                         unsigned long&     out_len,
                         const std::string& filename);

//-----------------------------------------------------------------------------
// get_stupid_long
//-----------------------------------------------------------------------------
unsigned long get_stupid_long(unsigned char* buff);

#endif /* __ARC4COMMON_H__ */
