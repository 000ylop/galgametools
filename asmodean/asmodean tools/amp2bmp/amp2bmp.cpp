// amp2bmp.cpp, v1.0 2007/01/06
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool decompresses and fixes AMNP (*.AMP) images.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdio>
#include <string>
#include <algorithm>
#include <png.h>

struct AMNPHDR {
  unsigned char signature[4]; // "AMNP" 
};

static const long PNGFRAGLEN = 21;

struct memory_reader_state_t { 
  unsigned char* buff; 
  unsigned long  len;
  unsigned long  pos;
};

void memory_reader(png_structp png_ptr, png_bytep data, png_uint_32 length) { 
   memory_reader_state_t* state = (memory_reader_state_t*) png_get_io_ptr(png_ptr); 

   if (length > (state->len - state->pos)) {
     png_error(png_ptr, "out of data in png_read_memory()"); 
   }

   memcpy(data, state->buff + state->pos, length); 
   state->pos += length; 
} 

void loadfail(char* msg) {
  fprintf(stderr, "%s\n", msg);
  exit(-1);
}

void decompress_fix_png(unsigned char*  buff,
                        unsigned long   len, 
                        unsigned long&  width,
                        unsigned long&  height,
                        unsigned char*& rgb_buff,
                        unsigned long& rgb_len) 
{
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    loadfail("png_create_struct()");

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (!info_ptr)
    loadfail("png_create_info_struct()");

  if (setjmp(png_jmpbuf(png_ptr)))
    loadfail("setjmp(png_jmpbuf())");

  memory_reader_state_t read_state;
  read_state.buff = buff; 
  read_state.len  = len;
  read_state.pos  = 0; 

  png_set_read_fn(png_ptr, &read_state, (png_rw_ptr)memory_reader); 
  png_set_sig_bytes(png_ptr, 8);    
  png_read_info(png_ptr, info_ptr);

  width  = info_ptr->width;
  height = info_ptr->height;

  png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);   

  rgb_len  = info_ptr->height * info_ptr->rowbytes;
  rgb_buff = new unsigned char[rgb_len];

	png_bytep *row_pointers = new png_bytep[height]; 

  for (unsigned long y = 0; y < height; y++) {
    row_pointers[y] = rgb_buff + (height - y - 1 ) * info_ptr->rowbytes;
  }

  png_set_rows(png_ptr, info_ptr, row_pointers);

  if (setjmp(png_jmpbuf(png_ptr))) 
    loadfail("read_image()");

  png_read_image(png_ptr, row_pointers);	

  for (y = 0; y < height; y++) {
    unsigned char* line = rgb_buff + (info_ptr->rowbytes * y);

    for (unsigned long x = 0; x < width; x++) {
      line[x * 4 + 3] = 0xFF - line[x * 4 + 3];
    }
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  delete [] row_pointers;
}

using std::string;

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "amp2bmp, v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.amp> [output.bmp]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename;

  if (argc > 2) {
    out_filename = argv[2];
  } else {  
    out_filename          = in_filename; 
    string::size_type pos = out_filename.find_last_of(".");

    if (pos != string::npos)
      out_filename = out_filename.substr(0, pos);

    out_filename += ".bmp";
  }

  int fd = open(in_filename.c_str(), O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s\n", in_filename.c_str());
    return -1;
  }

  AMNPHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long len;
  {
    struct stat file_stat;
    fstat(fd, &file_stat);
    len = file_stat.st_size - sizeof(hdr) - PNGFRAGLEN;
  }

  // Skip extra null at the end (always?)
  len--;

  // Big enough to hold the fixed header stuff
  unsigned long  png_len  = 8 + PNGFRAGLEN + 4 + len;
  unsigned char* png_buff = new unsigned char[png_len];

  // Produce a complete IHDR and first IDAT
  {
    unsigned char* p = png_buff;

    memcpy(p, "\x00\x00\x00\x0DIHDR", 8);
    p += 8;

    read(fd, p, PNGFRAGLEN);  
    p += PNGFRAGLEN;

    memcpy(p, "IDAT", 4);
    p += 4;

    read(fd, p, len);
  }

  close(fd);

  unsigned long  width     = 0;
  unsigned long  height    = 0;
  unsigned long  rgb_len   = 0;
  unsigned char* rgb_buff = NULL;

  decompress_fix_png(png_buff, png_len, width, height, rgb_buff, rgb_len);

  fd = open(out_filename.c_str(), O_CREAT | O_TRUNC | O_RDWR | O_BINARY, S_IREAD | S_IWRITE);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s\n", out_filename.c_str());
    return -1;
  }

  {
    BITMAPFILEHEADER bmf;
    BITMAPINFOHEADER bmi;

    memset(&bmf, 0, sizeof(bmf));
    memset(&bmi, 0, sizeof(bmi));

    bmf.bfType     = 0x4D42;
    bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + rgb_len;
    bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi);

    bmi.biSize     = sizeof(bmi);
    bmi.biWidth    = width;
    bmi.biHeight   = height;
    bmi.biPlanes   = 1;
    bmi.biBitCount = 32;
    
    write(fd, &bmf, sizeof(bmf));
    write(fd, &bmi, sizeof(bmi));
  }

  write(fd, rgb_buff, rgb_len);
  close(fd);

  delete [] rgb_buff;
  delete [] png_buff;

  return 0;
}
