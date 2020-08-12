// exsec.cpp, v1.2 2010/11/24
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts secB (*.sec) archives.

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <Xgraphics.h>
#include "as-util.h"

// Not useful
//#define EXTRACT_CL2

struct SECHDR {
  unsigned char signature[4]; // "secB"
  unsigned long unknown1;
  unsigned long block_size;
  unsigned long filenames_offset;
  unsigned long data_offset;
  unsigned long unknown2;
  unsigned long section_count;
  unsigned long total_entry_count;  

  void flip_endian(void) {
    block_size       = as::flip_endian(block_size);
    filenames_offset = as::flip_endian(filenames_offset);
    data_offset      = as::flip_endian(data_offset); 
    section_count    = as::flip_endian(section_count);
  }
};

struct SECENTRYHDR {
  unsigned long entry_count;

  void flip_endian(void) {
    entry_count = as::flip_endian(entry_count);
  }
};

struct SECENTRY {
  unsigned long index;
  unsigned long type;
  unsigned long filename_offset;
  unsigned long offset; // in blocks
  unsigned long unknown1;
  unsigned long original_length;
  unsigned long length;
  unsigned long unknown2;

  void flip_endian(void) {
    index           = as::flip_endian(index);
    type            = as::flip_endian(type);
    filename_offset = as::flip_endian(filename_offset);
    offset          = as::flip_endian(offset);
    unknown1        = as::flip_endian(unknown1);
    original_length = as::flip_endian(original_length);
    length          = as::flip_endian(length);     
  }
};

static const unsigned long SECENTRY_TYPE_DIRECTORY   = 0x001;
static const unsigned long SECENTRY_TYPE_COMPRESSED1 = 0x500;
static const unsigned long SECENTRY_TYPE_COMPRESSED2 = 0x100;

struct CLC2HDR {
  unsigned char signature[4]; // "CLC2"
  unsigned long entry_count;
};

struct CLC2ENTRY {
  unsigned long index;
  unsigned long offset;
  unsigned long length;
};

struct CL3BHDR1 {
  unsigned char signature[4]; // "CL3B"
  unsigned long unknown1;     // 0
  unsigned long unknown2;     // 1
  unsigned long unknown3;     // 1
  unsigned long offset_next;  // 64

  void flip_endian(void) {
    offset_next = as::flip_endian(offset_next);
  }
};

struct CL3BHDR2 {
  unsigned char signature[32]; // "FILE_COLLECTION"
  unsigned long entry_count;
  unsigned long length;
  unsigned long offset_next;   // 192

  void flip_endian(void) {
    entry_count = as::flip_endian(entry_count);
    offset_next = as::flip_endian(offset_next);
  }
};

struct CL3BENTRY {
  char          filename[512];
  unsigned long index;
  unsigned long offset; // from first entry
  unsigned long length;
  unsigned long unknown1[9];

  void flip_endian(void) {
    offset = as::flip_endian(offset);
    length = as::flip_endian(length);
  }
};

struct MPDATAHDR {
  unsigned char signature[32]; // "MP_ONE_DATA Ver1.02"
  unsigned long length;
  unsigned long entry_count;

  void flip_endian(void) {
    entry_count = as::flip_endian(entry_count);
  }
};

struct MPDATAENTRY {
  unsigned long index;
  char          filename[256];
  unsigned long offset;
  unsigned long original_length;
  unsigned long length;

  void flip_endian(void) {
    offset          = as::flip_endian(offset);
    original_length = as::flip_endian(original_length);
    length          = as::flip_endian(length);
  }
};

struct TIDHDR {
  unsigned char signature[3]; // "TID\x13"
  unsigned char type; // 0x13, 0x9F etc
  unsigned long length;
  unsigned long header_length;
  unsigned long unknown1;
  char          filename[32];
  unsigned long unknown2;
  unsigned long width;
  unsigned long height;
  unsigned long depth;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long data_length;
  unsigned long data_offset;
  unsigned long unknown5[12];

  void flip_endian(void) {
    length        = as::flip_endian(length);
    header_length = as::flip_endian(header_length);
    unknown1      = as::flip_endian(unknown1);
    width         = as::flip_endian(width);
    height        = as::flip_endian(height);
    depth         = as::flip_endian(depth);
    data_length   = as::flip_endian(data_length);
    data_offset   = as::flip_endian(data_offset);
  }
};

class bitbuff_t {
public:
  bitbuff_t(unsigned char* buff, unsigned long len)
    : buff(buff), len(len), bit(0)
  {}

