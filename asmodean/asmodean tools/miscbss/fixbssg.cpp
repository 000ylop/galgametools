// fixbssg.cpp, v1.0 2007/07/05
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool tweaks a header field in multi-frame BSS-Graphics so that older
// tools will be happy...

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstdio>
#include <string>

using std::string;

#pragma pack(1)
struct BSSGRAPHDR {
  unsigned char  signature[16]; // "BSS-Graphics"
  unsigned char  unknown1[16];
  unsigned short offset_x;
  unsigned short offset_y;
  unsigned char  unknown2[4];
  unsigned short width;
  unsigned short height;
  unsigned char  unknown3;
  unsigned char  frame_count;
  unsigned char  unknown4[8];
  unsigned long  length;
  unsigned char  unknown5[6];
};
#pragma pack()

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

string get_file_prefix(const std::string& filename) {
  string temp(filename);

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
  if (argc != 2) {
    fprintf(stderr, "fixbssg v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bsg>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  

  int fd = open_or_die(in_filename, O_RDWR | O_BINARY);

  BSSGRAPHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  if (memcmp(hdr.signature, "BSS-Graphics", 12)) {
    printf("%s: not a BSS-Graphics\n", in_filename.c_str());
    return 0;
  }

  if (hdr.frame_count != 1) {
    hdr.frame_count = 1;

    lseek(fd, 0, SEEK_SET);
    write(fd, &hdr, sizeof(hdr));
  } else {
    printf("%s: doesn't need fixed\n", in_filename.c_str());
  }

  close(fd);

  return 0;
}

