// arc4common.cpp, v1.01 2007/03/13
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This is a set of common functionality used by exarc4 and fcb2bmp.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "arc4common.h"

using std::string;

//*****************************************************************************
// Generic utilities
//*****************************************************************************

//-----------------------------------------------------------------------------
// open_or_die
//-----------------------------------------------------------------------------
int open_or_die(const string& filename, int flags, int mode) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

//-----------------------------------------------------------------------------
// get_file_size
//-----------------------------------------------------------------------------
unsigned long get_file_size(int fd) {
  struct stat file_stat;
  fstat(fd, &file_stat);
  return file_stat.st_size;
}

//-----------------------------------------------------------------------------
// get_file_prefix
//-----------------------------------------------------------------------------
string get_file_prefix(const std::string& filename) {
  string temp(filename);

  string::size_type pos = temp.find_last_of(".");

  if (pos != string::npos) {
    temp = temp.substr(0, pos);
  }

  return temp;
}

//-----------------------------------------------------------------------------
// get_file_prefix
//-----------------------------------------------------------------------------
string get_file_extension(const std::string& filename) {
  string temp;

  string::size_type pos = filename.find_last_of(".");

  if (pos != string::npos) {
    temp = filename.substr(pos + 1);
  }

  return temp;
}

//-----------------------------------------------------------------------------
// flip_endian
//-----------------------------------------------------------------------------
unsigned long flip_endian(unsigned long x) {
  return (x >> 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x << 24);
}

//*****************************************************************************
// ARC4 related functions
//*****************************************************************************

//-----------------------------------------------------------------------------
// unobfuscate
//-----------------------------------------------------------------------------
void unobfuscate(unsigned char* buff, 
                 unsigned long  len, 
                 unsigned long  seed) 
{
  unsigned short* p   = (unsigned short*) buff;
  unsigned short* end = (unsigned short*) (buff + len);

  while (p < end) {
    seed *= 0x1465D9;
    seed += 0x0FB5;

    *p++ -= (seed >> 16);
  }
}

//-----------------------------------------------------------------------------
// uncompress
//-----------------------------------------------------------------------------
unsigned long uncompress(unsigned char* buff, 
                         unsigned long  len,
                         unsigned char* out_buff,
                         unsigned long  out_len)
{
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;

  while (true) {
    unsigned long  c      = *buff++;
    unsigned long  p      = 0;
    unsigned long  n      = 0;
    unsigned char* source = NULL;

    if (c & 0x80) {
      if (c & 0x40) {
        if (c & 0x20) {
          c = (c << 8) | *buff++;
          c = (c << 8) | *buff++;

          n = (c & 0x3F) + 4;
          p = (c >> 6) & 0x7FFF;
        } else {
          c = (c << 8) | *buff++;

          n = (c & 0x07) + 3;
          p = (c >> 3) & 0x3FF;
        }
      } else {
        n = (c & 0x03) + 2;
        p = (c >> 2) & 0x0F;
      }

      source = out_buff - p - 1;
    } else {
      n      = c;
      p      = 0;
      source = buff;

      buff += n;

      if (!n) {
        break;
      }
    }

    while (n--) {
      *out_buff++ = *source++;
    }
  }

  return out_len - (out_end - out_buff);
}

//-----------------------------------------------------------------------------
// uncompress_sequence
//-----------------------------------------------------------------------------
void uncompress_sequence(unsigned char*  buff, 
                         unsigned long   len,
                         unsigned char*& out_buff,
                         unsigned long&  out_len,
                         const string&   filename) 
{
  ARC4COMPHDR* hdr = (ARC4COMPHDR*) buff;
  buff += sizeof(*hdr);

  out_len  = hdr->length;
  out_buff = new unsigned char[out_len];

  unsigned char* end     = buff + len;
  unsigned char* out     = out_buff;
  unsigned char* out_end = out_buff + out_len;

  while (buff < end && out < out_end) {
    ARC4COMPCHUNKHDR* chunk = (ARC4COMPCHUNKHDR*) buff;
    buff += sizeof(*chunk);

    unobfuscate(buff, chunk->length, chunk->seed);

    if (chunk->type == 0x745A) {      
      uncompress(buff, chunk->length, out, chunk->original_length);
    } else if (chunk->type == 0x7453) {
      memcpy(out, buff, chunk->original_length);
    } else {
      fprintf(stderr, "%s: unknown chunk type: 0x%04X\n", 
              filename.c_str(), chunk->type);
    }

    buff += chunk->length;
    out  += chunk->original_length;
  }
}

//-----------------------------------------------------------------------------
// get_stupid_long
//-----------------------------------------------------------------------------
unsigned long get_stupid_long(unsigned char* buff) {
  return (buff[0] << 16) | (buff[1] << 8) | buff[2];
}
