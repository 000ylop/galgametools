// exbssc.cpp, v1.0 2007/07/05
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts individual frames from BSS-Composition (*.bsg) composites.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstdio>
#include <string>

using std::string;

#pragma pack(1)
struct BSSCOMPHDR {
  unsigned char  signature[16]; // "BSS-Composition"
  unsigned char  unknown1;
  unsigned long  entry_count;
  unsigned char  unknown2;
  unsigned short base_width;
  unsigned short base_height;
  unsigned char  unknown3[6];
};

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
    fprintf(stderr, "exbssc v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bsg>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  
  string prefix = get_file_prefix(in_filename);

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  BSSCOMPHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  if (memcmp(hdr.signature, "BSS-Composition", 15)) {
    printf("%s: not a BSS-Composition\n", in_filename.c_str());
    return 0;
  }

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    BSSGRAPHDR graphdr;
    read(fd, &graphdr, sizeof(graphdr));

    // Some BSG tools require this value
    graphdr.frame_count = 1;

    unsigned long  len  = graphdr.length;
    unsigned char* buff = new unsigned char[len];
    read(fd, buff, len);

    char filename[4096] = { 0 };
    sprintf(filename, "%s+%05d_%dx_%dy.bsg", prefix.c_str(), i, graphdr.offset_x, graphdr.offset_y);

    int out_fd = open_or_die(filename,
                             O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
                             S_IREAD | S_IWRITE);
    write(out_fd, &graphdr, sizeof(graphdr));
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  close(fd);

  return 0;
}

