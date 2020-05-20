// exmnvdat.cpp, v1.02 2009/12/21
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts and decompresses 24-bit graphics and 8-bit masks from
// M no Violet's *.DAT archives.

#include "as-util.h"
#include "as-lzss.h"

//#define MNV_VERSION2
#define MNV_VERSION3

#ifdef MNV_VERSION3
#define MNV_FILENAME_LEN 68
#else
#define MNV_FILENAME_LEN 44
#endif

struct MNVHDR {
  unsigned long entry_count;
};

struct MNVENTRY {
  char          filename[MNV_FILENAME_LEN];
  unsigned long length;
  unsigned long offset;
};

struct MNVDATAHDR {
#ifdef MNV_VERSION3
  unsigned long type;
#else
  unsigned char signature[4]; // "mas" or "gra"
#endif
  unsigned long width;
  unsigned long height;
#ifdef MNV_VERSION3
  unsigned long depth;
#endif
  unsigned long length;
  unsigned long original_length;
};

struct MNV_DIFFHDR {
  unsigned long data_length;
  unsigned long original_data_length;
  unsigned long entries_length;
  unsigned long original_entries_length;
  unsigned long entry_count;
};

struct MNV_DIFFENTRY {
  unsigned long offset;
  unsigned long count;
};

void read_uncompress(int fd, unsigned long len, void* out_buff, unsigned long out_len) {
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  as::unlzss(buff, len, (unsigned char*)out_buff, out_len);

  delete [] buff;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmnvdat v1.02 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n", argv[0]);
    return -1;
  }

  string filename(argv[1]);
  
  int fd = as::open_or_die(filename, O_RDONLY | O_BINARY);

  MNVHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  MNVENTRY* entries = new MNVENTRY[hdr.entry_count];
  read(fd, entries, sizeof(MNVENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    MNVDATAHDR datahdr;
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, &datahdr, sizeof(datahdr));

    bool is_diff = false;    
#ifdef MNV_VERSION3
    is_diff = datahdr.type == 2;
#endif

    MNV_DIFFHDR diffhdr = { 0 };
    if (is_diff) {
      read(fd, &diffhdr, sizeof(diffhdr));
    }

    unsigned long depth = 0;

#ifdef MNV_VERSION3
    depth = datahdr.depth;
#else 
    if (!memcmp(datahdr.signature, "gra\0", 4)) {
      depth = 24;
    } else if (!memcmp(datahdr.signature, "mas\0", 4)) {
      depth = 8;
    } else {
      fprintf(stderr, "%s: unknown type\n", entries[i].filename);
      continue;
    }
#endif

    unsigned long  depth_bytes = depth / 8;

    unsigned long  out_len  = 0;
    unsigned char* out_buff = NULL;

    if (is_diff) {
      MNV_DIFFENTRY* diffentries = new MNV_DIFFENTRY[diffhdr.entry_count];
      read_uncompress(fd, diffhdr.entries_length, diffentries, sizeof(MNV_DIFFENTRY) * diffhdr.entry_count);      

      unsigned long  len  = diffhdr.original_data_length;
      unsigned char* buff = new unsigned char[diffhdr.original_data_length];
      read_uncompress(fd, diffhdr.data_length, buff, len);

      {
        string base_fn = entries[i].filename;
        base_fn = base_fn.substr(0, base_fn.length() - 1) + "1.bmp";

        unsigned long base_width  = 0;
        unsigned long base_height = 0;
        unsigned long base_depth  = 0;
        unsigned long base_stride = 0;

        as::read_bmp(base_fn,
                     out_buff,
                     out_len,
                     base_width,
                     base_height,
                     base_depth,
                     base_stride,
                     as::READ_BMP_TO24BIT);
      }

      unsigned char* p = buff;
      for (unsigned long j = 0; j < diffhdr.entry_count; j++) {
        memcpy(out_buff + diffentries[j].offset, p, diffentries[j].count);
        p += diffentries[j].count;
      }

      delete [] buff;
      delete [] diffentries;
    } else {
      out_len  = datahdr.original_length;
      out_buff = new unsigned char[out_len];
      read_uncompress(fd, datahdr.length, out_buff, out_len);
    }

    string out_filename = as::stringf("%s.bmp", entries[i].filename);

    as::make_path(out_filename);
    as::write_bmp_ex(out_filename,
                     out_buff,
                     out_len,
                     datahdr.width,
                     datahdr.height,
                     depth_bytes,
                     256, 
                     NULL);

    delete [] out_buff;
  }

  delete [] entries;

  close(fd);     

  return 0;
}
