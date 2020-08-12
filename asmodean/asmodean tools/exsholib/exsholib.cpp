// exsholib.cpp, v1.0 2007/08/13
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Extracts *.lib archives used by light's Shopan.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstdio>
#include <string>

using std::string;

struct LIBUHDR {
  unsigned char signature[4]; // "LIBU"
  unsigned long unknown1;
  unsigned long entry_count;
  unsigned long unknown2;
};

struct LIBUENTRY {
  wchar_t       filename[32 + 1];
  unsigned long length;
  unsigned long offset;   // from top of header
  unsigned long unknown2;
};

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

string convert_filename(wchar_t* s) {
  char filename[4096] = { 0 };
  wcstombs(filename, s, sizeof(filename));
  return filename;
}

string get_prefix(const string& s) {
  string temp = s;

  string::size_type pos = temp.find_last_of(".");
  if (pos != string::npos) {
    temp = temp.substr(0, pos);
  }

  return temp;
}

void extract(int           fd, 
             const string& prefix,
             unsigned long offset,
             unsigned long length,
             bool          flatten)
{
  char dir_delim = flatten && prefix != "." ? '+' : '/';

  LIBUHDR hdr;
  lseek(fd, offset, SEEK_SET);
  read(fd, &hdr, sizeof(hdr));

  if (memcmp(hdr.signature, "LIBU", 4) == 0) {
    LIBUENTRY* entries = new LIBUENTRY[hdr.entry_count];
    read(fd, entries, sizeof(LIBUENTRY) * hdr.entry_count);

    if (!flatten) {
      mkdir(prefix.c_str());
    }

    for (unsigned long i = 0; i < hdr.entry_count; i++) {     
      extract(fd, 
              prefix + dir_delim + convert_filename(entries[i].filename),
              offset + entries[i].offset,
              entries[i].length,
              flatten);
    }

    delete [] entries;
  } else {
    unsigned char* buff = new unsigned char[length];
    lseek(fd, offset, SEEK_SET);
    read(fd, buff, length);

    bool is_mgf = length >= 7 && !memcmp(buff, "MalieGF\0", 8);

    string filename(prefix);
    if (is_mgf) {
      filename = get_prefix(prefix) + ".png";
      memcpy(buff, "\x89PNG\x0D\x0A\x1A\x0A", 8);
    }

    int out_fd = open_or_die(filename,
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);
    write(out_fd, buff, length);
    close(out_fd);

    delete [] buff;
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "exsholib v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.lib> [-flatten]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  
  bool   flatten = argc > 2 && !strcmp(argv[2], "-flatten");

  int fd = open_or_die(in_filename, O_RDWR | O_BINARY);
  extract(fd, ".", 0, 0, flatten);
  close(fd);

  return 0;
}
