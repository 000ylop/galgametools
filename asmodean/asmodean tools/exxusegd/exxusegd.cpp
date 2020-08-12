// exxusegd.cpp, v1.0 2007/08/05
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.gd+*.dll archives used by Xuse's Eien no Aselia CS.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdio>
#include <string>

using std::string;

struct DLLHDR {
  unsigned long entry_count;
};

struct DLLENTRY {
  unsigned long offset;
  unsigned long length;
};

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

unsigned long get_file_size(int fd) {
  struct stat file_stat;
  fstat(fd, &file_stat);
  return file_stat.st_size;
}

string get_file_prefix(const string& s) {
  string temp = s;

  string::size_type pos = temp.find_last_of(".");
  if (pos != string::npos) {
    temp = temp.substr(0, pos);
  }

  pos = temp.find_last_of("/\\");
  if (pos != string::npos) {
    temp = temp.substr(pos + 1);
  }

  return temp;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "exxusegd v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.gd> <input.dll>\n", argv[0]);
    return -1;
  }

  string gd_filename(argv[1]);
  string dll_filename(argv[2]);
  string prefix(get_file_prefix(gd_filename));

  int fd = open_or_die(dll_filename, O_RDONLY | O_BINARY);

  DLLHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  // For some reason the entry count in the header is sometimes wrong
  unsigned long entries_len = get_file_size(fd) - sizeof(hdr);
  unsigned long entry_count = entries_len / sizeof(DLLENTRY);
  DLLENTRY*     entries     = new DLLENTRY[entry_count];
  read(fd, entries, entries_len);
  close(fd);

  fd = open_or_die(gd_filename, O_RDONLY | O_BINARY);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];    
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    string ext;
    if (!memcmp(buff, "\x89PNG", 4)) {
      ext = ".png";
    } else if (!memcmp(buff, "BM", 2)) {
      ext = ".bmp";
    } else if (!memcmp(buff, "OggS", 4)) {
      ext = ".ogg";
    }

    char filename[4096] = { 0 };
    sprintf(filename, "%s_%05d%s", prefix.c_str(), i, ext.c_str());

    int out_fd = open_or_die(filename,
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  close(fd);

  delete [] entries;

  return 0;
}
