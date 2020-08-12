// exhxp.cpp, v1.1 2010/06/28
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts SHS7, SHS6 and Him5 (*.hxp) archives.

#include "as-util.h"

#pragma pack(1)
struct HXPHDR {
  char          signature[4]; // "SHS7", "SHS6", "Him5", "Him4"
  unsigned long entry_count;
};

struct HXPDATAHDR {
  unsigned long length;
  unsigned long original_length;
};

struct SHS7SECTENTRY {
  unsigned long section_length;
  unsigned long toc_offset;
};

struct SHS7ENTRY {
  unsigned char entry_length;
  unsigned long offset; // big endian
  char          name[1];
};

struct SHS6ENTRY {
  unsigned long offset;
};

struct IMGHDR {
  unsigned long  signature; // 0x20000
  unsigned long  unknown1;
  unsigned long  unknown2;
  unsigned short width;
  unsigned short height;
  unsigned char  depth;
  unsigned char  unknown3;
};
#pragma pack()

void unlz77ish(unsigned char* buff,
               unsigned long  len,
               unsigned char* out_buff,
               unsigned long  out_len)
{
  unsigned char* end = buff + len;

  while (buff < end) {
    unsigned char  c   = *buff++;
    unsigned long  n   = 0;
    unsigned char* src = NULL;

    if (c < 0x20) {
      if (c < 0x1D) {
        n = c + 1;
      } else if (c == 0x1D) {
        n = *buff++ + 0x1E;
      } else if (c == 0x1E) {
        n  = *buff++;
        n  = (n << 8) | *buff++;
        n += 0x11E;
      } else {
        n = *buff++;
        n = (n << 8) | *buff++;
        n = (n << 8) | *buff++;
        n = (n << 8) | *buff++;
      }

      src = buff;
      buff += n;
    } else {
      unsigned long p = 0;

      if (c & 0x80) { 
        n = (c >> 5) & 3;
        p = c & 0x1F;
        p = (p << 8) | *buff++;
      } else {
        switch (c >> 5) {
        case 0:
          // should be unreachable..
          fprintf(stderr, "unlz77ish: unexpected code!\n");
          break;

        case 1:
          n = c & 3;
          p = (c >> 2) & 7;
          break;

        case 2:
          n = (c & 0x1F) + 4;
          p = *buff++;
          break;

        case 3:
          p = c & 0x1F;
          p = (p << 8) | *buff++;

          n = *buff++;

          if (n == 0xFE) {
            n = *buff++;
            n = (n << 8) | *buff++;
            n += 0x102;
          } else if (n == 0xFF) {
            n = *buff++;
            n = (n << 8) | *buff++;
            n = (n << 8) | *buff++;
            n = (n << 8) | *buff++;
          } else {
            n += 4;
          }
          break;
        }
      }

      n += 3;
      p += 1;

      src = out_buff - p;
    }

    while (n--) {
      *out_buff++ = *src++;
    }
  }
}

void process_file(int fd, const string& name, unsigned long offset) {
  HXPDATAHDR datahdr;
  lseek(fd, offset, SEEK_SET);
  read(fd, &datahdr, sizeof(datahdr));

  unsigned long  len  = datahdr.length ? datahdr.length : datahdr.original_length;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  if (len != datahdr.original_length) {
    unsigned long  temp_len  = datahdr.original_length;
    unsigned char* temp_buff = new unsigned char[temp_len];
    unlz77ish(buff, len, temp_buff, temp_len);

    delete [] buff;

    len  = temp_len;
    buff = temp_buff;
  }

  IMGHDR* imghdr = (IMGHDR*) buff;

  if (imghdr->signature == 0x20000) {
    as::write_bmp(name + ".bmp",
                  buff + sizeof(*imghdr),
                  len  - sizeof(*imghdr),
                  imghdr->width,
                  imghdr->height,
                  imghdr->depth / 8);
  } else {
    as::write_file(name + as::guess_file_extension(buff, len), buff, len);
  }
  
  delete [] buff;
}

void process_shs7(int fd, const HXPHDR& hdr) {
  unsigned long sections_len = sizeof(SHS7SECTENTRY) * hdr.entry_count;
  SHS7SECTENTRY* sections    = new SHS7SECTENTRY[hdr.entry_count];
  read(fd, sections, sections_len);

  // "big enough"
  unsigned long  toc_len  = 1024 * 1024 * 5;
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    if (sections[i].section_length == 0) {
      continue;
    }

    unsigned char* p = toc_buff - sizeof(hdr) - sections_len + sections[i].toc_offset;  

    for (unsigned long j = 0; true; j++) {
      SHS7ENTRY* entry = (SHS7ENTRY*) p;
      p += entry->entry_length;

      if (entry->entry_length == 0) {
        break;
      }

      process_file(fd, entry->name, as::flip_endian(entry->offset));
    }
  }

  delete [] toc_buff;
  delete [] sections;
}

void process_shs6(int fd, const HXPHDR& hdr, const string& prefix) {
  SHS6ENTRY* entries = new SHS6ENTRY[hdr.entry_count];
  read(fd, entries, sizeof(SHS6ENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    process_file(fd, as::stringf("%s+%05d", prefix.c_str(), i), entries[i].offset);
  }

  delete [] entries;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exhxp v1.1 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.hxp>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  
  string prefix = as::get_file_prefix(in_filename, true);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  HXPHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  if (!memcmp(hdr.signature, "SHS7", 4) || !memcmp(hdr.signature, "Him5", 4)) {
    process_shs7(fd, hdr);
  } else if (!memcmp(hdr.signature, "SHS6", 4) || !memcmp(hdr.signature, "Him4", 4)) {
    process_shs6(fd, hdr, prefix);
  } else {
    fprintf(stderr, "%s: unknown signature\n", in_filename.c_str());
    exit(-1);
  }

  close(fd);

  return 0;
}
