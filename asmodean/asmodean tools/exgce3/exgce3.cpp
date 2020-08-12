// exgce3.cpp, v1.1 2007/11/09
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from GCEX/GCE3 archives.

#include "as-util.h"

struct GCEXHDR {
  unsigned char signature[4]; // "GXEX"
  unsigned long unknown1;
  unsigned long toc_offset;
  unsigned long unknown2;
};

struct GCE3HDR {
  unsigned char signature[4]; // "GCE3"
  unsigned long unknown1;
  unsigned long length;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long entry_count;
  unsigned long unknown5;
  unsigned long original_length;
  unsigned long unknown6;
};

struct GCE3ENTRY {
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long original_length;
  unsigned long unknown5;
  unsigned long length;
  unsigned long unknown6;
};

struct GCE1HDR {
  unsigned char signature[4]; // "GCE1"
  unsigned long original_length;
  unsigned long unknown2;
  unsigned long data_len2;    // why?
  unsigned long data_len;
  unsigned long cmd_len;
};

struct BGRAHDR {
  unsigned char signature[4];
  unsigned long unknown;
  unsigned long width;
  unsigned long height;
};

class bitbuff_t {
public:
  bitbuff_t(unsigned char* buff, unsigned long len) 
    : buff(buff),
      len(len),
      index(0)
  {}

  bool get_bit(void) {
    if (index > 7) {
      buff++;
      len--;
      index = 0;
    }

    return (*buff << index++) & 0x80;
  }

  // Elias gamma coding with a special case for 0
  unsigned long get_elias_gamma_value(void) {
    unsigned long value  = 0;    

    if (!get_bit()) {
      unsigned long digits = 0;

      while (!get_bit()) digits++;

      value = 1 << digits;

      while (digits--) {
        if (get_bit()) {
          value |= 1 << digits;
        }
      }
    }

    return value;
  }

private:
  unsigned long  index;
  unsigned char* buff;
  unsigned long  len;
};

unsigned long ungce1(unsigned char* buff, 
                     unsigned long  len, 
                     unsigned char* cmd_buff,
                     unsigned long  cmd_len,
                     unsigned char* out_buff,
                     unsigned long  out_len) 
{
  bitbuff_t cmds(cmd_buff, cmd_len);
  
  unsigned char* backref_table[65536] = { 0 };
  unsigned short backref_index        = 0;

  unsigned char* out_end = out_buff + out_len;

  while (out_buff < out_end) {
    unsigned long n = cmds.get_elias_gamma_value();

    while (n--) {
      backref_table[backref_index] = out_buff;
      backref_index                = (backref_index << 8) | *buff;

      *out_buff++ = *buff++;
    }

    if (out_buff >= out_end) {
      break;
    }

    n = cmds.get_elias_gamma_value() + 1;

    unsigned char* src = backref_table[backref_index];

    while (n--) {
      backref_table[backref_index] = out_buff;
      backref_index                = (backref_index << 8) | *src;

      *out_buff++ = *src++;
    }
  }

  return out_len - (out_end - out_buff);
}

void read_uncompress(int fd, unsigned long len, unsigned char*& out_buff, unsigned long& out_len) {
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);  

  if (!memcmp(buff, "GCE1", 4)) {
    out_buff = new unsigned char[out_len];

    unsigned char* p     = buff;
    unsigned char* end   = buff + len;

    unsigned char* out_p = out_buff;

    while (p < end) {
      GCE1HDR* hdr = (GCE1HDR*) p;
      p += sizeof(*hdr);

      unsigned char* data_buff = p;
      p += hdr->data_len;

      unsigned char* cmd_buff = p;
      p += hdr->cmd_len;

      ungce1(data_buff, hdr->data_len, cmd_buff, hdr->cmd_len, out_p, hdr->original_length);
      out_p += hdr->original_length;
    }

    delete [] buff;
  } else {
    out_len  = len;
    out_buff = buff;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exgce3 v1.1 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  GCEXHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  GCE3HDR tochdr;
  lseek(fd, hdr.toc_offset, SEEK_SET);
  read(fd, &tochdr, sizeof(tochdr));

  unsigned long  toc_len   = tochdr.original_length;
  unsigned char* toc_buff  = NULL;
  read_uncompress(fd, tochdr.length - sizeof(tochdr), toc_buff, toc_len);

  GCE3ENTRY*     entries   = (GCE3ENTRY*) toc_buff;
  unsigned char* filenames = toc_buff + sizeof(GCE3ENTRY) * tochdr.entry_count;

  // For some reason the TOC entry has no recognizable offsets, so we'll just hope
  // they're in sequential order...
  lseek(fd, sizeof(hdr), SEEK_SET);

  for (unsigned long i = 0; i < tochdr.entry_count; i++) {
    unsigned short filename_len = *(unsigned short*) filenames;
    filenames += sizeof(filename_len);

    string filename((char*)filenames, filename_len);
    filenames += filename_len;   

    if (entries[i].length == 0) {
      as::make_path(filename);
    } else {
      unsigned long  len  = entries[i].original_length;
      unsigned char* buff = NULL;
      read_uncompress(fd, entries[i].length, buff, len); 

      if (len >= 4 && !memcmp(buff, "BGRA", 4)) {
        BGRAHDR* bgrahdr = (BGRAHDR*) buff;

        as::write_bmp(as::get_file_prefix(filename) + ".bmp",
                      buff + sizeof(*bgrahdr),
                      len  - sizeof(*bgrahdr),
                      bgrahdr->width,
                      bgrahdr->height,
                      4,
                      true);
      } else {
        int out_fd = as::open_or_die(filename, 
                                     O_CREAT | O_WRONLY | O_BINARY, 
                                     S_IREAD | S_IWRITE);
        write(out_fd, buff, len);
        close(out_fd);
      }

      delete [] buff;
    }
  }

  delete [] toc_buff;

  close(fd);     

  return 0;
}
