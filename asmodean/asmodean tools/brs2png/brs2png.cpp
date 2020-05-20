// brs2png.cpp, v1.01 2007/03/23
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool de-obfuscates *.BRS (EENC) images to PNG images.

#include <io.h>       // open() etc on windows
#include <sys/stat.h> // S_IREAD flags
#include <fcntl.h>    // O_RDONLY flags
#include <cerrno>
#include <cstdio>
#include <string>

struct BRSHDR {
  unsigned char signature[4]; // "EENC"
  unsigned long length;
};

static const unsigned long PNG_SIG = 0x474E5089;

using std::string;

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "brs2png, v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.brs> [output.png]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename;

  if (argc > 2)
    out_filename = argv[2];

  int fd = open(in_filename.c_str(), O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename.c_str(), strerror(errno));
    return -1;
  }

  BRSHDR hdr;

  read(fd, &hdr, sizeof(hdr));

  if (memcmp(hdr.signature, "EENC", 4)) {
    fprintf(stderr, "%s: not an EENC (may be raw)\n", in_filename.c_str());
    return 0;
  }

  // Allocate the buffer with a little slack to always allow 4-byte xor
  unsigned long  len  = hdr.length + 4;
  unsigned char* buff = new unsigned char[len];

  read(fd, buff, hdr.length);
  close(fd);

  {
    unsigned long* p   = (unsigned long*) buff;
    unsigned long  key = *p ^ PNG_SIG;

    for (unsigned long i = 0; i < hdr.length; i += 4) {
      *p++ ^= key;
    }
  }  

  if (out_filename.empty()) {
    out_filename = in_filename; 

    string::size_type pos = out_filename.find_last_of(".");

    if (pos != string::npos) {
      out_filename = out_filename.substr(0, pos);
    }

    out_filename += ".png";
  }

  fd = open(out_filename.c_str(), O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", out_filename.c_str(), strerror(errno));
    return -1;
  }

  write(fd, buff, hdr.length);
  close(fd);

  delete [] buff;

  return 0;
}