  unsigned long get_bits(unsigned long n) {
    unsigned long val = 0;

    while (n--) {
      val <<= 1;

      if (bit == 8) {
        buff++;
        bit = 0;
      }

      val |= (*buff >> bit) & 1;
      bit++;
    }

    return val;
  }

  unsigned long get_variable(void) {
    unsigned long val   = 0;
    unsigned long count = get_bits(4);

    if (count) {
      count--;

      val = get_bits(count);
      val |= (1 << count);
    }

    return val;
  }

private:
  unsigned char* buff;
  unsigned long  len;
  unsigned long  bit;
};

void uncompress1(unsigned char* buff,
                unsigned long  len,
                unsigned char* out_buff,
                unsigned long  out_len)
{
  bitbuff_t bits(buff, len);

  unsigned char* out_end = out_buff + out_len;

  while (out_buff < out_end) {
    if (bits.get_bits(1)) {
      unsigned long p = bits.get_variable();
      unsigned long n = bits.get_variable() + 3;      

      while (n--) {
        *out_buff = *(out_buff - p);
        out_buff++;
      }      
    } else {
      unsigned long n = bits.get_variable();

      while (n--) {
        *out_buff++ = (unsigned char) bits.get_bits(8);
      }
    }
  }
}

void uncompress2(unsigned char* buff,
                unsigned long  len,
                unsigned char* out_buff,
                unsigned long  out_len)
{
  unsigned char* out_end = out_buff + out_len;

  while (out_buff < out_end) {
    unsigned char n = *buff++;

    if (n & 0x80) {
      n <<= 1;

      while (n-- && out_buff < out_end) {
        *out_buff++ = *buff;
      }

      *buff++;
    } else {
      while (n-- && out_buff < out_end) {
        *out_buff++ = *buff++;
      }
    }
  }
}

void process_tid(const string&  filename, 
                 unsigned char* buff, 
                 unsigned long  len,
                 bool           flip)
{
  TIDHDR hdr = *(TIDHDR*) buff;
  hdr.flip_endian();

  unsigned long majortype = hdr.type >> 4;
  unsigned long subtype = hdr.type & 0xF;

  unsigned char* pal_buff  = buff + sizeof(hdr);
  unsigned long  data_len  = hdr.data_length;
  unsigned char* data_buff = buff + hdr.data_offset;

#ifdef EXTRACT_CL2
  string out_filename = filename + "+" + as::get_file_prefix(hdr.filename) + ".bmp";
#else
  string out_filename = filename + ".bmp";
#endif

  switch (majortype) {
    case 1:
      if (hdr.data_offset + hdr.data_length > len) {
        as::write_file(filename + ".tid", buff, len);
      } else {
        unsigned long options = as::WRITE_BMP_BIGENDIAN;

        if (flip) options |= as::WRITE_BMP_FLIP;

        as::write_bmp_ex(out_filename,
                         data_buff,
                         data_len,
                         hdr.width,
                         hdr.height,
                         hdr.depth / 8,
                         256,
                         pal_buff,
                         options);
      }
      break;

    case 3:
      {
        unsigned long  out_stride = hdr.width * 4;
        unsigned long  out_len    = hdr.height * out_stride;
        unsigned char* out_buff   = new unsigned char[out_len];

        XGPTCDecompressSurface(out_buff,
                               out_stride,
                               hdr.width,
                               hdr.height,
                               D3DFMT_LIN_A8R8G8B8,
                               NULL,
                               data_buff,
                               data_len);

        as::write_bmp(out_filename,
                      out_buff,
                      out_len,
                      hdr.width,
                      hdr.height,
                      4,
                      as::WRITE_BMP_FLIP | as::WRITE_BMP_BIGENDIAN);

        delete [] out_buff;
      }
      break;

    // Fixme: MCT but I don't feel like initializing a d3d device to deal with textures functions
    case 9:
    default:
      as::write_file(filename, buff, len);
  }
}

void process_data(const string&  filename,
                  unsigned char* buff, 
                  unsigned long len,
                  bool          flip_tid = true);

void process_clc2(const string&  filename, 
                  unsigned char* buff, 
                  unsigned long  len)
{
  CLC2HDR*   hdr     = (CLC2HDR*) buff;
  CLC2ENTRY* entries = (CLC2ENTRY*) (hdr + 1);

  for (unsigned long i = 0; i < hdr->entry_count; i++) {
    process_data(filename + as::stringf("+%05d", i),
                 buff + entries[i].offset,
                 entries[i].length);
  }
}

