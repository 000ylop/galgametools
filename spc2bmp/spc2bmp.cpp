// spc2bmp.cpp, v1.0 2011/05/01
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts *.spc / xtx images to bitmaps.

#include "as-util.h"
#include "as-lzss.h"
#include <windows.h>
#include <d3d9.h>
#include <Xgraphics.h>

struct SPCHDR {
  unsigned long original_length;
};

struct SPCENTRY {
  unsigned long offset;
  unsigned long length;
  unsigned long unknown1;
  unsigned long unknown2;
};

struct XTXHDR {
  unsigned char signature[4]; // "xtx"
  unsigned long type;
  unsigned long tile_width;
  unsigned long tile_height;
  unsigned long width;
  unsigned long height;
  unsigned long offset_x;
  unsigned long offset_y;

  void flip_endian(void) {
    tile_width  = as::flip_endian(tile_width);
    tile_height = as::flip_endian(tile_height);
    width       = as::flip_endian(width);
    height      = as::flip_endian(height);
    offset_x    = as::flip_endian(offset_x);
    offset_y    = as::flip_endian(offset_y);
  }
};

static const unsigned long XTX_TYPE_32BIT   = 0;

// Some kind of 8-bit format but I couldn't figure out what it was.
static const unsigned long XTX_TYPE_UNKNOWN = 2;


void proc_xtx(const string&  prefix,
              unsigned char* buff,
              unsigned long  len)
{
  XTXHDR* hdr = (XTXHDR*) buff;
  hdr->flip_endian();

  unsigned long depth = 0;

  switch (hdr->type) {
  case XTX_TYPE_32BIT:
    depth = 4;
    break;

  case XTX_TYPE_UNKNOWN:
  default:
    printf("%s: unsupported type %d\n", prefix.c_str(), hdr->type);
    return;
  }

  unsigned long  data_len  = len  - sizeof(*hdr);
  unsigned char* data_buff = buff + sizeof(*hdr);

  unsigned long  out_stride = (hdr->width * depth + 3) & ~3;
  unsigned long  out_len    = out_stride * hdr->height;
  unsigned char* out_buff   = new unsigned char[out_len];

  POINT point = { 0, 0 };
  RECT  rect  = { 0, 0, hdr->width, hdr->height };

  XGUntileSurface(out_buff,
                  out_stride,
                  &point,
                  data_buff,
                  hdr->tile_width,
                  hdr->tile_height,
                  &rect,
                  depth);

  string out_filename;

  if (hdr->offset_x || hdr->offset_y) {
    out_filename = prefix + as::stringf("+x%dy%d.bmp", hdr->offset_x, hdr->offset_y);
  } else {
    out_filename = prefix + ".bmp";
  }

  as::write_bmp(out_filename,
                out_buff,
                data_len,
                hdr->width,
                hdr->height,
                depth,
                as::WRITE_BMP_BIGENDIAN | as::WRITE_BMP_FLIP);

  delete [] out_buff;
}

void proc_spc(const string&  prefix,
              unsigned char* buff, 
              unsigned long  len)
{
  SPCENTRY* entries = (SPCENTRY*) buff;

  unsigned long entry_count = entries[0].offset / sizeof(SPCENTRY);

  for (unsigned long i = 0; i < entry_count; i++) {
    string out_filename = prefix + as::stringf("+%03d", i);

    unsigned char* data_buff = buff + entries[i].offset;
    unsigned long  data_len  = entries[i].length;

    if (data_len >= 3 && !memcmp(data_buff, "xtx", 3)) {  
      proc_xtx(out_filename, data_buff, data_len);      
    } else {
      proc_spc(out_filename, data_buff, data_len);
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "spc2bmp, v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.spc>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix = as::get_file_prefix(in_filename);

  int fd = as::open_or_die(in_filename, O_RDWR | O_BINARY);

  unsigned long  len  = as::get_file_size(fd);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  if (len >= 3 && !memcmp(buff, "xtx", 3)) {
    proc_xtx(prefix, buff, len);
  } else if (len >= sizeof(SPCHDR)) {
    SPCHDR* hdr = (SPCHDR*) buff;

    unsigned long  out_len  = hdr->original_length;
    unsigned char* out_buff = new unsigned char[out_len*5];
    as::unlzss(buff + sizeof(*hdr), 
               len  - sizeof(*hdr), 
               out_buff, 
               out_len);

    proc_spc(prefix, out_buff, out_len);

    delete [] out_buff;
  }

  delete [] buff;

  close(fd);

  return 0;
}
