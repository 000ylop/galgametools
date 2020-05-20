// exarc4.cpp, v1.0 2007/03/11
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from F&C's ARC4 (*.BIN) archives.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include "arc4common.h"

using std::string;

#pragma pack(1)
struct ARC4HDR {
  unsigned char signature[4];     // "ARC4"
  unsigned long unknown1;
  unsigned long toc_length;
  unsigned long unknown2;         // 2048 - block size?
  unsigned long entry_count;
  unsigned long header_length;    // 48
  unsigned long table_length;     // first part of uncompressed toc
  unsigned long filenames_offset; // header_length + table_length
  unsigned long filenames_length;
  unsigned long subtoc_offset;   // header_length + table_length + filenames_length
  unsigned long subtoc_length;
  unsigned long data_base;
};

struct ARC4ENTRY {
  unsigned char filename_offset[3]; // in 2-byte blocks
  unsigned char filename_length;
  unsigned char subentry_count;
  unsigned char offset[3];          // in 2048-byte blocks (relative to data_base)
};

struct ARC4DATAHDR {
  unsigned long length_blocks; // in 2048-byte blocks
  unsigned long length;
  unsigned long length2;       // ??
  unsigned long unknown;       // padding
};

#pragma pack()

void save_entry(int fd, unsigned long offset, const string& filename) {
  offset *= 2048;

  ARC4DATAHDR datahdr;
  lseek(fd, offset, SEEK_SET);
  read(fd, &datahdr, sizeof(datahdr));

  datahdr.length_blocks = flip_endian(datahdr.length_blocks);
  datahdr.length        = flip_endian(datahdr.length);
  datahdr.length2       = flip_endian(datahdr.length2);

  unsigned long  len  = datahdr.length;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  int out_fd = open_or_die(filename,
                           O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                           S_IREAD | S_IWRITE);
  write(out_fd, buff, len);
  close(out_fd);

  delete [] buff;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exarc4 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bin>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  
  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  ARC4HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = 0;
  unsigned char* toc_buff = NULL;
  {
    unsigned char* buff = new unsigned char[hdr.toc_length];
    read(fd, buff, hdr.toc_length);

    uncompress_sequence(buff, hdr.toc_length, toc_buff, toc_len, in_filename);

    delete [] buff;
  }

  unsigned char* subtoc_buff = toc_buff + hdr.subtoc_offset - hdr.header_length;

  ARC4ENTRY* entries   = (ARC4ENTRY*) toc_buff;
  char*      filenames = (char*) (toc_buff + hdr.table_length);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long filename_offset = get_stupid_long(entries[i].filename_offset) * 2;
    unsigned long offset          = get_stupid_long(entries[i].offset);

    string filename(filenames + filename_offset, entries[i].filename_length);

    if (entries[i].subentry_count == 1) {
      save_entry(fd, hdr.data_base + offset, filename);
    } else {
      offset *= 3;

      for (unsigned long j = 0; j < entries[i].subentry_count; j++) {
        string prefix = get_file_prefix(filename);
        string ext    = get_file_extension(filename);

        char   subname[4096] = { 0 };
        sprintf(subname, "%s+%03d.%s", prefix.c_str(), j, ext.c_str());

        unsigned long real_offset = get_stupid_long(subtoc_buff + offset);
        offset += 3;

        save_entry(fd, 
                   hdr.data_base + real_offset, 
                   subname);
      }
    }
  }

  delete [] toc_buff;

  return 0;
}


