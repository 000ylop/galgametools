// sigren.cpp, v1.0 2007/01/06
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool renames files to the correct extension based on their signature.
// Supports PNG, JPEG, Windows/OS2 bitmaps, GIF, Photoshop images, and OGG
// audio.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>

using std::string;

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "sigren v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input>\n", argv[0]);
    return -1;
  }

  char* in_filename  = argv[1];

  string prefix(in_filename);
  {
    string::size_type pos = prefix.find_last_of(".");

    if (pos != string::npos) {
      prefix = prefix.substr(0, pos);
    }
  }

  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  char sigcheck[64] = { 0 };
  read(fd, sigcheck, sizeof(sigcheck));
  close(fd);

  string ext;

  if (!memcmp(sigcheck, "\x89PNG", 4)) {
    ext = "png";
  } else if (!memcmp(sigcheck + 6, "JFIF", 4)) {
    ext = "jpg";
  } else if (!memcmp(sigcheck, "BM", 2)) {
    ext = "bmp";
  } else if (!memcmp(sigcheck, "GIF", 3)) {
    ext = "gif";
  } else if (!memcmp(sigcheck, "8BPS", 4)) {
    ext = "psd";
  } else if (!memcmp(sigcheck, "OggS", 4)) {
    ext = "ogg";
  }

  if (!ext.empty()) {
    char filename[4096] = { 0 };
    sprintf(filename, "%s.%s", prefix.c_str(), ext.c_str());
    rename(in_filename, filename);
  } else {
    fprintf(stderr, "%s: don't know what this is.\n", in_filename);
  }

  return 0;
}
