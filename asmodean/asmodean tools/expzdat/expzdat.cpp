// expzdat.cpp, v1.0 2007/07/27
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts PlumZERO's *.HED+*.DAT archives.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstdio>
#include <string>

using std::string;

#pragma pack(1)
struct HEDENTRY1 {
  unsigned char filename_length;
};

struct HEDENTRY2 {
  unsigned long offset;
};

static const unsigned char GRAHDR_TYPE_UNKNOWN   = 1;
static const unsigned char GRAHDR_TYPE_PALETTE   = 2;
static const unsigned char GRAHDR_TYPE_TRUECOLOR = 3;

struct GRAHDR {
  unsigned char  has_alpha;
  unsigned char  type;
  unsigned short width;
  unsigned short height;
};

struct GRAHDR2 {
  unsigned long length;
};

struct ALPHDR {
  unsigned long width;
  unsigned long height;
};
#pragma pack()

static const unsigned long LZ78_DICT_SIZE = 65536;

// LZ78 with some disfunctional huffman-like coding which doubles the bits
// needed to represent each literal...
class lz78wtfman_t {
public:
  lz78wtfman_t(unsigned char* buff, unsigned long len, unsigned char* symbol_table) 
    : buff(buff),
      len(len),
      symbol_table(symbol_table),
      saved_bits(0),
      current_bit(0),
      dict_index(0)
  {
    memset(dict, 0, sizeof(dict));
  }

  unsigned long uncompress(unsigned char* out_buff, unsigned long out_len) {
    unsigned char* end     = buff + len;
    unsigned char* out_end = out_buff + out_len;

    while (buff < end && out_buff < out_end) {
      unsigned long index = -1;
      dict_entry_t  entry = { 0 };

      if (get_bit()) {
        unsigned long need_bits = dict_index == LZ78_DICT_SIZE ? dict_index - 1 : dict_index;

        index = get_bit();
        for (unsigned long i = 0; need_bits > 1; i++) {
          index |= get_bit() << (i + 1);
          need_bits /= 2;
        }

        entry = dict[index];

        // Follow chain up to a root node and then copy it out backwards
        {
          unsigned char temp[LZ78_DICT_SIZE];
          unsigned long temp_len   = 0;
          unsigned long curr_index = index;

          do {
            temp[temp_len++] = dict[curr_index].c;
            curr_index       = dict[curr_index].parent;
          } while (curr_index != -1);

          while (temp_len--) {
            *out_buff++ = temp[temp_len];
          }
        }
      }

      unsigned long expect_bits = 2;
      while (get_bit()) {
        expect_bits++;
      }

      unsigned char c = 0;
      while (expect_bits--) {
        c |= get_bit() << expect_bits;
      }

      unsigned char symbol = symbol_table[c];

      *out_buff++ = symbol;

      if (dict_index < LZ78_DICT_SIZE) {
        dict[dict_index].parent = index;
        dict[dict_index].c      = symbol;
        dict_index++;
      }      
    }

    return out_len - (out_end - out_buff);
  }

private:
  inline bool get_bit(void) {
    if (!current_bit) {
      saved_bits  = *buff++;
      current_bit = 1;
    }

    bool rc = (saved_bits & current_bit) != 0;
    current_bit <<= 1;

    return rc;
  }

  struct dict_entry_t {
    unsigned long parent;
    unsigned char c;
  };

  dict_entry_t  dict[LZ78_DICT_SIZE];
  unsigned long dict_index;

  unsigned char* buff;
  unsigned long  len;
  unsigned char* symbol_table;
  unsigned char  saved_bits;
  unsigned char  current_bit;
};

// Paeth's predictor with PNG-style tie resolution
inline unsigned char paeth_predictor(long left, long prev, long prev_left) {
  long p  = left + prev - prev_left;
  long pa = abs(p - left);
  long pb = abs(p - prev);
  long pc = abs(p - prev_left);

  if (pa <= pb && pa <= pc) {
    return (unsigned char) left;
  }

  if (pb <= pc) {
    return (unsigned char) prev;
  }

  return (unsigned char) prev_left;
}

