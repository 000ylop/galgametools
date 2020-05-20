// exlfa.cpp, v1.0 2009/12/20
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts Leaf's *.a archives.

#include "as-util.h"
#include "as-lzss.h"
#include "mt.h"

struct AHDR {
  unsigned char  signature[2]; // 1EAF
  unsigned short entry_count;
};

struct AENTRY {
  char           filename[22];
  unsigned short flags;
  unsigned long  length;
  unsigned long  offset;
};

static const unsigned long AENTRY_FLAG_COMPRESSED = 0x00000100;

void unobfuscate(AENTRY entry, unsigned char* buff, unsigned long len) {
  unsigned long seed = strlen(entry.filename);

  for (unsigned long i = 1; true; i++) {
    seed = _rotl(seed, 2) ^ entry.filename[i];

    if (!entry.filename[i]) break;
  }

  mt_sgenrand(seed);  

  for (unsigned long i = 0; i < len; i++) {
    unsigned char mutator = mt_genrand() % 0xFF;    
    buff[i] ^= mutator;
  }
}

struct PXHDR {
  unsigned short file_count;
  unsigned short unknown1;
  unsigned long  block_size;
  unsigned long  unknown2;
  unsigned long  unknown3;
  unsigned short unknown4;    // 0C00
  unsigned short depth;       // ??
  unsigned short width;
  unsigned short height;
  unsigned long  unknown5;

  // I've seen some bogus values (effect_h.px) so maybe these are
  // chars with random data padding?
  //unsigned short width_blocks;
  //unsigned short height_blocks;
  unsigned char width_blocks;
  unsigned char pad1;
  unsigned char height_blocks;
  unsigned char pad2;
};

struct PXENTRY {
  unsigned short offset;
};

struct PXBLOCKHDR {
  unsigned char width;
  unsigned char height;
};

struct PXHDR2 {
  unsigned long  unknown1;
  unsigned long  file_count;
  unsigned long  unknown2;
  unsigned long  unknown3;
  unsigned short unknown4;
  unsigned short unknown5;
  unsigned char  signature[12]; // "LeafAQUAPLUS"
};

struct PXENTRY2 {
  unsigned long  width;
  unsigned long  height;
  unsigned long  unknown1;
  unsigned long  unknown2;
  unsigned short unknown3;
  unsigned short depth;
  unsigned long  width2;
  unsigned long  height2;
  unsigned long  unknown4;
};

bool process_px2(const string&  filename,
                 unsigned char* buff,
                 unsigned long  len)
{
  PXHDR2* hdr = (PXHDR2*) buff;
  buff += sizeof(*hdr);

  if (as::stringtol(filename).find(".px") == string::npos || 
      len < sizeof(*hdr) || memcmp(hdr->signature, "LeafAQUAPLUS", 12)) {
    return false;
  }

  for (unsigned long i = 0; i < hdr->file_count; i++) {
    string out_filename;

    if (hdr->file_count > 1) {
      out_filename = as::get_file_prefix(filename) + as::stringf("+%03d.bmp", i);
    } else {
      out_filename = as::get_file_prefix(filename) + ".bmp";
    }

    PXENTRY2* entry = (PXENTRY2*) buff;
    buff += sizeof(*entry);

    unsigned long len = entry->width * entry->height * entry->depth / 8;

    as::write_bmp(out_filename,
                  buff,
                  len,
                  entry->width,
                  entry->height,
                  entry->depth / 8,
                  as::WRITE_BMP_FLIP);

    buff += len;
  }

  return true;
}

bool process_px(const string&  filename,
                unsigned char* buff,
                unsigned long  len)
{
  PXHDR* hdr = (PXHDR*) buff;

  if (as::stringtol(filename).find(".px") == string::npos || 
      hdr->unknown4 != 0xC || hdr->depth != 32) {
    return false;
  }

  unsigned long entry_count = hdr->width_blocks * hdr->height_blocks * hdr->file_count;

  if (hdr->unknown1) {
    entry_count += hdr->file_count;
  }

  PXENTRY*       entry       = (PXENTRY*) (hdr + 1);  
  unsigned char* data        = buff + sizeof(*hdr) + sizeof(PXENTRY) * entry_count;
  
  unsigned long  out_width  = hdr->width;
  unsigned long  out_height = hdr->height;
  unsigned long  out_depth  = hdr->depth / 8;
  unsigned long  out_stride = out_width * out_depth;
  unsigned long  out_len    = out_height * out_stride;
  unsigned char* out_buff   = new unsigned char[out_len];  

  unsigned long block_width  = hdr->block_size;
  unsigned long block_height = hdr->block_size;
  unsigned long block_pad    = 2;

  unsigned long block_len    = sizeof(PXBLOCKHDR) + (block_width + block_pad) * (block_height + block_pad) * 4;

  for (unsigned long i = 0; i < hdr->file_count; i++) {
    string out_filename;

    if (hdr->file_count > 1) {
      out_filename = as::get_file_prefix(filename) + as::stringf("+%03d.bmp", i);
    } else {
      out_filename = as::get_file_prefix(filename) + ".bmp";
    }

    memset(out_buff, 0, out_len);

    if (hdr->unknown1) {
      entry++;
    }

    for (unsigned long y = 0; y < hdr->height_blocks; y++) {
      as::RGBA* dst_block = (as::RGBA*) (out_buff + y * block_height * out_stride);

      for (unsigned long x = 0; x < hdr->width_blocks; x++) {      
        as::RGBA* dst_line = &dst_block[x * block_width];

        if (entry->offset) {
          PXBLOCKHDR* blockhdr = (PXBLOCKHDR*) (data + (entry->offset - 1) * block_len);
          as::RGBA*   src_line = (as::RGBA*) (blockhdr + 1);

          for (unsigned long yy = 0; yy + block_pad < blockhdr->height; yy++) {
            for (unsigned long xx = 0; xx + block_pad < blockhdr->width; xx++) {
              dst_line[xx] = src_line[xx];
            }

            dst_line += out_width;
            src_line += blockhdr->width;
          }
        }

        entry++;
      }
    }

    as::write_bmp(out_filename,
                  out_buff,
                  out_len,
                  out_width,
                  out_height,
                  out_depth,
                  as::WRITE_BMP_FLIP);
  }

  delete [] out_buff;

  return true;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exlfpa v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.a>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  AHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  if (memcmp(hdr.signature, "\x1E\xAF", 2)) {
    fprintf(stderr, "%s: not supported\n", in_filename.c_str());
    return 0;
  }

  AENTRY* entries = new AENTRY[hdr.entry_count];
  read(fd, entries, sizeof(AENTRY) * hdr.entry_count);

  unsigned long data_base = tell(fd);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, data_base + entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    string filename = entries[i].filename;

    if (entries[i].flags & AENTRY_FLAG_COMPRESSED) {
      unsigned long  out_len  = *(unsigned long*) buff;
      unsigned char* out_buff = new unsigned char[out_len];
      as::unlzss(buff + 4, out_len - 4, out_buff, out_len);

      delete [] buff;

      len  = out_len;
      buff = out_buff;
    }

    if (as::stringtol(filename).find(".fat") != string::npos) {
      filename = as::get_file_prefix(entries[i].filename);
      unobfuscate(entries[i], buff, len);
    }    

    if (!process_px2(filename, buff, len) && !process_px(filename, buff, len)) {
      as::write_file(filename, buff, len);
    }

    delete [] buff;   
  }

  close(fd);

  return 0;
}
