// extcd3.cpp, v1.04 2010/08/27
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from TCD3 (*.tcd) archives.

#include "as-util.h"

#define TCD3_VERSION 2

struct TCD3HDRSECT {
  unsigned long toc_offset_relative; // toc_offset - sizeof(TCD3HDR)
  unsigned long toc_offset;
  unsigned long dir_count;
  unsigned long dir_name_length;
  unsigned long file_count;
  unsigned long file_name_length;
  unsigned long unknown1;
  unsigned long unknown2;
};

static const unsigned long TCD3_MAX_SECTIONS = 5;

struct TCD3HDR {
  unsigned char signature[4]; // "TCD3"
  unsigned long entry_count;  // total number of files in all sections
  TCD3HDRSECT   sections[TCD3_MAX_SECTIONS];
};

struct TCD3DIRENTRY {
  unsigned long entry_count;
  unsigned long filenames_offset;
  unsigned long start_index;
  unsigned long unknown;
};

struct TCD3FILEENTRY {
  unsigned long offset;
};

void unobfuscate(unsigned char* buff, 
                 unsigned long  len, 
                 unsigned long  entry_length) 
{
  unsigned char* end = buff + len;
  unsigned char  key = buff[entry_length - 1];

  while (buff < end) {
    *buff++ -= key;
  }
}

void unobfuscate2(unsigned char* buff, 
                  unsigned long  len, 
                  unsigned long  index)
{
  unsigned long* words = (unsigned long*) buff;

  unsigned long t = words[0] + 0x5053128D - 0x3A8EF9D1 * index;

  // Try version 4 (‚Ó‚¥‚¢‚Î‚è‚Á‚Æ)
  if (t == 0x43445053) {
    words[0]  = t;
    words[1] += 0x15C418BC - 0x3A8EF9D1 * index;
    words[2] += 0xC571062F * (index + 5);
    words[3] += 0xC571062F * (index + 6);
    words[4] += 0xC571062F * (index + 7);
    return;
  }

  t = words[0] + 0x4F7F0A46 - 0x3AD5A73E * index;

  // Try version 3 (‚È‚ÈƒvƒŠ)
  if (t == 0x43445053) {
    words[0]  = t;
    words[1] += 0x14A96308 - 0x3AD5A73E * index;
    words[2] += 0xC52A58C2 * (index + 5);
    words[3] += 0xC52A58C2 * (index + 6);
    words[4] += 0xC52A58C2 * (index + 7);
    return;
  }

  // Try version 2
  unsigned long n = std::min(len / 4, 5LU);

  for (unsigned long i = 0; i < n; i++) {
    t = words[i] + (0x137E59A1 * (index + 3 + i));

    if (i == 0 && t != 0x43445053) {
      // Assume version 1
      return;
    }

    words[i] = t;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "extcd3 v1.04 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.tcd>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  TCD3HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  for (unsigned long k = 0; k < TCD3_MAX_SECTIONS; k++) {
    TCD3HDRSECT& sect = hdr.sections[k];

    if (!sect.toc_offset) {
      continue;
    }

    lseek(fd, sect.toc_offset, SEEK_SET);

    unsigned long  dir_len  = sect.dir_count * sect.dir_name_length;
    unsigned char* dir_buff = new unsigned char[dir_len];    
    read(fd, dir_buff, dir_len);
    unobfuscate(dir_buff, dir_len, sect.dir_name_length);

    TCD3DIRENTRY* dir_entries = new TCD3DIRENTRY[sect.dir_count];
    read(fd, dir_entries, sizeof(TCD3DIRENTRY) * sect.dir_count);  

    unsigned long  file_len  = sect.file_count * sect.file_name_length;
    unsigned char* file_buff = new unsigned char[file_len];
    read(fd, file_buff, file_len);
    unobfuscate(file_buff, file_len, sect.file_name_length);

    // Extra entry at the end is convenient to compute the last file size
    TCD3FILEENTRY* file_entries = new TCD3FILEENTRY[sect.file_count + 1];
    read(fd, file_entries, sizeof(TCD3FILEENTRY) * (sect.file_count + 1));  

    char* dirname = (char*) dir_buff;

    for (unsigned long i = 0; i < sect.dir_count; i++) {
      char* filename = (char*) (file_buff + dir_entries[i].filenames_offset);

      for (unsigned long j = 0; j < dir_entries[i].entry_count; j++) {
        unsigned long file_index = dir_entries[i].start_index + j;

        unsigned long  len  = file_entries[file_index + 1].offset - file_entries[file_index].offset;
        unsigned char* buff = new unsigned char[len];
        lseek(fd, file_entries[file_index].offset, SEEK_SET);
        read(fd, buff, len);
        unobfuscate2(buff, len, file_index);

        string fullname = as::stringf("%s/%s", dirname, filename) + 
                            as::guess_file_extension(buff, len);

        as::make_path(fullname);

        int out_fd = as::open_or_die(fullname,
                                     O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                                     S_IREAD | S_IWRITE);
        write(out_fd, buff, len);
        close(out_fd);

        delete [] buff;

        filename += sect.file_name_length;
      }

      dirname += sect.dir_name_length;
    }

    delete [] file_entries;
    delete [] file_buff;
    delete [] dir_entries;
    delete [] dir_buff;
  }

  return 0;
}
