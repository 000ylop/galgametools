// extttdpk.cpp, v1.0 2010/11/19
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts archives used by Tears To Tiara (PS3) & Tears to Tiara 
// Gaiden (PS3).

#include "as-util.h"
#include "zlib.h"

//#define DPK_VERSION 1
#define DPK_VERSION 2
#define EXTRACT_PDA

struct DKHHDR {
  unsigned char signature[4]; // "HKS@" ("HKD@")
  unsigned long entry_count;

  void flip_endian(void) {
    entry_count = as::flip_endian(entry_count);
  }
};

struct DKHENTRY1 {
  unsigned long      id;
  unsigned long      entry2_count;
#if DPK_VERSION > 1
  unsigned long long offset;
  unsigned long      length;
  unsigned long      entries2_offset;
#else
  unsigned long      entries2_offset;
  unsigned long      length;
#endif

  void flip_endian(void) {
    id              = as::flip_endian(id);
    entry2_count    = as::flip_endian(entry2_count);
#if DPK_VERSION > 1
    offset          = as::flip_endian_longlong(offset);    
#endif
    length          = as::flip_endian(length);
    entries2_offset = as::flip_endian(entries2_offset);
  }
};

struct DKHENTRY2 {
  unsigned long unknown1;
  unsigned long offset; // LBA
  unsigned long length;
  unsigned long original_length;

  void flip_endian(void) {
    offset          = as::flip_endian(offset);    
    length          = as::flip_endian(length);
    original_length = as::flip_endian(original_length);
  }
};

struct PDAHDR {
  unsigned char signature[4]; // "PDA@"
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long entry_count;
  unsigned long unknown3[6];
  unsigned long entries[1];
};

struct DDSHDR {
  unsigned char signature[4]; // "DDS "
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long height;
  unsigned long width;
  unsigned long length;
  unsigned long unknown3[16];
  unsigned long depth;
  unsigned long unknown4[9];
};

void unobfuscate(unsigned char* buff, unsigned long len, unsigned long seed) {
  unsigned long magic = 0x6C078965;

  for (unsigned long i = 0; i < len; i++) {
    seed *= magic;
    seed += 7;

    unsigned long key = seed >> 8;

    buff[i] -= (unsigned char) key;
    buff[i] ^= 0x36; 
  }
}

void unobfuscate2(unsigned char* buff, unsigned long len) {
  unsigned long long magic = 0xE7119EFE1120F40C;

  unsigned long long* p   = (unsigned long long*) buff;
  unsigned long long* end = p + len / 8;

  while (p < end) {
    *p++ ^= magic;
  }
}

void process_pda(const string& prefix, unsigned char* buff, unsigned long len) {
  PDAHDR* hdr = (PDAHDR*) buff;

  unsigned long entry_count = as::flip_endian(hdr->entry_count);

  for (unsigned long i = 0; i < entry_count; i++) {
    DDSHDR* dds = (DDSHDR*) (buff + as::flip_endian(hdr->entries[i]));

    if (dds->depth) {
      // Yes, lame :)
      as::write_bmp(prefix + as::stringf("+%05d.bmp", i),
                    (unsigned char*) (dds + 1),
                    dds->length - sizeof(*dds),
                    dds->width,
                    dds->height,
                    dds->depth / 8,
                    as::WRITE_BMP_BIGENDIAN | as::WRITE_BMP_FLIP);                    
    } else {
      as::write_file(prefix + as::stringf("+%05d.dds", i), (unsigned char*)dds, dds->length);
    }
  }
}

int main(int argc, char** argv) {
#if DPK_VERSION >= 2
  if (argc != 3) {
    fprintf(stderr, "extttdpk v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <datadkh> <datadkb>\n", argv[0]);
    return -1;
  }

  string dkb_filename(argv[2]);

  int dat_fd = as::open_or_die(dkb_filename, O_RDONLY | O_BINARY);
#else 
  if (argc != 2) {
    fprintf(stderr, "extttdpk v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <data.dkh>\n", argv[0]);
    return -1;
  }
#endif

  string dkh_filename(argv[1]);

  int fd = as::open_or_die(dkh_filename, O_RDONLY | O_BINARY);

  unsigned long  toc_len  = as::get_file_size(fd);
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);
  close(fd);

  DKHHDR* hdr = (DKHHDR*) toc_buff;
  hdr->flip_endian();

  unsigned long  entries1_len = hdr->entry_count * sizeof(DKHENTRY1); 
  DKHENTRY1*     entries1     = (DKHENTRY1*) (hdr + 1);

  unobfuscate(toc_buff + sizeof(*hdr), entries1_len, 0x11779933);
  unobfuscate(toc_buff + sizeof(*hdr) + entries1_len, toc_len - sizeof(*hdr) - entries1_len, 0xAABBCCDD);

  for (unsigned long i = 0; i < hdr->entry_count; i++) {
    entries1[i].flip_endian();

#if DPK_VERSION == 1
    int                dat_fd    = as::open_or_die(as::stringf("dpk/%08x.dpk", entries1[i].id), O_RDONLY | O_BINARY);
    unsigned long long data_base = 0;
#else
    unsigned long long data_base = entries1[i].offset;
#endif

    DKHENTRY2* entries2 = (DKHENTRY2*) (toc_buff + entries1[i].entries2_offset);

    for (unsigned long j = 0; j < entries1[i].entry2_count; j++) {
      entries2[j].flip_endian();

      unsigned long  len  = entries2[j].length;
      unsigned char* buff = new unsigned char[len];
      _lseeki64(dat_fd, data_base + entries2[j].offset, SEEK_SET);
      read(dat_fd, buff, len);
      unobfuscate2(buff, len);

      if (entries2[j].length != entries2[j].original_length) {
        unsigned long  out_len  = entries2[j].original_length;
        unsigned char* out_buff = new unsigned char[out_len];
        uncompress(out_buff, &out_len, buff, len);

        delete [] buff;

        len  = out_len;
        buff = out_buff;
      }

      string prefix = as::stringf("%05d+%05d", i, j);

      as::write_file(prefix + as::guess_file_extension(buff, len),
                     buff,
                     len);

#ifdef EXTRACT_PDA
      if (len >= 4 && !memcmp(buff, "PDA@", 4)) {
        process_pda(prefix, buff, len);
      }
#endif

      delete [] buff;
    }

#if DPK_VERSION == 1
    close(dat_fd);
#endif
  }

  return 0;
}