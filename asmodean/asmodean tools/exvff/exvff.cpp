// exvff.cpp, v1.01 2007/01/11
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from VFF (*.dat) archives.
// Same GALE images can be converted with transparency using a Photoshop plugin.
// To make it easier to do this in batch, two special command line options are
// provided.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <string>
#include <vector>
#include "zlib.h"

#pragma pack(1)
struct VFFHDR {
  unsigned char signature[6]; // "vff"
  unsigned long entry_count;
};

struct VFFENTRY {
  unsigned long offset;
  unsigned long unknown2;
};
#pragma pack()

using std::string;
using std::vector;

unsigned long inflate_buff(unsigned char* in_buff, 
                           unsigned long  in_len, 
                           unsigned char* out_buff, 
                           unsigned long  out_len) {
  z_stream d_stream;

  d_stream.zalloc    = NULL;
  d_stream.zfree     = NULL;
  d_stream.opaque    = NULL;
  d_stream.next_in   = in_buff;
  d_stream.avail_in  = in_len;
  d_stream.next_out  = out_buff;
  d_stream.avail_out = out_len; 

  unsigned long actual_out = 0;
     
  if (inflateInit(&d_stream) >= 0) {  
    inflate(&d_stream, Z_FINISH);
    inflateEnd(&d_stream);
    actual_out = d_stream.total_out;
  }

  return actual_out;
}

void unobfuscate_filename(unsigned char* buff, unsigned long len) {
  static unsigned long key   = 0;
  unsigned char*       key_p = (unsigned char*) &key;

  for (unsigned long i = 0; i <len; i++) {
    key += key * 4;
    key += 0x75D6EE39;

    buff[i] ^= *key_p;
  }
}

void unobfuscate_long(unsigned long* v) {
  static unsigned long key = 0;

  key += key * 4;
  key += 0x75D6EE39;

  *v ^= key;
}

void write_file(int fd, unsigned long offset, unsigned long len, string filename) {
  unsigned char* buff = new unsigned char[len];

  lseek(fd, offset, SEEK_SET);
  read(fd, buff, len);

  // Don't know how much data to expect, so ... "big enough"
  unsigned long  out_len     = 1024 * 1024 * 10;
  unsigned char* out_buff    = new unsigned char[out_len];
  unsigned long  inflate_len = inflate_buff(buff, len, out_buff, out_len);

  int out_fd = open(filename.c_str(), 
                    O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                    S_IREAD | S_IWRITE);

  if (out_fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  if (inflate_len) {
    write(out_fd, out_buff, inflate_len);
  } else {
    write(out_fd, buff, len);
  }

  close(out_fd);

  delete [] out_buff;
  delete [] buff;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "exvff v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <game.ext | game.dat> <game.dat> [-flatten | -unflatten]\n\n", argv[0]);
    fprintf(stderr, "\t-flatten   = flatten filenames for easy Photoshop batch processing\n");
    fprintf(stderr, "\t-unflatten = rename previously extracted/converted files to the real filename\n");
    return -1;
  }

  char* ext_filename = argv[1];
  char* dat_filename = argv[2];
  bool  flatten      = false;
  bool  unflatten    = false;

  if (argc > 4) {
    flatten   = !strcmp(argv[2], "-flatten");
    unflatten = !strcmp(argv[2], "-unflatten");   
  }
  
  int toc_fd = open(ext_filename, O_RDONLY | O_BINARY);

  if (toc_fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", ext_filename, strerror(errno));
    return -1;
  }

  int dat_fd = open(dat_filename, O_RDONLY | O_BINARY);

  if (dat_fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", dat_filename, strerror(errno));
    return -1;
  }

  unsigned long file_length;
  {
    struct stat file_stat;
    fstat(dat_fd, &file_stat);
    file_length = file_stat.st_size;
  }

  VFFHDR hdr;
  read(toc_fd, &hdr, sizeof(hdr));

  typedef vector<string> filename_list_t;
  filename_list_t filenames;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long name_length = 0;
    read(toc_fd, &name_length, sizeof(name_length));

    char name[4096] = { 0 };
    read(toc_fd, name, name_length);

    unobfuscate_filename((unsigned char*)name, name_length);

    for (unsigned int j = 0; j < name_length; j++) {
      if (name[j] == '/' || name[j] == '\\') {
        char t  = name[j];
        name[j] = '\0';
        mkdir(name);
        name[j] = t;
      }
    }

    string ext(name);
    {
      string::size_type pos = ext.find_last_of(".");

      if (pos != string::npos) {
        ext = ext.substr(pos);
      } else {
        ext = "";
      }
    }

    // Restore a tree of PNG images to the original filenames (hack for photoshop)
    if (unflatten) {
      char flat_name[4096] = { 0 };
      sprintf(flat_name, "%05d%s", i, ext.c_str());
      rename(flat_name, name);

      // Assume GAL images have been converted to PNG      
      string png_name(name);
      {
        string::size_type pos = png_name.find_last_of(".");
        if (pos != string::npos) {
          png_name.substr(0, pos);
        }
      }
      png_name += ".png";

      sprintf(flat_name, "%05d.png", i);
      if (rename(flat_name, png_name.c_str()) == -1) {
        //fprintf(stderr, "Failed to rename [%s] to [%s] (%s)\n", flat_name, name, strerror(errno));
      }

      continue;
    }

    // Flatten the tree of files (hack for photoshop)
    if (flatten) {
      sprintf(name, "%05d%s", i, ext.c_str());
    }

    filenames.push_back(name);
  }

  if (!unflatten) {
    unsigned long last_offset = 0;

    for (i = 0; i < hdr.entry_count; i++) {
      VFFENTRY entry;
      read(toc_fd, &entry, sizeof(entry));

      unobfuscate_long(&entry.offset);

      if (last_offset) {     
        write_file(dat_fd, last_offset, entry.offset - last_offset, filenames[i - 1]);
      }

      last_offset = entry.offset;
    }

    if (last_offset) {
      write_file(dat_fd, last_offset, file_length - last_offset, *filenames.rbegin());
    }
  }

  close(dat_fd);
  close(toc_fd);

  return 0;
}
