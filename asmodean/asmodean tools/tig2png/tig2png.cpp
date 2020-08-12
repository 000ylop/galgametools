// tig2bmp.cpp, v1.0 2007/07/29
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool deobfuscates Gesen 18's *.tig data.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdio>
#include <string>

using std::string;

void unobfuscate(unsigned char* buff, unsigned long len) {
  unsigned long  key = 0x7F7F7F7F;
  unsigned char* end  = buff + len;

  while (buff < end) {
    key *= 0x343FD;
    key += 0x269EC3;

    *buff++ -= (unsigned char) ((key >> 16) & 0xFF);
  }
}

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

string get_file_prefix(const std::string& filename) {
  string temp(filename);

  string::size_type pos = temp.find_last_of(".");
  if (pos != string::npos) {
    temp = temp.substr(0, pos);
  }

  return temp;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "tig2png v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.tig> [output.png]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  
  string out_filename = get_file_prefix(in_filename) + ".png";

  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  unsigned long  len  = get_file_size(fd);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  unobfuscate(buff, len);

  fd = open_or_die(out_filename,
                   O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                   S_IREAD | S_IWRITE);
  write(fd, buff, len);
  close(fd);

  return 0;
}
