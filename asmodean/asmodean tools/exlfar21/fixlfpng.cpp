// fixlfpng.cpp, v1.0 2009/12/19
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#include "as-util.h"
#include <list>
#include <png.h>

struct inStHDR {
  unsigned short width;
  unsigned short height;
  unsigned short block_width;
  unsigned short block_height;
  unsigned short is_continued;
  unsigned short entry_count;
  unsigned short unknown2;
};

struct inStENTRY {
  short src_x;
  short src_y;
  short ukn_x;
  short ukn_y;
  short dst_x;
  short dst_y;
};

// libpng has a pretty disgusting API
void read_png(const string&   filename, 
              unsigned long&  width, 
              unsigned long&  height, 
              unsigned long&  depth,
              unsigned char*& buff, 
              unsigned long&  len,
              unsigned char*& inst_buff,
              unsigned long&  inst_len)
{
#define LOADFAIL(m) { printf("%s: read_png failed %s\n", filename.c_str(), m); exit(-1); }

  FILE *fp = fopen(filename.c_str(), "rb");

  if (!fp) LOADFAIL("fopen()");

  unsigned char png_hdr[8];
  fread(png_hdr, 1, 8, fp);
  if (png_sig_cmp(png_hdr, 0, 8)) LOADFAIL("png_sig_cmp()");

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) LOADFAIL("png_create_struct()");  

  // Why doesn't it work this way?
  //png_byte inSt[] = { 0x69, 0x6E, 0x53, 0x74, (png_byte) '\0' };
  //png_set_keep_unknown_chunks(png_ptr, 2, inSt, 1);
  png_set_keep_unknown_chunks(png_ptr, 2, NULL, 0);

	png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) LOADFAIL("png_create_info_struct()");

  if (setjmp(png_jmpbuf(png_ptr))) LOADFAIL("setjmp(png_jmpbuf())");
  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);    
  png_read_info(png_ptr, info_ptr);  

  width  = info_ptr->width;
  height = info_ptr->height;
  depth  = info_ptr->pixel_depth / 8;
	len    = info_ptr->height * info_ptr->rowbytes;
  buff   = new unsigned char[len];

  png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);     

	png_bytep *row_pointers = new png_bytep[height]; 

  for (unsigned long y = 0; y < height; y++) {
    row_pointers[y] = buff + (info_ptr->rowbytes * y);
  }

  png_set_rows(png_ptr, info_ptr, row_pointers);

  if (setjmp(png_jmpbuf(png_ptr))) LOADFAIL("png_read_image()");
  png_read_image(png_ptr, row_pointers);
  png_read_end(png_ptr, NULL);    

  for (unsigned long i = 0; i < info_ptr->unknown_chunks_num; i++) {
    if (!memcmp(info_ptr->unknown_chunks[i].name, "inSt", 4)) {
      inst_len  = info_ptr->unknown_chunks[i].size;
      inst_buff = new unsigned char[inst_len];
      memcpy(inst_buff, info_ptr->unknown_chunks[i].data, inst_len);

      break;
    }
  }

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);

  delete [] row_pointers;

#undef LOADFAIL
}

