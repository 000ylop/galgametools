// exsui2rom.cpp, v1.01 2012/01/09
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.dat archives used by êÖåé ìÛ.

#include "as-util.h"
#include "as-ps3.h"
#include "zlib.h"

struct CMPHDR {
  unsigned long signature;
  unsigned long length;
  unsigned long original_length;

  void flip_endian(void) {
    length          = as::flip_endian(length);
    original_length = as::flip_endian(original_length);
  }
};

struct PACKHDR {
  unsigned char signature[4]; // "KCAP"
  unsigned long length;
  unsigned long alignment;
  char          type_name[1];

  void flip_endian(void) {
    length    = as::flip_endian(length);
    alignment = as::flip_endian(alignment);
  }
};

struct DATAHDR {
  unsigned char signature[4]; // "ATAD"
  unsigned long length;

  void flip_endian(void) {
    length = as::flip_endian(length);
  }
};

struct DCGPHDR {
  unsigned char signature[4]; // "PGCD"
  unsigned long length;
  unsigned long block_count;
  unsigned char unknown[128];

  void flip_endian(void) {
    length      = as::flip_endian(length);
    block_count = as::flip_endian(block_count);
  }
};

typedef DATAHDR PICIHDR;

struct PICIENTRY {
  unsigned short index;

  void flip_endian(void) {
    index = as::flip_endian_short(index);
  }
};

struct PICI {
  PICIHDR       hdr;
  PICIENTRY     entries[1];

  void flip_endian(void) {
    hdr.flip_endian();

    unsigned long entry_count = (hdr.length - sizeof(hdr)) / sizeof(PICIENTRY);

    for (unsigned long i = 0; i < entry_count; i++) {
      entries[i].flip_endian();
    }
  }
};

typedef PICI PICC;
typedef PICIENTRY PICCENTRY;

struct DEADBEEFHDR : public DATAHDR {
  void skip(unsigned char*& buff) {
    if (!memcmp(signature, "\xde\xad\xbe\xef", 4)) {
      flip_endian();
      buff += length;
    }
  }
};

typedef DATAHDR COLHDR;

typedef DATAHDR IMGHDR;

struct CLUTHDR {
  unsigned char signature[4]; // "TULC"
  unsigned long length;
  unsigned long block_count;

  void flip_endian(void) {
    length      = as::flip_endian(length);
    block_count = as::flip_endian(block_count);
  }
};

struct CLUTENTRY {
  unsigned short bits;
  unsigned short entry_count;
  unsigned long  col_offset;

  void flip_endian(void) {
    bits        = as::flip_endian_short(bits);
    entry_count = as::flip_endian_short(entry_count);
    col_offset  = as::flip_endian(col_offset);
  }
};

struct CLUT {
  CLUTHDR   hdr;
  CLUTENTRY entries[1];

  void flip_endian(void) {
    hdr.flip_endian();

    for (unsigned long i = 0; i < hdr.block_count; i++) {
      entries[i].flip_endian();
    }
  }
};

typedef CLUTHDR IMGIHDR;

struct IMGIENTRY {  
  unsigned short unknown1;
  unsigned short block_width;
  unsigned short block_height;
  unsigned short unknown2;
  unsigned long  img_offset;

  void flip_endian(void) {
    unknown1     = as::flip_endian_short(unknown1);
    block_width  = as::flip_endian_short(block_width);
    block_height = as::flip_endian_short(block_height);
    unknown2     = as::flip_endian_short(unknown2);
    img_offset   = as::flip_endian(img_offset);
  }
};

struct IMGI {
  IMGIHDR   hdr;
  IMGIENTRY entries[1];

  void flip_endian(void) {
    hdr.flip_endian();

    for (unsigned long i = 0; i < hdr.block_count; i++) {
      entries[i].flip_endian();
    }
  }
};

struct DCPIHDR {
  unsigned char  signature[4]; // "IPCD"
  unsigned short width;
  unsigned short height;
  unsigned short block_width;
  unsigned short block_height;
  unsigned short width_blocks;
  unsigned short height_blocks;

