// exarc2.cpp, v1.0 2010/10/15
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts ARC2 archives.

#include "as-util.h"
#include "as-lzss.h"
#include <vector>

#pragma pack(1)
struct ARC2HDR {
  unsigned char signature[4]; // "ARC2"
  unsigned long entry_count;
};

struct ARC2ENTRY {
  unsigned long offset;
  unsigned long original_length;
  unsigned char filename_length;
};
#pragma pack()

struct full_entry_t {
  ARC2ENTRY entry;
  string    filename;
};

typedef std::vector<full_entry_t> full_entries_t;

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exarc2 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  ARC2HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  full_entries_t full_entries;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    ARC2ENTRY entry;
    read(fd, &entry, sizeof(entry));

    char filename[4096] = { 0 };
    read(fd, filename, entry.filename_length);

    for (unsigned long j = 0; j < entry.filename_length; j++) {
      filename[j] ^= 0xFF;
    }

    full_entry_t full_entry = { entry, filename };
    full_entries.push_back(full_entry);
  }

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long len = 0;
    
    if (i + 1 < hdr.entry_count) {
      len = full_entries[i + 1].entry.offset - full_entries[i].entry.offset;
    } else {
      len = as::get_file_size(fd) - full_entries[i].entry.offset;
    }

    unsigned char* buff = new unsigned char[len];
    lseek(fd, full_entries[i].entry.offset, SEEK_SET);
    read(fd, buff, len);

    if (len != full_entries[i].entry.original_length) {
      unsigned long  out_len  = full_entries[i].entry.original_length;
      unsigned char* out_buff = new unsigned char[out_len];
      as::unlzss(buff, len, out_buff, out_len);

      delete [] buff;

      len  = out_len;
      buff = out_buff;
    }

    bool is_wav = len >= 4 && !memcmp(buff, "RIFF", 4);
    bool is_ogg = len >= 4 && !memcmp(buff, "OggS", 4);
    
    if (!is_wav && !is_ogg) {
      for (unsigned long j = 0; j < len; j++) {
        buff[j] ^= 0xFF;
      }
    }

    as::make_path(full_entries[i].filename);
    as::write_file(full_entries[i].filename, buff, len);

    delete [] buff;
  }

  close(fd);

  return 0;
}
