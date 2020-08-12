// exddp.cpp, v1.0 2009/08/12
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts DDP2/DDP3 (*.dat) archives.

#include "as-util.h"

#pragma pack(1)
struct DDPHDR {
  unsigned char signature[4]; // "DDP3" or "DDP2"
  unsigned long entry_count;
  unsigned long toc_length;
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long unknown5;
};

struct DDP2ENTRY {
  unsigned long offset;
  unsigned long original_length;
  unsigned long length;
  unsigned long unknown1;
};

struct DDP3ENTRY1 {
  unsigned long entry_length;
  unsigned long entry_offset;
};

struct DDP3ENTRY2 {
  unsigned char entry_length;
  unsigned long offset;
  unsigned long original_length;
  unsigned long length;
  unsigned long unknown1;
  char          filename[1];
};

#pragma pack()

// I wasn't expecting to see this again ...
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

template<class ENTRY_T> 
void 
process_entry(int            fd,
              const string&  filename, 
              const ENTRY_T& entry)
{
  unsigned long  out_len  = entry.original_length;
  unsigned char* out_buff = new unsigned char[out_len];  
  
  lseek(fd, entry.offset, SEEK_SET);

  if (entry.length) {
    unsigned long  len  = entry.length;
    unsigned char* buff = new unsigned char[len];    
    read(fd, buff, len);

    unlz77ish(buff, len, out_buff, out_len);

    delete [] buff;
  } else {
    read(fd, out_buff, out_len);
  }

  as::write_file(filename + as::guess_file_extension(out_buff, out_len), 
                 out_buff,
                 out_len);

  delete [] out_buff;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exddp v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  DDPHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  if (memcmp(hdr.signature, "DDP", 3)) {
    fprintf(stderr, "%s: Not a DDP archive (might be MPEG)\n", in_filename.c_str());
    return 0;
  }

  unsigned long  toc_len  = hdr.toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];
  lseek(fd, 0, SEEK_SET);
  read(fd, toc_buff, toc_len);

  if (!memcmp(hdr.signature, "DDP3", 4)) {
    DDP3ENTRY1* entries = (DDP3ENTRY1*) (toc_buff + sizeof(hdr));

    for (unsigned long i = 0; i < hdr.entry_count; i++) {
      unsigned char* p   = toc_buff + entries[i].entry_offset;
      unsigned char* end = p + entries[i].entry_length;

      while (end - p >= sizeof(DDP3ENTRY2)) {
        DDP3ENTRY2* entry = (DDP3ENTRY2*) p;
        p += entry->entry_length;

        process_entry(fd, entry->filename, *entry);
      }
    }
  } else if (!memcmp(hdr.signature, "DDP2", 4)) {
    string     prefix  = as::get_file_prefix(in_filename, true);
    DDP2ENTRY* entries = (DDP2ENTRY*) (toc_buff + sizeof(hdr));

    for (unsigned long i = 0; i < hdr.entry_count; i++) {
      if (entries[i].original_length) {
        process_entry(fd, prefix + as::stringf("%05d", i), entries[i]);
      }
    }
  } else {
    fprintf(stderr, "%s: Unknown DDP type (%d)\n", in_filename.c_str(), hdr.signature[3]);
  }

  delete [] toc_buff;

  return 0;
}