void process_plain(const string&  filename,
                   unsigned char* buff,
                   unsigned long  len,
                   unsigned long  width,
                   unsigned long  height,
                   bool           do_cleanup)
{
  if (width != 1024 || height != 1024) {
    fprintf(stderr, "%s: don't support this\n", filename.c_str());
    return;
  }

  unsigned long  stride     = width * 4;

  unsigned long  out_width  = 1280;
  unsigned long  out_height = 720;
  unsigned long  out_stride = out_width * 4;
  unsigned long  out_len    = out_height * out_stride;
  unsigned char* out_buff   = new unsigned char[out_len];

  for (unsigned long y = 0; y < out_height; y++) {
    as::RGBA* src_line = (as::RGBA*) (buff + (y + 22) * stride);
    as::RGBA* dst_line = (as::RGBA*) (out_buff + y * out_stride);

    for (unsigned long x = 0; x < width; x++) {
      dst_line[x] = src_line[x];
    }

    unsigned long remain = out_width - width;

    for (unsigned long x = 0; x < remain; x++) {
      src_line = (as::RGBA*) (buff + (height - 1 - x) * stride);

      dst_line[width + x] = src_line[y + 22];
    }
  }

  as::write_bmp(as::get_file_prefix(filename) + ".bmp",
                out_buff,
                out_len,
                out_width,
                out_height,
                4,
                as::WRITE_BMP_FLIP | as::WRITE_BMP_BGR);

  if (do_cleanup) {
    unlink(filename.c_str());
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "fixlfpng v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.png> [-cleanup]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix(as::get_file_prefix(in_filename));
  bool   do_cleanup = argc > 2 && !strcmp(argv[2], "-cleanup");

  if (*prefix.rbegin() == ']') {
    fprintf(stderr, "%s: skipping group subpart.\n", in_filename.c_str());
    return 0;
  }

  typedef std::list<string> cleanup_list_t;
  cleanup_list_t cleanup_list;

  unsigned long  out_width  = 1280;
  unsigned long  out_height = 760;
  unsigned long  out_stride = 0;
  unsigned long  out_len    = 0;
  unsigned char* out_buff   = NULL;

  bool done  = false;
  bool retry = true;

  for (long n = 0; retry || !done; n++) {
    string filename = in_filename;

    if (retry) {
      n = 0;

      delete [] out_buff;

      out_stride = out_width * 4;
      out_len    = out_height * out_stride;
      out_buff   = new unsigned char[out_len];
      memset(out_buff, 0, out_len);
    }

    retry = false;

    if (n > 0) {
      filename = prefix + as::stringf("[%d].png", n);
    }

    unsigned long  width     = 0;
    unsigned long  height    = 0;
    unsigned long  depth     = 0;
    unsigned long  len       = 0;
    unsigned char* buff      = NULL;
    unsigned long  inst_len  = 0;
    unsigned char* inst_buff = NULL;
    read_png(filename, width, height, depth, buff, len, inst_buff, inst_len);
    unsigned long  stride    = width * 4;

    if (depth != 4) {
      fprintf(stderr, "%s: unsupported color depth\n", filename.c_str());
      return 0;
    }

    if (!inst_buff) {
      process_plain(filename, buff, len, width, height, do_cleanup);
      return 0;
    }

    cleanup_list.push_back(filename);

    inStHDR*   hdr     = (inStHDR*) inst_buff;
    inStENTRY* entries = (inStENTRY*) (hdr + 1);

    for (unsigned long i = 0; i < hdr->entry_count; i++) {
      entries[i].dst_x += short(out_width / 2);
      entries[i].dst_y += short(out_height / 2);

      // This is a hack since I don't feel like redesigning to
      // pre-scan the min/max dimensions...
      if (entries[i].dst_x < 0 || entries[i].dst_y < 0) {
        if (entries[i].dst_x < 0) out_width += hdr->block_width;
        if (entries[i].dst_y < 0) out_height += hdr->block_height;
        retry = true;
        break;
      }      

      for (unsigned long y = 0; !retry && y + 1 < hdr->block_height; y++) {
        as::RGBA* src_line = (as::RGBA*) (buff     + (entries[i].src_y + y) * stride);
        as::RGBA* dst_line = (as::RGBA*) (out_buff + (entries[i].dst_y + y) * out_stride);

        for (unsigned long x = 0; x + 1 < hdr->block_width; x++) {
          as::RGBA& src = src_line[entries[i].src_x + x];

          if (entries[i].dst_x + x >= out_width || entries[i].dst_y + y >= out_height) {
            // Don't expand for block padding...
            if (src.r == 0 && src.g == 0 && src.b == 0 && src.a == 0) {
              continue;
            }

            if (entries[i].dst_x + x >= out_width) out_width += hdr->block_width;
            if (entries[i].dst_y + y >= out_height) out_height += hdr->block_height;

            retry = true;
            break;
          }

          dst_line[entries[i].dst_x + x] = src;
        }
      }
    }

    done = !hdr->is_continued;

    delete [] inst_buff;
    delete [] buff;
  }

  as::write_bmp(as::get_file_prefix(in_filename) + ".bmp",
                out_buff,
                out_len,
                out_width,
                out_height,
                4,
                as::WRITE_BMP_FLIP | as::WRITE_BMP_BGR);

  if (do_cleanup) {
    for (cleanup_list_t::iterator i = cleanup_list.begin();
         i != cleanup_list.end();
         ++i) {
      unlink(i->c_str());
    }
  }

  return 0;
}
