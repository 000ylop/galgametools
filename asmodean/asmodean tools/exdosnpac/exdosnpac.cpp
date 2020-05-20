// exdosnpac.cpp, v1.03 2012/04/01
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.pac archives used by ÉhSÇ»Ç®éoÇ≥ÇÒÇÕçDÇ´Ç≈Ç∑Ç©ÅH and others.

#include "as-util.h"

#include <list>

#define PAC_VERSION 2

#pragma pack(1)
struct PACHDR {
  unsigned short entry_count;
  unsigned char  filename_length;
  unsigned long  data_base;
};

struct PACENTRY {
#if PAC_VERSION >= 2
  unsigned long long offset;
#else
  unsigned long      offset;
#endif
  unsigned long      length;
};

struct IMGHDR {
  unsigned short type; // 0x0101, 0x0102, 0xA101, 0xA102, 0xA202
  unsigned short width;
  unsigned short height;
  unsigned short depth;
  unsigned short x;
  unsigned short cx;
  unsigned short y;
  unsigned short cy;
  unsigned long  a_length;
  unsigned long  r_length;
  unsigned long  g_length;
  unsigned long  b_length;
};

#pragma pack()

void unhuffman(unsigned char* buff,
               unsigned long  len,
               unsigned char* out_buff,
               unsigned long  out_len)
{
  struct node_t {
    node_t(unsigned char value, unsigned long frequency) 
      : left(NULL),
        right(NULL),
        value(value),
        frequency(frequency)
    {}

    node_t(node_t* left, node_t* right)
      : left(left),
        right(right),
        value(0),
        frequency(left->frequency + right->frequency)
    {}

    ~node_t(void) {
      delete right;
      delete left;
    }

    bool is_leaf(void) const {
      return !left || !right;
    }

    node_t*       right;
    node_t*       left;
    unsigned char value;
    unsigned long frequency;
  };

  typedef std::list<node_t*> nodes_t;
  nodes_t nodes;

  for (unsigned long i = 0; i < 256; i++) {
    unsigned long frequency = *(unsigned long*)buff;
    buff += 4;

    nodes_t::iterator j = nodes.begin();
    for (; j != nodes.end() && (*j)->frequency <= frequency; j++);
    nodes.insert(j, new node_t((char)i, frequency));
  }

  while (nodes.size() > 1) {
    node_t* a = nodes.front();
    nodes.pop_front();

    node_t* b = nodes.front();
    nodes.pop_front();

    node_t* node = new node_t(a, b);

    nodes_t::iterator j = nodes.begin();
    for (; j != nodes.end() && (*j)->frequency <= node->frequency; j++);
    nodes.insert(j, node);
  }

  node_t*        head    = nodes.front();
  unsigned char* out_end = out_buff + out_len;
  unsigned char  bit     = 0;

  while (out_buff < out_end) {
    node_t* node = head;

    while (!node->is_leaf()) {
      if (bit == 8) {
        buff++;
        bit = 0;
      }

      if (*buff & (1 << bit++)) {
        node = node->right;
      } else {
        node = node->left;
      }
    }

    *out_buff++ = node->value;
  }

  delete head;
}

void unrle(unsigned char* buff,
           unsigned long  len,
           unsigned char* out_buff,
           unsigned long  out_len)
{
  unsigned char* end = buff + len;

  while (buff < end) {
    unsigned char n = *buff++;

    if (n & 0x80) {
      n &= 0x7F;

      while (n--) {
        *out_buff++ = *buff;
      }

      buff++;
    } else {
      while (n--) {
        *out_buff++ = *buff++;
      }
    }
  }
}

void unbackref(unsigned char* buff,
               unsigned long  len,
               unsigned char* out_buff,
               unsigned long  out_len)
{
  struct hdr_t {
    unsigned long original_length;
    unsigned long length;
    unsigned long marker;
  };

  hdr_t* hdr = (hdr_t*) buff;
  buff += sizeof(*hdr);

  unsigned char* out_end = out_buff + out_len;

  while (out_buff < out_end) {
    unsigned char c = *buff++;

    if (c == hdr->marker) {
      unsigned char p = *buff++;

      if (p != hdr->marker) {
        unsigned char n = *buff++;

        if (p > hdr->marker) {
          p--;
        }

        while (n--) {
          *out_buff = *(out_buff - p);
          out_buff++;
        }
      } else {
        *out_buff++ = p;
      }
    } else {
      *out_buff++ = c;
    }
  }
}

