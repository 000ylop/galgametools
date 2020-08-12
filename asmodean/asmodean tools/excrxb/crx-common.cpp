// crx-common.cpp, v1.02 2012/06/16
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This file contains shared functions for processing CRXG (*.crx) graphics.

#include "crx-common.h"
#include "zlib.h"

struct CRXHDR {
  unsigned char  signature[4]; // "CRXG"
  unsigned short offset_x;
  unsigned short offset_y;
  unsigned short width;
  unsigned short height;
  unsigned short type;
  unsigned short unknown2;
  unsigned short depth_type;
  unsigned short color_type; // 0 = normal, 
                             // 1 = premultiplied or something?
                             // 2 = delta (no inverted alpha)
};

struct CRXHDR_DELTA {
  unsigned long  unknown1;
  unsigned long  unknown2;
  unsigned short base_width;
  unsigned short base_height;
  unsigned short offset_x;
  unsigned short offset_y;
  unsigned short width;
  unsigned short height;
  unsigned long  data_length;
};

static const unsigned long LINETYPE_DELTA_LEFT    = 0;
static const unsigned long LINETYPE_DELTA_UP      = 1;
static const unsigned long LINETYPE_DELTA_UPLEFT  = 2;
static const unsigned long LINETYPE_DELTA_UPRIGHT = 3;
static const unsigned long LINETYPE_RLE           = 4;

unsigned char* unrle(unsigned char* buff, 
                     unsigned char* out_buff, 
                     unsigned long  out_len)
{
  unsigned char* out_end = out_buff + out_len;

  memset(out_buff, 0xFF, out_len);

  while (out_buff < out_end) {
    unsigned char c = *buff++;
    *out_buff++ = c;

    if (out_buff < out_end && c == *buff) {
      c = *buff++;
      unsigned char n = *buff++;

      while (n--) {
        *out_buff++ = c;
      }
    }
  }

  return buff;
}

void undeltarle(const string&   filename,
                unsigned char*  buff,
                unsigned long   len,
                unsigned char*  out_buff,
                unsigned long   out_len,
                unsigned long   base_width,
                unsigned long   base_height,
                unsigned long   width,
                unsigned long   height,
                unsigned long   offset_x,
                unsigned long   offset_y,
                unsigned long   depth_bytes,
                unsigned short  color_type)
{
  // My alpha handling here may be unnecessarily complicated...
  bool inverse_alpha = color_type != 2;

  unsigned long  out_stride  = (base_width * depth_bytes + 3) & ~3;
  unsigned long  in_stride   = width * depth_bytes;

  unsigned char* red_line    = new unsigned char[width];
  unsigned char* green_line  = new unsigned char[width];
  unsigned char* blue_line   = new unsigned char[width];
  unsigned char* alpha_line  = new unsigned char[width];
  unsigned char* temp_line   = new unsigned char[in_stride];  

  unsigned char* p           = buff;

  for (unsigned long y  = 0; y < height; y++) {
    unsigned char  line_type = *p++;
    unsigned char* src       = NULL;
    unsigned char* dst       = out_buff + (base_height - y - offset_y - 1) * out_stride;

    if (line_type == LINETYPE_RLE) {
      src = temp_line;

      if (depth_bytes > 3) p = unrle(p, alpha_line, width);
      p = unrle(p, red_line, width);
      p = unrle(p, green_line, width);
      p = unrle(p, blue_line, width);

      for (unsigned long x = 0; x < width; x++) {
        unsigned long color_index = 0;

        if (depth_bytes > 3) temp_line[x * depth_bytes + color_index++] = alpha_line[x];
        temp_line[x * depth_bytes + color_index++] = red_line[x];
        temp_line[x * depth_bytes + color_index++] = green_line[x];
        temp_line[x * depth_bytes + color_index++] = blue_line[x];        
      }
    } else {
      src = p;
      p += in_stride;
    }

    for (unsigned long x = 0; x < width; x++) {
      unsigned char* src_pixel   = src + x * depth_bytes;
      unsigned char* dst_pixel   = dst + (x + offset_x) * depth_bytes;
      unsigned char* delta_pixel = NULL;
      unsigned char* prev_dst    = out_buff + (base_height - y - offset_y) * out_stride;

      switch (line_type) {
      case LINETYPE_DELTA_LEFT:
        if (x != 0) delta_pixel = dst + (x + offset_x - 1) * depth_bytes;
        break;

      case LINETYPE_DELTA_UP:
        delta_pixel = prev_dst + (x + offset_x) * depth_bytes;
        break;

      case LINETYPE_DELTA_UPLEFT:
        if (x != 0) delta_pixel = prev_dst + (x + offset_x - 1) * depth_bytes;
        break;

      case LINETYPE_DELTA_UPRIGHT:
        if (x != width - 1) delta_pixel = prev_dst + (x + offset_x + 1) * depth_bytes;
        break;

      case LINETYPE_RLE:
        // rle line already decompressed
        break;

      default:
        fprintf(stderr, "%s: WARNING: ignoring unexpected line type (%d)\n", filename.c_str(), line_type);
      }

      if (depth_bytes > 3) dst_pixel[3] = *src_pixel++;
      dst_pixel[0] = *src_pixel++;
      dst_pixel[1] = *src_pixel++;
      dst_pixel[2] = *src_pixel++;

      if (delta_pixel) {
        dst_pixel[0] += delta_pixel[0];
        dst_pixel[1] += delta_pixel[1];
        dst_pixel[2] += delta_pixel[2];
        if (depth_bytes > 3) dst_pixel[3] += inverse_alpha ? ~delta_pixel[3] : delta_pixel[3];        
      }

      if (inverse_alpha && depth_bytes > 3) dst_pixel[3] = ~dst_pixel[3];
    }
  }

  delete [] temp_line;
  delete [] alpha_line;
  delete [] blue_line;
  delete [] green_line;
  delete [] red_line;
}

