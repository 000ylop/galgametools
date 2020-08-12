// exmpsaf.cpp, v1.0 2007/06/20
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts Milky Pearl's *.saf archives.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstdio>
#include <string>
#include <list>
#include "zlib.h"

using std::string;
using std::list;

struct SAFHDR {
  unsigned short unknown;
  unsigned short entry_count;
  unsigned long  filenames_length;
};

static const unsigned long DIRECTORY_FLAG = 0x80000000;

struct SAFENTRY {
  unsigned long filename_offset;
  unsigned long offset; // in 2048-byte blocks
  unsigned long length; // in bytes
  unsigned long original_length;
};

void unobfuscate(unsigned char* buff, unsigned long len) {
  unsigned char* end = buff + len;

  while (buff < end) {
    for (unsigned char k = 0xEF; k < 0xFF && buff < end; k++) {
      *buff++ ^= k;
    }
  }
}

void unobfuscate_filenames(char* buff, unsigned long len) {
  char* end = buff + len;

  for (unsigned char k = 0xFF; buff < end; k--) {
    *buff++ ^= k;
  }
}

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

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

class directories_t {
public:
  void add(const string& dirname, unsigned long entry_count) {
    dir_entry_t dir_entry = { dirname, entry_count };
    dir_entries.push_back(dir_entry);
  }

  void consume() {
    dir_entries_t::reverse_iterator d = dir_entries.rbegin();

    if (d != dir_entries.rend() && !--d->entry_count) {
      dir_entries.pop_back();
    }
  }

  string get_path(const string& filename = "") {
    string path = ".";

    for (dir_entries_t::const_iterator i = dir_entries.begin();
         i != dir_entries.end();
         i++) {
      path += "/" + i->name;
    }

    if (!filename.empty()) {
      path += "/" + filename;
    }

    return path;
  }

private:
  struct dir_entry_t {
    string        name;
    unsigned long entry_count;
  };

  typedef list<dir_entry_t> dir_entries_t;
  dir_entries_t dir_entries;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmpsaf v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.saf>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  SAFHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  SAFENTRY*     entries     = new SAFENTRY[hdr.entry_count];
  unsigned long entries_len = sizeof(SAFENTRY) * hdr.entry_count;
  read(fd, entries, entries_len);
  unobfuscate((unsigned char*) entries, entries_len);
  
  char *filenames = new char[hdr.filenames_length];
  read(fd, filenames, hdr.filenames_length);
  unobfuscate_filenames(filenames, hdr.filenames_length);

  directories_t directories;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    bool  is_directory = entries[i].filename_offset & DIRECTORY_FLAG;
    char* filename     = filenames + (entries[i].filename_offset & ~DIRECTORY_FLAG);
    
    if (is_directory) {
      directories.add(filename, entries[i].original_length);
      mkdir(directories.get_path().c_str());
    } else {
      unsigned long  len           = entries[i].length;
      unsigned char* buff = new unsigned char[len];
      lseek(fd, entries[i].offset * 2048, SEEK_SET);
      read(fd, buff, len);

      if (entries[i].original_length != 0) {
        unsigned long  temp_len  = entries[i].original_length;
        unsigned char* temp_buff = new unsigned char[temp_len];
        inflate_buff(buff, len, temp_buff, temp_len);

        delete [] buff;

        len  = temp_len;
        buff = temp_buff;
      }

      int out_fd = open_or_die(directories.get_path(filename),
                               O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
                               S_IREAD | S_IWRITE);
      write(out_fd, buff, len);
      close(out_fd);

      delete [] buff;

      directories.consume();
    }
  }

  delete [] entries;

  close(fd);

  return 0;
}

