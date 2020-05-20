// exdxa.cpp, v1.0r1 2010/09/19
// coded by asmodeanÅïâ¸ë¢8pvc09vr11

// This tool extracts *.dxa archives.

#include "as-util.h"

struct DXAHDR {
  unsigned char signature[4]; // "DX\x03\x00"
  unsigned long toc_length;
  unsigned long unknown1;
  unsigned long toc_offset;
  unsigned long entries_offset;
  unsigned long directories_offset;
};

struct DXADIRENTRY {
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long entry_count;
  unsigned long entries_offset;
};

struct DXAENTRY {
  unsigned long filename_offset;
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long unknown5;
  unsigned long unknown6;
  unsigned long unknown7;
  unsigned long offset;
  unsigned long original_length;
  unsigned long length;
};

#pragma pack(1)
  struct DXACOMPHDR {
    unsigned long original_length;
    unsigned long length;
    unsigned char escape;
  };
#pragma pack()  

void unobfuscate(unsigned long offset, unsigned char* buff, unsigned long len) {
  static const unsigned long KEY_LEN = 12;

  unsigned char key[KEY_LEN] = { 0 };
  memset(key, 0xAA, KEY_LEN);

  memcpy(key, "aja_ef_relea", KEY_LEN);

  key[0]  ^= 0xFF;
  key[1]   = (key[1] << 4) | (key[1] >> 4);
  key[2]  ^= 0x8A;
  key[3]   = ~((key[3] << 4) | (key[3] >> 4));
  key[4]  ^= 0xFF;
  key[5]  ^= 0xAC;
  key[6]  ^= 0xFF;
  key[7]   = ~((key[7] << 5) | (key[7] >> 3));
  key[8]   = (key[8] << 3) | (key[8] >> 5);
  key[9]  ^= 0x7F;
  key[10]  = ((key[10] << 4) | (key[10] >> 4)) ^ 0xD6;
  key[11] ^= 0xCC;

  for (unsigned long i = 0; i < len; i++) {
    buff[i] ^= key[(offset + i) % KEY_LEN];
  }
}

void read_unobfuscate(int fd, unsigned long offset, void* buff, unsigned long len) {
  lseek(fd, offset, SEEK_SET);
  read(fd, buff, len);

  unobfuscate(offset, (unsigned char*) buff, len);
}

void uncompress(unsigned char*  buff,
                unsigned long   len,
                unsigned char*& out_buff,
                unsigned long&  out_len)
{
  DXACOMPHDR* hdr = (DXACOMPHDR*) buff;

  unsigned char* end = buff + hdr->length;

  buff += sizeof(*hdr);

  out_len  = hdr->original_length;
  out_buff = new unsigned char[out_len];

  unsigned char* out_p   = out_buff;
  unsigned char* out_end = out_buff + out_len;

  while (buff < end) {
    while (buff < end) {
      unsigned char c = *buff++;

      if (c == hdr->escape) {
        c = *buff++;

        if (c == hdr->escape) {
          break;
        }

        if (c > hdr->escape) {
          c--;
        }

        unsigned long n = c >> 3;

        if (c & 4) {
          n |= *buff++ << 5;
        }

        n += 4;

        unsigned long p = 0;

        switch (c & 3) {
        case 0:
          p = *buff++;
          break;

        case 1:
          p = *(unsigned short*)buff;
          buff += 2;
          break;

        case 2:
          p = *(unsigned short*)buff;
          buff += 2;
          p |= *buff++ << 16;
          break;

        case 3:
          p = 0; // unused case?
        }

        p++;

        unsigned char* src = out_p - p;

        while (n--) {
          *out_p++ = *src++;
        }

      } else {
        *out_p++ = c;
      }
    }

    if (out_p < out_end) {
      *out_p++ = hdr->escape;
    }
  }
}

void process_dir(int            fd,
                 DXAHDR&        hdr,
                 unsigned char* toc_buff, 
                 unsigned long  dir_offset,
                 const string&  prefix)
{
  char*        filenames = (char*) toc_buff;
  DXADIRENTRY* dir       = (DXADIRENTRY*) (toc_buff + hdr.directories_offset + dir_offset);
  DXAENTRY*    entries   = (DXAENTRY*) (toc_buff + hdr.entries_offset + dir->entries_offset);

  unsigned long base_offset = sizeof(hdr);

  for (unsigned long i = 0; i < dir->entry_count; i++) {
    // Not really sure what all the crap and duplicate strings are in the filenames
    char* filename = filenames + entries[i].filename_offset + 4;

    string out_filename = prefix + "/" + filename;

    if (entries[i].original_length) {
      if (entries[i].length != -1) {
        unsigned long  len  = entries[i].length;
        unsigned char* buff = new unsigned char[len];    
        read_unobfuscate(fd, base_offset + entries[i].offset, buff, len);

        unsigned long  out_len  = 0;
        unsigned char* out_buff = NULL;
        uncompress(buff, len, out_buff, out_len);

        as::make_path(out_filename);
        as::write_file(out_filename, out_buff, out_len);

        delete [] out_buff;
        delete [] buff;
      } else {
        unsigned long  len  = entries[i].original_length;
        unsigned char* buff = new unsigned char[len];    
        read_unobfuscate(fd, base_offset + entries[i].offset, buff, len);

        as::make_path(out_filename);
        as::write_file(out_filename, buff, len);

        delete [] buff;
      }
    } else {
      if (entries[i].offset) {
        process_dir(fd, hdr, toc_buff, entries[i].offset, out_filename);
      }
    }
  }

}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exdxa v1.0r1 by asmodeanÅïâ¸ë¢8pvc09vr11\n\n");
    fprintf(stderr, "usage: %s <input.dxa>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  DXAHDR hdr;
  read_unobfuscate(fd, 0, &hdr, sizeof(hdr));

  unsigned long  toc_len  = hdr.toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];
  read_unobfuscate(fd, hdr.toc_offset, toc_buff, toc_len);

  process_dir(fd, hdr, toc_buff, 0, ".");

  delete [] toc_buff;

  return 0;
}