  void flip_endian(void) {
    width         = as::flip_endian_short(width);
    height        = as::flip_endian_short(height);
    block_width   = as::flip_endian_short(block_width);
    block_height  = as::flip_endian_short(block_height);
    width_blocks  = as::flip_endian_short(width_blocks);
    height_blocks = as::flip_endian_short(height_blocks);
  }
};

struct DCPIENTRY {
  unsigned short picx_index; // offset?

  void flip_endian(void) {
    picx_index = as::flip_endian_short(picx_index);

    // It's wrong as both and index and a offset.  Hm...
    picx_index /= 4;
  }
};

struct DCPI {
  DCPIHDR   hdr;
  DCPIENTRY entries[1];

  void flip_endian(void) {
    hdr.flip_endian();

    unsigned long block_count = hdr.width_blocks * hdr.height_blocks;

    for (unsigned long i = 0; i < block_count; i++) {
      entries[i].flip_endian();
    }
  }
};

unsigned long aligned_len(unsigned long len, unsigned long alignment) {
  unsigned long remain = len % alignment;

  if (remain) {
    len += alignment - remain;
  }

  return len;
}

bool proc_pack(const string&  filename,
               unsigned char* buff, 
               unsigned long len)
{
  if (memcmp(buff, "KCAP", 4)) {
    return false;
  }

  PACKHDR* hdr = (PACKHDR*) buff;
  hdr->flip_endian();
  buff += aligned_len(sizeof(*hdr), hdr->alignment);

  DATAHDR* datahdr = (DATAHDR*) buff;
  datahdr->flip_endian();

  unsigned char* data_buff = buff + aligned_len(sizeof(*datahdr), hdr->alignment);

  buff += aligned_len(datahdr->length, hdr->alignment);

  DCGPHDR* dcgphdr = (DCGPHDR*) data_buff;
  dcgphdr->flip_endian();

  unsigned char* dcgp_buff = data_buff + sizeof(*dcgphdr);

  data_buff += aligned_len(dcgphdr->length, hdr->alignment);

  PICI* pici = (PICI*) dcgp_buff;
  pici->flip_endian();
  dcgp_buff += pici->hdr.length;

  PICC* picc = (PICC*) dcgp_buff;
  picc->flip_endian();
  dcgp_buff += picc->hdr.length;
  
  DEADBEEFHDR* deadbeefhdr = (DEADBEEFHDR*) dcgp_buff;
  deadbeefhdr->skip(dcgp_buff);

  COLHDR* colhdr = (COLHDR*) dcgp_buff;
  colhdr->flip_endian();

  unsigned char* col_buff = dcgp_buff + sizeof(*colhdr);

  dcgp_buff += colhdr->length;

  // This isn't always there
  deadbeefhdr = (DEADBEEFHDR*) dcgp_buff;
  deadbeefhdr->skip(dcgp_buff);

  IMGHDR* imghdr = (IMGHDR*) dcgp_buff;
  imghdr->flip_endian();

  unsigned char* img_buff = dcgp_buff + sizeof(*imghdr);

  dcgp_buff += imghdr->length;

  CLUT* clut = (CLUT*) dcgp_buff;
  clut->flip_endian();
  dcgp_buff += clut->hdr.length;

  IMGI* imgi = (IMGI*) dcgp_buff;
  imgi->flip_endian();
  dcgp_buff += imgi->hdr.length;

  DCPI* dcpi = (DCPI*) data_buff;
  dcpi->flip_endian();

  unsigned long  out_len      = dcpi->hdr.width * dcpi->hdr.height * 4;
  unsigned char* out_buff     = new unsigned char[out_len];

  unsigned long  block_pixels = dcpi->hdr.block_width * dcpi->hdr.block_height;
  unsigned long  block_len    = block_pixels * 4;
  unsigned char* block_buff   = new unsigned char[block_len];

  unsigned long block_i = 0;
  for (unsigned long by = 0; by < dcpi->hdr.height_blocks; by++) {
    for (unsigned long bx = 0; bx < dcpi->hdr.width_blocks; bx++, block_i++) {
      IMGIENTRY& imgientry = imgi->entries[pici->entries[dcpi->entries[block_i].picx_index].index];
      CLUTENTRY& clutentry = clut->entries[picc->entries[dcpi->entries[block_i].picx_index].index];

      unsigned char* in  = img_buff + imgientry.img_offset;
      unsigned long* out = (unsigned long*) block_buff;
      unsigned long* pal = (unsigned long*) (col_buff + clutentry.col_offset);

      for (unsigned long i = 0; i < block_pixels; i++) {
        unsigned long index = 0;

        switch (clutentry.bits) {
          case 8:
            index = *in++;
            break;

          // I haven't actually seen more than 8 bits though...
          case 16:
            index = as::flip_endian_short(*(unsigned short*)in);
            in += 2;
            break;

          case 32:
            index = as::flip_endian(*(unsigned short*)in);
            in += 4;
            break;
        }

        *out++ = as::flip_endian(pal[index]);
      }

      as::unswizzle_ps3(block_buff, block_len, dcpi->hdr.block_width, dcpi->hdr.block_height);

      unsigned long out_block_x = bx * dcpi->hdr.block_width;
      unsigned long out_block_y = by * dcpi->hdr.block_height;

      for (unsigned long y = 0; y < imgientry.block_height && out_block_y + y < dcpi->hdr.height; y++) {
        as::RGBA* src = (as::RGBA*) (block_buff + (dcpi->hdr.block_height - y - 1) * dcpi->hdr.block_width * 4);
        as::RGBA* dst = (as::RGBA*) (out_buff   + (out_block_y + y) * dcpi->hdr.width * 4);

        for (unsigned long x = 0; x < imgientry.block_width && out_block_x + x < dcpi->hdr.width; x++) {
          dst[out_block_x + x] = src[x];
        }
      }
    }
  }

  as::write_bmp(filename + ".bmp",
                out_buff,
                out_len,
                dcpi->hdr.width,
                dcpi->hdr.height,
                4,
                as::WRITE_BMP_FLIP);

  delete [] out_buff;
  delete [] block_buff;

  return true;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exsui2rom v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.datk>\n", argv[0]);    

    return -1;
  }

  string in_filename(argv[1]);
  string prefix = as::get_file_prefix(in_filename, true);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);  
  
  unsigned long offset = 2048;

  for (unsigned long i = 0; true; i++) {
    string filename = prefix + as::stringf("+%05d", i);

    CMPHDR hdr;
    lseek(fd, offset, SEEK_SET);

    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
      break;
    }

    unsigned long  len  = 0;
    unsigned char* buff = NULL;

    if (hdr.signature == 0x1A619) {
      hdr.flip_endian();

      unsigned long  temp_len  = hdr.length;
      unsigned char* temp_buff = new unsigned char[temp_len];
      read(fd, temp_buff, temp_len);
      offset += sizeof(hdr) + temp_len;

      len  = hdr.original_length;
      buff = new unsigned char[len];
      uncompress(buff, &len, temp_buff, temp_len);

      delete [] temp_buff;

    // This is kinda pointless, but since I already added it...
    } else if (hdr.signature == 0x46464952) {
      len = hdr.length + 8;
      lseek(fd, -(int)sizeof(hdr), SEEK_CUR);
    } else if (hdr.signature == 0x4346534D) {
      read(fd, &len, sizeof(len));
      len = as::flip_endian(len) + 64;
      lseek(fd, -(int)sizeof(hdr) - sizeof(len), SEEK_CUR);
    } else if (hdr.signature == 0x4C495042) {
      len = 20;
      lseek(fd, -(int)sizeof(hdr), SEEK_CUR);
    } else {
      printf("%s: unknown file type at 0x%X (stopped)\n", filename.c_str(), offset);
      break;
    }

    if (!buff) {
      buff = new unsigned char[len];
      read(fd, buff, len);
      offset += len;
    }

    if (!proc_pack(filename, buff, len)) {
      as::write_file(filename + as::guess_file_extension(buff, len), 
                     buff,
                     len);
    }

    delete [] buff;

    offset = (offset + 2047) & ~2047;
  }

  close(fd);

  return 0;
}
