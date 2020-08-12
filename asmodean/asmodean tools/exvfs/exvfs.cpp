// exvfs.cpp, v1.0 2010/07/30
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts VF (*.vfs) archives.

#include "as-util.h"

#pragma pack(1)

struct VFHDR1 {
  unsigned char  signature[4]; // "VF\x00\x02"
  unsigned short entry_count;
  unsigned short entry_length;
  unsigned long  toc_length;
  unsigned long  file_size;
};

struct VFHDR2 {
  unsigned long filenames_length; // in wchar_t
  unsigned long unknown1;
};

struct VFENTRY {
  unsigned long  filename_offset; // in wchar_t
  unsigned short unknown1;
  unsigned long  unknown2;
  unsigned long  offset;
  unsigned long  length;
  unsigned long  length2;
  unsigned char  unknown3;
};

struct AGFHDR {
  unsigned char signature[4]; // "AGF"
  unsigned long unknown1;
  unsigned long length;
  unsigned long unknown2; // 0x74
  unsigned long unknown3; // 0
  unsigned long unknown4; // 0
  unsigned long unknown5; // 0x5C
  unsigned long width;
  unsigned long height;
  unsigned long width2;
  unsigned long height2;
  unsigned char unknown6[72];
};

#pragma pack()

bool process_agf(const string&  filename,
                 unsigned char* buff,
                 unsigned long len)
{
  AGFHDR* hdr = (AGFHDR*) buff;

  if (memcmp(hdr->signature, "AGF", 3)) {
    return false;
  }

  unsigned long  out_len  = hdr->width * hdr->height * 4;
  unsigned char* out_buff = new unsigned char[out_len];

  unsigned long* p       = (unsigned long*) (hdr + 1);
  unsigned long* out_p   = (unsigned long*) out_buff;
  unsigned long* out_end = out_p + out_len / 4;

  while (out_p < out_end) {
    unsigned long flags = *p++;
    unsigned long n     = flags >> 8;
    flags &= 0xFF;

    switch (flags) {
      case 1:
        while (n--) {
          *out_p++ = *p++;
        }
        break;

      case 2:
        while (n--) {
          *out_p++ = *p;
        }
        p++;
        break;

      case 3:
        {
          unsigned long r = n >> 8;
          n &= 0xFF;

          unsigned long* src = NULL;

          while (n--) {
            src = p;

            for (unsigned long i = 0; i < r; i++) {
              *out_p++ = *src++;
            }
          }

          p = src;
        }
        break;

      case 4:
        {
          unsigned long p = n & 0x7FF;
          n >>= 12;

          unsigned long* src = out_p - p;

          while (n--) {
            *out_p++ = *src++;
          }
        }
        break;

      default:
        fprintf(stderr, "%s: unrecognized compression code (%d)\n", filename.c_str(), flags);
        delete [] out_buff;
        return false;
    }
  }

  as::write_bmp(as::get_file_prefix(filename) + ".bmp",
                out_buff,
                out_len,
                hdr->width,
                hdr->height,
                4,
                as::WRITE_BMP_FLIP);

  delete [] out_buff;

  return true;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exvfs v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.vfs>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  VFHDR1 hdr;
  read(fd, &hdr, sizeof(hdr));

  VFENTRY* entries = new VFENTRY[hdr.entry_count];
  read(fd, entries, sizeof(VFENTRY) * hdr.entry_count);

  VFHDR2 hdr2;
  read(fd, &hdr2, sizeof(hdr2));

  wchar_t* filenames = new wchar_t[hdr2.filenames_length];
  read(fd, filenames, sizeof(wchar_t) * hdr2.filenames_length);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    string filename = as::convert_wchar(filenames + entries[i].filename_offset);

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (!process_agf(filename, buff, len)) {
      as::write_file(filename, buff, len);
    }

    delete [] buff;
  }

  delete [] filenames;
  delete [] entries;

  close(fd);

  return 0;
}