void undeltafilter(unsigned char* buff, unsigned long width, unsigned long height) {
  for (unsigned long x = 1; x < width; x++) {
    buff[x] += buff[x - 1];
  }

  for (unsigned long y = 1; y < height; y++) {
    unsigned char* line      = buff + y * width;
    unsigned char* prev_line = buff + (y - 1) * width;

    line[0] += prev_line[0];

    for (unsigned long x = 1; x < width; x++) {
      line[x] += paeth_predictor(line[x - 1], prev_line[x], prev_line[x - 1]);
    }
  }
}

unsigned long unrle(unsigned char* buff, 
                    unsigned long  len,
                    unsigned char* out_buff, 
                    unsigned long  out_len) 
{
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;

  while (buff < end && out_buff < out_end) {
    unsigned char c = *buff++;
    unsigned char n = *buff++;

    while (n-- && out_buff < out_end) {
      *out_buff++ = c;
    }
  }

  return out_len - (out_end - out_buff);
}

void unobfuscate(char* buff, unsigned long len) { 
  for (unsigned long i = 0; i < len; i++) {
    buff[i] ^= 0xFF;
  }
}

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

unsigned long get_file_size(int fd) {
  struct stat file_stat;
  fstat(fd, &file_stat);
  return file_stat.st_size;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "expzdat v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.hed> <inut.dat>\n", argv[0]);
    return -1;
  }

  string hed_filename(argv[1]);  
  string dat_filename(argv[2]);  

  int fd = open_or_die(hed_filename, O_RDONLY | O_BINARY);

  unsigned long  toc_len  = get_file_size(fd);
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);
  close(fd);

  fd = open_or_die(dat_filename, O_RDONLY | O_BINARY);

  unsigned long dat_length = get_file_size(fd);

  unsigned char* p   = toc_buff;
  unsigned char* end = toc_buff + toc_len;
  
  while (p < end) {
    HEDENTRY1* entry1 = (HEDENTRY1*) p;
    p += sizeof(*entry1);

    char filename[4096] = { 0 };
    memcpy(filename, p, entry1->filename_length);
    p += entry1->filename_length;    
    unobfuscate(filename, entry1->filename_length);
    
    HEDENTRY2* entry2 = (HEDENTRY2*) p;
    p += sizeof(*entry2);

    // Annoying sloppy code needed to figure out complete length for the alpha data
    unsigned long total_len = 0;
    if (p < end) {
      HEDENTRY1* next_entry1 = (HEDENTRY1*) p;
      HEDENTRY2* next_entry2 = (HEDENTRY2*) (p + sizeof(*next_entry1) + next_entry1->filename_length);

      total_len = next_entry2->offset - entry2->offset;
    } else {
      total_len = dat_length - entry2->offset;
    }

    GRAHDR grahdr;
    lseek(fd, entry2->offset, SEEK_SET);
    read(fd, &grahdr, sizeof(grahdr));

    if (grahdr.type == GRAHDR_TYPE_UNKNOWN) {
      fprintf(stderr, "%s: don't support type 1 compression (lazy)\n", filename);
      continue;
    }

    // There may be palette data...
    unsigned char  pal[256 * 3] = { 0 };
    unsigned short pal_entries  = 0;

    if (grahdr.type == GRAHDR_TYPE_PALETTE) {
      read(fd, &pal_entries, sizeof(pal_entries));
      read(fd, pal, pal_entries * 3);
    }

    // Truecolor provides a symbol table, otherwise use a naive fixed one
    unsigned char symbol_table[256];
    for (unsigned long i = 0; i <= 255; i++) {
      symbol_table[i] = (unsigned char) i;
    }

    if (grahdr.type == GRAHDR_TYPE_TRUECOLOR) {
      read(fd, symbol_table, sizeof(symbol_table));
    }

    GRAHDR2 grahdr2;
    read(fd, &grahdr2, sizeof(grahdr2));

    unsigned long  len  = grahdr2.length;
    unsigned char* buff = new unsigned char[len];
    read(fd, buff, len);

    unsigned long  depth    = 24;
    unsigned long  out_len  = grahdr.width * grahdr.height * depth / 8;
    unsigned char* out_buff = new unsigned char[out_len];

    lz78wtfman_t lz78wtfman(buff, len, symbol_table);
    lz78wtfman.uncompress(out_buff, out_len);

    ALPHDR         alphdr         = { 0 };
    unsigned long  alpha_len      = 0;
    unsigned char* alpha_buff     = NULL;
    unsigned long  out_alpha_len  = 0;
    unsigned char* out_alpha_buff = NULL;
    
    if (grahdr.has_alpha) {      
      depth      = 32;
      alpha_len  = total_len - grahdr2.length - sizeof(alphdr);
      alpha_buff = new unsigned char[alpha_len];

      read(fd, &alphdr, sizeof(alphdr));
      read(fd, alpha_buff, alpha_len);

      out_alpha_len  = grahdr.width * grahdr.height;
      out_alpha_buff = new unsigned char[out_alpha_len];
      unrle(alpha_buff, alpha_len, out_alpha_buff, out_alpha_len);
    }

    unsigned char* red_buff   = out_buff;
    unsigned char* green_buff = red_buff   + grahdr.width * grahdr.height;
    unsigned char* blue_buff  = green_buff + grahdr.width * grahdr.height;    

    if (grahdr.type == GRAHDR_TYPE_TRUECOLOR) {
      undeltafilter(red_buff, grahdr.width, grahdr.height);
      undeltafilter(green_buff, grahdr.width, grahdr.height);
      undeltafilter(blue_buff, grahdr.width, grahdr.height);
    }

    unsigned long  pixel_bytes = depth / 8;
    unsigned long  rgb_stride  = (grahdr.width * pixel_bytes + 3) & ~3;
    unsigned long  rgb_len     = grahdr.height * rgb_stride;
    unsigned char* rgb_buff    = new unsigned char[rgb_len];

    for (unsigned long y = 0; y < grahdr.height; y++) {
      unsigned char* red_line   = red_buff       + y * grahdr.width;
      unsigned char* green_line = green_buff     + y * grahdr.width;
      unsigned char* blue_line  = blue_buff      + y * grahdr.width;
      unsigned char* alpha_line = out_alpha_buff + y * grahdr.width;
      unsigned char* rgb_line   = rgb_buff       + (grahdr.height - y - 1) * rgb_stride;

      for (unsigned long x = 0; x < grahdr.width; x++) {        
        // Convert everything to 24-bit since even the palette images may have
        // alpha and I'm lazy to convert them differently.
        if (grahdr.type == GRAHDR_TYPE_TRUECOLOR) {
          rgb_line[x * pixel_bytes + 0] = red_line[x];
          rgb_line[x * pixel_bytes + 1] = green_line[x];
          rgb_line[x * pixel_bytes + 2] = blue_line[x];
        } else {
          rgb_line[x * pixel_bytes + 0] = pal[red_line[x] * 3 + 0];
          rgb_line[x * pixel_bytes + 1] = pal[red_line[x] * 3 + 1];
          rgb_line[x * pixel_bytes + 2] = pal[red_line[x] * 3 + 2];
        }

        if (grahdr.has_alpha) {
          rgb_line[x * pixel_bytes + 3] = (unsigned char) (alpha_line[x] * 2.55);
        }
      }
    }   

    strcat(filename, ".bmp");

    int out_fd = open_or_die(filename,
                             O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
                             S_IREAD | S_IWRITE);

    {
      BITMAPFILEHEADER bmf;
      BITMAPINFOHEADER bmi;

      memset(&bmf, 0, sizeof(bmf));
      memset(&bmi, 0, sizeof(bmi));

      bmf.bfType     = 0x4D42;
      bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + rgb_len;
      bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi);

      bmi.biSize     = sizeof(bmi);
      bmi.biWidth    = grahdr.width;
      bmi.biHeight   = grahdr.height;
      bmi.biPlanes   = 1;
      bmi.biBitCount = (WORD) depth;
     
      write(out_fd, &bmf, sizeof(bmf));
      write(out_fd, &bmi, sizeof(bmi));
    }

    write(out_fd, rgb_buff, rgb_len);
    close(out_fd);

    delete [] out_alpha_buff;
    delete [] alpha_buff;
    delete [] rgb_buff;
    delete [] out_buff;
    delete [] buff;
  }

  close(fd);

  delete [] toc_buff;

  return 0;
}
