// bt22bmp.cpp, v1.0 2007/01/21
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts Asmik Ace's *.bt2 images to bitmaps.  There's an
// outstanding bug unswizzling some of the images...

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cmath>

struct BT2HDR {
  unsigned long unknown1;
  unsigned long width;
  unsigned long height;
  unsigned long width_excess;  // beyond even swizzle block width
  unsigned long height_excess; // beyond even swizzle block height
  unsigned long table_offset;  // swizzle block offsets
  unsigned long unknown4;
  unsigned long unknown5;
};

static const unsigned long SWIZ_WIDTH  = 128;
static const unsigned long SWIZ_HEIGHT = 64;

void unswizzle(unsigned char* in, 
               unsigned char* out,                
               unsigned long  width, 
               unsigned long  height,
               unsigned long  width_excess,
               unsigned long  height_excess,
               unsigned long  pixel_bytes,
               unsigned long  block_width,
               unsigned long  block_height,
               unsigned long* offset_table)
{
  unsigned long blocks_per_row = (unsigned long) ceil((double)width / 
                                                        (double)block_width);

  for (unsigned long y = 0; y < height; y++) {
    for (unsigned long x = 0; x < width; x++) {
      unsigned int blockx       = x / block_width;
      unsigned int blocky       = y / block_height;     

      unsigned int blocki       = blockx + blocky * blocks_per_row;
      unsigned int block_offset = offset_table[blocki] - offset_table[0];
 
      unsigned int srcx         = x - blockx * block_width;
      unsigned int srcy         = y - blocky * block_height;

      // BUG: This is clunky and doesn't always work.
      //      (when width_excess > block_width/2, I think)
      if (x >= width - width_excess) {
        srcy /= block_width / width_excess;
      }
 
      // Flip the image while we're at it
      memcpy(out + ((height - y - 1) * width + x) * pixel_bytes, 
             in + (block_offset + srcy * block_width + srcx) * pixel_bytes,
             pixel_bytes);
     }
  }
}

// Funky palette re-ordering.  I have no idea why.
unsigned long get_pal_index(unsigned long n) {  
  if ((n & 31) >= 8) {
    if ((n & 31) < 16) {
      n += 8; // +8 - 15 to +16 - 23
    } else if ((n & 31) < 24) {
      n -= 8; // +16 - 23 to +8 - 15
    }
  }

  return n;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "bt22bmp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bt2> <output.bmp>\n", argv[0]);
    return -1;
  }

  char* in_filename  = argv[1];
  char* out_filename = argv[2];

  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  BT2HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned char pal[1024];
  read(fd, pal, sizeof(pal));

  unsigned char fixed_pal[1024];

  for (unsigned long i = 0; i < 256; i++) {
    memcpy(fixed_pal + i * 4, pal + get_pal_index(i) * 4, 4);
  }

  unsigned long  len  = hdr.width * hdr.height;
  unsigned char* buff = new unsigned char[len];

  unsigned long  block_count 
    = (unsigned long) (ceil((double)hdr.width / (double)SWIZ_WIDTH) * 
                         ceil((double)hdr.height / (double)SWIZ_HEIGHT));
  unsigned long* offset_table = new unsigned long[block_count];
  read(fd, offset_table, sizeof(long) * block_count);

  // I've seen extra entries after the table (padding?), so let's avoid it.
  lseek(fd, -1 * len, SEEK_END);
  read(fd, buff, len);
  close(fd);  

  unsigned char* out_buff = new unsigned char[len];

  unswizzle(buff, 
            out_buff, 
            hdr.width, 
            hdr.height, 
            hdr.width_excess, 
            hdr.height_excess, 
            1, 
            SWIZ_WIDTH, 
            SWIZ_HEIGHT, 
            offset_table);

  fd = open(out_filename, O_CREAT | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", out_filename, strerror(errno));
    return -1;
  }

  {
    BITMAPFILEHEADER bmf;
    BITMAPINFOHEADER bmi;

    memset(&bmf, 0, sizeof(bmf));
    memset(&bmi, 0, sizeof(bmi));

    bmf.bfType     = 0x4D42;
    bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + sizeof(pal) + len;
    bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi) + sizeof(pal);

    bmi.biSize     = sizeof(bmi);
    bmi.biWidth    = hdr.width;
    bmi.biHeight   = hdr.height;
    bmi.biPlanes   = 1;
    bmi.biBitCount = 8;
    
    write(fd, &bmf, sizeof(bmf));
    write(fd, &bmi, sizeof(bmi));
    write(fd, fixed_pal, sizeof(fixed_pal));
  }

  write(fd, out_buff, len);
  close(fd);

  delete [] out_buff;
  delete [] buff;

  return 0;
}