void process_color(unsigned long  type,
                   unsigned char* buff,
                   unsigned long  len,                   
                   unsigned long  color_len,
                   unsigned char* out_buff,
                   unsigned long  depth_bytes)
{
  unsigned char* color_buff = new unsigned char[color_len];

  switch (type) {
    case 0x0101:
    case 0x0102:
      unrle(buff, len, color_buff, color_len);
      break;

    case 0xA101:
    case 0xA102:
    case 0xA201:
    case 0xA202:
      {
        struct huffhdr_t {
          unsigned long original_length;
          unsigned long length;
        };
        
        huffhdr_t*     hdr       = (huffhdr_t*) buff;
        unsigned long  huff_len  = hdr->length - 8;
        unsigned char* huff_buff = (unsigned char*) (hdr + 1);

        unsigned long  temp_len  = hdr->original_length;
        unsigned char* temp_buff = new unsigned char[temp_len];
        unhuffman(huff_buff, huff_len, temp_buff, temp_len);

        switch (type) {
          case 0xA201:
          case 0xA202:        
            unbackref(temp_buff, temp_len, color_buff, color_len);
            break;

          default:
            unrle(temp_buff, temp_len, color_buff, color_len);
        }

        delete [] temp_buff;
      }
      break;
  }

  for (unsigned long i = 0; i < color_len; i++) {
    out_buff[i * depth_bytes] = color_buff[i];
  }

  delete [] color_buff;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exdosnpac v1.03 by asmodean\n\n");
    fprintf(stderr, "usage: %s <grd.pac>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int toc_fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  int dat_fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PACHDR hdr;
  read(toc_fd, &hdr, sizeof(hdr));

  char* filename = new char[hdr.filename_length + 1];

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    memset(filename, 0, hdr.filename_length + 1);
    read(toc_fd, filename, hdr.filename_length);

    PACENTRY entry;
    read(toc_fd, &entry, sizeof(entry));

    unsigned long  len  = entry.length;
    unsigned char* buff = new unsigned char[len];
    _lseeki64(dat_fd, hdr.data_base + entry.offset, SEEK_SET);
    read(dat_fd, buff, len);

    IMGHDR* imghdr = (IMGHDR*) buff;

    bool sane_type  = imghdr->type == 0x0101 || imghdr->type == 0x0102 || 
                      imghdr->type == 0xA101 || imghdr->type == 0xA102 || 
                      imghdr->type == 0xA201 || imghdr->type == 0xA202;
    bool sane_depth = imghdr->depth == 24 || imghdr->depth == 32;

    if (sane_type && sane_depth) {
      unsigned char* a_buff = (unsigned char*) (imghdr + 1);
      unsigned char* r_buff = a_buff + imghdr->a_length;
      unsigned char* g_buff = r_buff + imghdr->r_length;
      unsigned char* b_buff = g_buff + imghdr->g_length;

      unsigned long width  = imghdr->cx - imghdr->x;
      unsigned long height = imghdr->cy - imghdr->y;

      unsigned long  color_len   = width * height;
      unsigned long  depth_bytes = imghdr->depth / 8;
      unsigned long  out_len     = color_len * depth_bytes;
      unsigned char* out_buff    = new unsigned char[out_len];
      
      process_color(imghdr->type, b_buff, imghdr->b_length, color_len, out_buff + 0, depth_bytes);
      process_color(imghdr->type, g_buff, imghdr->g_length, color_len, out_buff + 1, depth_bytes);
      process_color(imghdr->type, r_buff, imghdr->r_length, color_len, out_buff + 2, depth_bytes);      

      if (imghdr->depth == 32) {
        process_color(imghdr->type, a_buff, imghdr->a_length, color_len, out_buff + 3, depth_bytes);
      }

      string out_filename = filename;

      if (imghdr->x || imghdr->y) {
        out_filename += as::stringf("+x%dy%d", imghdr->x, imghdr->y);
      }

      as::write_bmp(out_filename + ".bmp",
                    out_buff,
                    out_len,
                    width,
                    height,
                    depth_bytes);

      delete [] out_buff;
    } else {
      as::write_file(filename, buff, len);
    }

    delete [] buff;
  }

  delete [] filename;

  close(dat_fd);
  close(toc_fd);    

  return 0;
}