bool proc_crxg(const string&  in_filename,
               const string&  out_filename,
               unsigned char* buff,
               unsigned long  len,
               const string&  base_filename)
{
  if (len < 4 || memcmp(buff, "CRXG", 4)) {
    return false;
  }

  CRXHDR* hdr = (CRXHDR*) buff;
  buff += sizeof(*hdr);
  len  -= sizeof(*hdr);

  if (hdr->color_type != 0 && hdr->color_type != 2) {
    printf("%s: warning, unusual color type %d not well supported\n", 
           in_filename.c_str(), 
           hdr->color_type);
  }

  unsigned long  depth_bytes = hdr->depth_type ? 4 : 3;

  // Worst case decompressed length
  unsigned long  out_len  = hdr->width * hdr->height * depth_bytes + hdr->height;
  unsigned char* out_buff = new unsigned char[out_len];

  unsigned long base_width    = hdr->width;
  unsigned long base_height   = hdr->height;
  unsigned long offset_x      = 0;
  unsigned long offset_y      = 0;

  switch (hdr->type) {    
    case 3:
      {
        CRXHDR_DELTA* hdr_delta = (CRXHDR_DELTA*) buff;

        // XXX: Ever more than one delta?  Odd that it repeats the width/height.
        if (hdr_delta->unknown1 != 1) {
          fprintf(stderr, "%s: file might have multiple deltas (tell asmodean)\n", in_filename.c_str());
        }

        base_width  = hdr_delta->base_width;
        base_height = hdr_delta->base_height;
        offset_x    = hdr_delta->offset_x;
        offset_y    = hdr_delta->offset_y;

        // XXX: Is this a data bug?  Seen in grp/BG/EV_SRR19_05.pck and grp/BST/TME_H.crm in D.C.III.
        if (hdr_delta->width  != hdr->width)  offset_x = hdr->offset_x;
        if (hdr_delta->height != hdr->height) offset_y = hdr->offset_y;

        unsigned long  data_len  = hdr_delta->data_length;
        unsigned char* data_buff = buff + sizeof(*hdr_delta);

        // Hmm ... not really sure if this is always right.  Seen in grp/system/yuki01.CRX in D.C.III.
        if (hdr->unknown2 == 1) {
          data_len   = len - sizeof(*hdr_delta);
          data_buff -= 4;
        }

        uncompress(out_buff, &out_len, data_buff, data_len);
      }
      break;

    case 2:  
      uncompress(out_buff, &out_len, buff, len);
      break;

    default:
      memcpy(out_buff, buff, len);
      out_len = len;
  }

  unsigned long orig_base_width  = base_width;
  unsigned long orig_base_height = base_height;

  // I've seen a few that don't fit (/grp/FACE/MOB_AND.crx in D.C.III).
  base_width  = std::max(base_width, offset_x + hdr->width);
  base_height = std::max(base_height, offset_y + hdr->height);
  
  unsigned long  rgb_stride  = (base_width * depth_bytes + 3) & ~3;
  unsigned long  rgb_len     = rgb_stride * base_height;
  unsigned char* rgb_buff    = new unsigned char[rgb_len];
  memset(rgb_buff, 0, rgb_len);

  undeltarle(in_filename,
             out_buff, 
             out_len, 
             rgb_buff, 
             rgb_len, 
             base_width,
             base_height,
             hdr->width, 
             hdr->height,
             offset_x,
             offset_y,
             depth_bytes,
             hdr->color_type);

  if (!base_filename.empty()) {
    unsigned char* delta_buff = rgb_buff;

    as::read_bmp(base_filename,
                 rgb_buff,
                 rgb_len,
                 base_width,
                 base_height,
                 depth_bytes,
                 rgb_stride);

    for (unsigned long i = 0; i < rgb_len; i += depth_bytes) {
      rgb_buff[i + 0] += delta_buff[i + 0];
      rgb_buff[i + 1] += delta_buff[i + 1];
      rgb_buff[i + 2] += delta_buff[i + 2];
      if (depth_bytes > 3) rgb_buff[i + 3] -= delta_buff[i + 3];
    }

    delete delta_buff;
  }

  as::write_bmp(out_filename, 
                rgb_buff, 
                rgb_len, 
                base_width, 
                base_height, 
                depth_bytes);

  delete [] rgb_buff;
  delete [] out_buff;
  
  return true;
}