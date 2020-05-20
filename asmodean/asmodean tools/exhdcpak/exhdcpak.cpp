// exhdcpak.cpp, v1.1 2008/05/30
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from HdC's PACK (*.pak) archives.

#include "as-util.h"
#include "as-lzss.h"

// Newer games use unicode
//#define UNICODE_FILENAMES

struct PACKHDR {
  unsigned char signature[4]; // "PACK"
  unsigned long entry_count;
  unsigned long unknown;
};

struct PACKENTRY {
#ifdef UNICODE_FILENAMES
  wchar_t       filename[32];
#else
  char          filename[32];
#endif
  unsigned long original_length;
  unsigned long length;
  unsigned long offset;
};

struct OPFHDR {
  unsigned char signature[4]; // "OPF " or "OPF2"
  unsigned long width;
  unsigned long height;
  unsigned long depth;
  unsigned long stride;
  unsigned long unknown;
  unsigned long original_length;
  unsigned long length;
};

void unobfuscate(unsigned char* buff, unsigned long len) {
  unsigned char* end = buff + len;

  while (buff < end) {
    *buff++ = ((*buff >> 4) & 0x0F) | *buff << 4;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exhdcpak v1.1 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PACKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  PACKENTRY*    entries     = new PACKENTRY[hdr.entry_count];
  unsigned long entries_len = sizeof(PACKENTRY) * hdr.entry_count;
  read(fd, entries, entries_len);
  unobfuscate((unsigned char*)entries, entries_len);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
#ifdef UNICODE_FILENAMES
    string out_filename(as::convert_wchar(entries[i].filename));
#else
    string out_filename(entries[i].filename);
#endif

    unsigned long  len  = entries[i].length ? entries[i].length : entries[i].original_length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (entries[i].length) {
      unsigned long  temp_len  = entries[i].original_length;
      unsigned char* temp_buff = new unsigned char[temp_len];
      as::unlzss(buff, len, temp_buff, temp_len);

      delete [] buff;
      len  = temp_len;
      buff = temp_buff;
    }

    OPFHDR* opfhdr = (OPFHDR*) buff;

    if (!memcmp(opfhdr->signature, "OPF2", 4)) {
      unsigned long  temp_len  = opfhdr->original_length;
      unsigned char* temp_buff = new unsigned char[temp_len];
      as::unlzss(buff + sizeof(*opfhdr), opfhdr->length, temp_buff, temp_len);

      as::write_bmp(as::get_file_prefix(out_filename) + ".bmp", 
                    temp_buff,
                    temp_len,
                    opfhdr->width,
                    opfhdr->height,
                    opfhdr->depth / 8,
                    as::WRITE_BMP_FLIP);      

      delete [] temp_buff;
    } else if (!memcmp(opfhdr->signature, "OPF ", 4)) {
      // Probably could merge this case with the above, but I don't have any
      // old OPF to test against at the moment.
      as::write_bmp(as::get_file_prefix(out_filename) + ".bmp", 
                    buff + sizeof(*opfhdr),
                    len  - sizeof(*opfhdr),
                    opfhdr->width,
                    opfhdr->height,
                    opfhdr->depth / 8,
                    as::WRITE_BMP_FLIP);
    } else {
      as::write_file(out_filename, buff, len);
    }   

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