void process_cl3b(const string&  filename, 
                  unsigned char* buff, 
                  unsigned long  len)
{
  CL3BHDR1*  hdr1    = (CL3BHDR1*) buff;
  hdr1->flip_endian();

  CL3BHDR2*  hdr2    = (CL3BHDR2*) (buff + hdr1->offset_next);
  hdr2->flip_endian();

  CL3BENTRY* entries = (CL3BENTRY*) (buff + hdr2->offset_next);

  unsigned char* data = buff + hdr2->offset_next;

  for (unsigned long i = 0; i < hdr2->entry_count; i++) {
    entries[i].flip_endian();

    process_data(filename + as::stringf("+%05d+%s", i, entries[i].filename),
                 data + entries[i].offset,
                 entries[i].length);
  }
}

void process_mpdata(const string&  filename, 
                    unsigned char* buff, 
                    unsigned long  len)
{
  MPDATAHDR*  hdr = (MPDATAHDR*) buff;
  hdr->flip_endian();

  MPDATAENTRY* entries = (MPDATAENTRY*) (hdr + 1);

  for (unsigned long i = 0; i < hdr->entry_count; i++) {
    entries[i].flip_endian();

    process_data(filename + as::stringf("+%05d+%s", i, entries[i].filename),
                 buff + entries[i].offset,
                 entries[i].length,
                 false);
  }
}

void process_data(const string&  filename,
                  unsigned char* buff, 
                  unsigned long  len,
                  bool           flip_tid)
{
  string prefix = as::get_file_prefix(filename);

  if (len >= 3 && !memcmp(buff, "TID", 3)) {
    process_tid(prefix, buff, len, flip_tid);
#ifdef EXTRACT_CL2
  } else if (len >= 4 && !memcmp(buff, "CLC2", 4)) {
    process_clc2(prefix + "_CLC2", buff, len);
#endif
  } else if (len >= 4 && !memcmp(buff, "CL3B", 4)) {
    process_cl3b(prefix + "_CL3B", buff, len);
  } else if (len >= 11 && !memcmp(buff, "MP_ONE_DATA", 11)) {
    process_mpdata(prefix + "_MpData", buff, len);
  } else {
    as::write_file(filename, buff, len);
  }
}

void process_dir(int            toc_fd, 
                 int            fd,
                 const SECHDR&  hdr,
                 char*          filenames_buff,
                 const string&  prefix)
{
  SECENTRYHDR entryhdr;
  read(toc_fd, &entryhdr, sizeof(entryhdr));
  entryhdr.flip_endian();

  SECENTRY* entries = new SECENTRY[entryhdr.entry_count];
  read(toc_fd, entries, sizeof(SECENTRY) * entryhdr.entry_count);

  for (unsigned long j = 0; j < entryhdr.entry_count; j++) {
    entries[j].flip_endian();

    string out_filename = prefix + as::stringf("+%05d", j);
    if (entries[j].filename_offset) out_filename += as::stringf("+%s", filenames_buff + entries[j].filename_offset);

    if (entries[j].type == SECENTRY_TYPE_DIRECTORY) {
      process_dir(toc_fd, fd, hdr, filenames_buff, out_filename + "_DIR");
      continue;
    }

    if (entries[j].original_length == 0) {
      continue;
    }

    bool do_decompress = entries[j].type == SECENTRY_TYPE_COMPRESSED1 ||
                         entries[j].type == SECENTRY_TYPE_COMPRESSED2;

    unsigned long  len  = entries[j].original_length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[j].offset * hdr.block_size, SEEK_SET);

    if (do_decompress) {
      unsigned long  temp_len  = entries[j].length;
      unsigned char* temp_buff = new unsigned char[len];      
      read(fd, temp_buff, temp_len);

      switch (entries[j].type) {
        case SECENTRY_TYPE_COMPRESSED1:
          uncompress1(temp_buff, temp_len, buff, len);
          break;

        case SECENTRY_TYPE_COMPRESSED2:
          uncompress2(temp_buff, temp_len, buff, len);
          break;
      }

      delete [] temp_buff;
    } else {
      read(fd, buff, len);
    }

    process_data(out_filename, buff, len);

    delete [] buff;
  }

  delete [] entries;

}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exsec v1.2 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.sec>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int toc_fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  int fd     = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  SECHDR hdr;
  read(toc_fd, &hdr, sizeof(hdr));
  hdr.flip_endian();

  unsigned long  filenames_len  = hdr.data_offset - hdr.filenames_offset;
  char*          filenames_buff = new char[filenames_len];
  lseek(fd, hdr.filenames_offset, SEEK_SET);
  read(fd, filenames_buff, filenames_len);

  for (unsigned long i = 0; i < hdr.section_count; i++) {
    process_dir(toc_fd, fd, hdr, filenames_buff, as::stringf("%05d", i));
  }

  delete [] filenames_buff;

  return 0;
}
