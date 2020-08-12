// pmchar2bmp.cpp, v1.0 2007/07/08
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool converts PMchar (*.char) images to bitmaps with alpha intact.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstdio>
#include <string>

using std::string;

struct PMCHARHDR {
  unsigned long unknown1;
  unsigned char sig1[4]; // OVFL
  unsigned char unknown2[2];
  unsigned char sig2[6]; // "PMchar"
  unsigned long length;
  unsigned char unknown3[28];
};

static const unsigned long PMCHAR_WIDTH  = 800;
static const unsigned long PMCHAR_HEIGHT = 600;

unsigned long unrle(unsigned char* buff, 
                    unsigned long  len,
                    unsigned char* out_buff,
                    unsigned long  out_len)
{
  unsigned char* end     = buff + len;
  unsigned char* out_end = out_buff + out_len;

  unsigned long remain = PMCHAR_WIDTH;

  memset(out_buff, 0, out_len);

  while (buff < end) {
    unsigned char c = *buff++;

    if (c == 0xFF) {
      out_buff += remain * 4;
      remain    = PMCHAR_WIDTH;
    } else if (c >= 0x9F) {
      c        -= 0x9E; 
      out_buff += c * 4;
      remain   -= c;
    } else if (c >= 0x7F) {
      c      -= 0x7E;
      remain -= c;

      while (c--) {
        *out_buff++ = buff[0];
        *out_buff++ = buff[1];
        *out_buff++ = buff[2];
        *out_buff++ = 0xFF;
      }

      buff += 3;
    } else {
      *out_buff++ = buff[0];
      *out_buff++ = buff[1];
      *out_buff++ = buff[2];
      *out_buff++ = c * 2;

      buff   += 3;
      remain -= 1;
    }
  }

  return out_len - (out_end - out_buff);
}

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
  if (argc < 2) {
    fprintf(stderr, "pmchar2bmp v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.char> [output.bmp]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  
  string out_filename = get_file_prefix(in_filename) + ".bmp";

  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  PMCHARHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = hdr.length;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  unsigned long  out_len  = PMCHAR_WIDTH * PMCHAR_HEIGHT * 8;
  unsigned char* out_buff = new unsigned char[out_len];

  unrle(buff, len, out_buff, out_len);

  fd = open_or_die(out_filename,
                   O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
                   S_IREAD | S_IWRITE);

  {
    BITMAPFILEHEADER bmf;
    BITMAPINFOHEADER bmi;

    memset(&bmf, 0, sizeof(bmf));
    memset(&bmi, 0, sizeof(bmi));

    bmf.bfType     = 0x4D42;
    bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + out_len;
    bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi);

    bmi.biSize     = sizeof(bmi);
    bmi.biWidth    = PMCHAR_WIDTH;
    bmi.biHeight   = PMCHAR_HEIGHT;
    bmi.biPlanes   = 1;
    bmi.biBitCount = 32;
   
    write(fd, &bmf, sizeof(bmf));
    write(fd, &bmi, sizeof(bmi));
  }

  write(fd, out_buff, out_len);
  close(fd);

  delete [] buff;

  return 0;
}

