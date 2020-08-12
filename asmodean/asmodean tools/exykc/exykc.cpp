// exykc.cpp, v1.1 2010/10/15
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from YKC001 (*.ykc, *.dat) archives.
// YKG000 (*.ykg) composite images are converted to PNG images.

#include "as-util.h"

// I coded this but decided it made merging alpha masks annoying.
#undef USE_PNG_TITLE

struct YKCHDR {
  unsigned char signature[8]; // "YKC001"
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long toc_offset;
  unsigned long toc_length;
};

struct YKCENTRY {
  unsigned long filename_offset;
  unsigned long filename_length;
  unsigned long offset;
  unsigned long length;
  unsigned long unknown;
};

struct YKGHDR {
  unsigned char signature[8]; // "YKG000"
  unsigned long hdr_length;
  unsigned char unknown1[28];
  unsigned long rgb_offset;
  unsigned long rgb_length;
  unsigned long msk_offset;
  unsigned long msk_length;
  unsigned long unknown2;
  unsigned long unknown3;
};

void write_data(const string& filename, unsigned char* buff, unsigned long len, bool mask = false) { 
  string out_filename = filename;

  unsigned long* sig = (unsigned long*) buff;
  if (*sig == 0x504E4789) {
    *sig = 0x474E5089;
    out_filename = as::get_file_prefix(out_filename) + ".png";
  }

  if (mask) {
    out_filename += as::get_file_prefix(out_filename) + "_m" + as::guess_file_extension(buff, len);
  }

#ifdef USE_PNG_TITLE
  // Dirty hack, but it's kindof nice to have the actual part name
  if (!memcmp(buff + 37, "tEXtTitle", 9)) {
    unsigned char* partname_length = buff + 36;
    char           partname[1024] = { 0 };
    memcpy(partname, buff + 47, *partname_length - 6);

    out_filename += "+";
    out_filename += partname;
  }
#endif   

  as::make_path(out_filename);
  as::write_file(out_filename, buff, len);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exykc v1.1 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.ykc|input.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  YKCHDR hdr;  
  read(fd, &hdr, sizeof(hdr));

  unsigned long entry_count = hdr.toc_length / sizeof(YKCENTRY);
  YKCENTRY*     entries     = new YKCENTRY[entry_count];
  _lseeki64(fd, hdr.toc_offset, SEEK_SET);
  read(fd, entries, hdr.toc_length);

  for (unsigned long int i = 0; i < entry_count; i++) {
    char filename[4096] = { 0 };

    _lseeki64(fd, entries[i].filename_offset, SEEK_SET);
    read(fd, filename, entries[i].filename_length);    

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];

    _lseeki64(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    YKGHDR* ykghdr = (YKGHDR*) buff;

    if (!memcmp(ykghdr->signature, "YKG000", 6)) {
      if (ykghdr->rgb_length) {
        write_data(filename, buff + ykghdr->rgb_offset, ykghdr->rgb_length);
      }

      if (ykghdr->msk_length) {
        write_data(filename, buff + ykghdr->msk_offset, ykghdr->msk_length, true);
      }
    } else {
      write_data(filename, buff, len);
    }

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
