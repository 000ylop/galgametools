// arkcmp2bmp.cpp, v1.0 2007/07/14
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts Ark's *.cmp images to bitmaps.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstdio>
#include <string>
#include <list>

using std::string;
using std::list;

#pragma pack(1)
struct CMPHDR {
  unsigned short width;
  unsigned short height;
  unsigned char  greyscale_flag;
  unsigned long  frequencies[32];
  unsigned long  length;
};
#pragma pack()

// Basic huffman coding, with each output value being a delta upon the previous.
// The symbol set is only 0-31 (enough to represent individual components of a
// 16-bit color pixel).
class huffdelta_t {
public:
  huffdelta_t(unsigned char* buff, unsigned long len, unsigned long* frequencies) 
    : root(NULL),
      buff(buff),
      len(len),
      saved_bits(0),
      current_bit(0)
  {
    build_tree(frequencies);
  }

  ~huffdelta_t(void) {
    delete root;
  }

  void uncompress(unsigned char* out_buff, unsigned long out_len) {
    unsigned char* out_end    = out_buff + out_len;
    unsigned char  last_value = 0;

    while (out_buff < out_end) {
      node_t* node = root;

      while (node->value == 0xFF) {
        if (get_bit()) {
          node = node->left;
        } else {
          node = node->right;
        }
      }      

      unsigned char value = last_value + node->value;

      if (value >= 0x20) {
        value -= 0x20;
      }

      last_value = *out_buff++ = value;
    }
  }

private:
  struct node_t {
    node_t(unsigned char value,
           unsigned long frequency, 
           node_t*       left  = NULL,
           node_t*       right = NULL) 
      : value(value),
        frequency(frequency),
        left(left),
        right(right)
    {}

    ~node_t(void) {
      delete left;
      delete right;
    }

    unsigned char value;
    unsigned long frequency;
    node_t*       left;
    node_t*       right;
  };

  typedef list<node_t*> node_list_t;

  void build_tree(unsigned long* frequencies) {
    node_list_t nodes;

    for (unsigned char i = 0; i < 32; i++) {
      nodes.push_back(new node_t(i, frequencies[i]));
    }

    // Add a full alphabet even though they're not used
    for (unsigned char i = 32; i < 255; i++) {
      nodes.push_back(new node_t(i, 0));
    }

    while (nodes.size() > 1) {
      node_t* pair[2] = { NULL, NULL };

      for (unsigned long i = 0; i < 2; i++) {
        unsigned long         last_freq = 0xFFFFFFFF;
        node_list_t::iterator last_node = nodes.end();

        for (node_list_t::iterator j = nodes.begin(); j != nodes.end(); j++) {
          if ((*j)->frequency <= last_freq) {
            last_freq    = (*j)->frequency;
            last_node    = j;
          }
        }

        pair[i] = *last_node;
        nodes.erase(last_node);
      }

      nodes.push_back(new node_t(0xFF, pair[0]->frequency + pair[1]->frequency, pair[0], pair[1]));
    }

    root = *nodes.begin();
  }

  bool get_bit(void) {
    if (!current_bit) {
      saved_bits  = *buff++;
      current_bit = 0x80;
    }

    bool rc = (saved_bits & current_bit) != 0;
    current_bit >>= 1;

    return rc;
  }

  node_t*        root;
  unsigned char* buff;
  unsigned long  len;
  unsigned char  saved_bits;
  unsigned char  current_bit;
};

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

string get_file_prefix(const std::string& filename) {
  string temp(filename);

  string::size_type pos = temp.find_last_of(".");
  if (pos != string::npos) {
    temp = temp.substr(0, pos);
  }

  pos = temp.find_last_of("/\\");
  if (pos != string::npos) {
    temp = temp.substr(pos + 1);
  }

  return temp;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "arkcmp2bmp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.char> [output.bmp]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  
  string out_filename = get_file_prefix(in_filename) + ".bmp";

  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  CMPHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = hdr.length;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  unsigned long  out_len  = hdr.width * hdr.height * 3;
  unsigned char* out_buff = new unsigned char[out_len];

  huffdelta_t huffdelta(buff, len, hdr.frequencies);
  huffdelta.uncompress(out_buff, out_len);

  unsigned long  rgb_stride = (hdr.width * 2 + 3) & ~3;
  unsigned long  rgb_len    = hdr.height * rgb_stride;
  unsigned char* rgb_buff   = new unsigned char[rgb_len];

  for (unsigned long y = 0; y < hdr.height; y++) {
    unsigned char*  red_line   = out_buff   + y * hdr.width;
    unsigned char*  green_line = red_line   + hdr.width * hdr.height;
    unsigned char*  blue_line  = green_line + hdr.width * hdr.height;
    unsigned short* rgb_line   = (unsigned short*) (rgb_buff + y * rgb_stride);

    for (unsigned long x = 0; x < hdr.width; x++) {
      rgb_line[x] = blue_line[x];
      rgb_line[x] = (rgb_line[x] << 5) | green_line[x];
      rgb_line[x] = (rgb_line[x] << 5) | red_line[x];
    }
  }

  fd = open_or_die(out_filename,
                   O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
                   S_IREAD | S_IWRITE);

  {
    BITMAPFILEHEADER bmf;
    BITMAPINFOHEADER bmi;

    memset(&bmf, 0, sizeof(bmf));
    memset(&bmi, 0, sizeof(bmi));

    bmf.bfType     = 0x4D42;
    bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + out_len;
    bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi);

    bmi.biSize     = sizeof(bmi);
    bmi.biWidth    = hdr.width;
    bmi.biHeight   = hdr.height;
    bmi.biPlanes   = 1;
    bmi.biBitCount = 16;
   
    write(fd, &bmf, sizeof(bmf));
    write(fd, &bmi, sizeof(bmi));
  }

  write(fd, rgb_buff, rgb_len);
  close(fd);

  delete [] rgb_buff;
  delete [] out_buff;
  delete [] buff;

  return 0;
}
