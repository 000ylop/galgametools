// gax2png.cpp, v1.01 2011/08/30
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool decrypts various data used by ãsèPÇS.

#include "as-util.h"

static const unsigned long KEY_LEN = 16;

struct GAXHDR {
  unsigned long type;
  unsigned char key[KEY_LEN];
};

void unobfuscate(unsigned char* buff,
                 unsigned long  len,
                 unsigned char* key)
{
  unsigned char* end = buff + len;

  while (buff < end) {
    for (unsigned long i = 0; i < 16 && buff < end; i++) {
      *buff++ ^= key[i];
    }

    unsigned char mutator = *(buff - 2);

    switch (mutator & 7) {
    case 0:
      key[0]  = key[0] + mutator;
      key[3]  = key[3] + mutator + 2;
      key[4]  = key[2] + mutator + 11;
      key[8]  = key[6] + 7;
      break;
    case 1:
      key[2]  = key[9] + key[10];
      key[6]  = key[7] + key[15];
      key[8]  = key[8] + key[1];
      key[15] = key[5] + key[3];
      break;
    case 2:
      key[1]  = key[1]  + key[2];
      key[5]  = key[5]  + key[6];
      key[7]  = key[7]  + key[8];
      key[10] = key[10] + key[11];
      break;
    case 3:
      key[9]  = key[2]  + key[1];
      key[11] = key[6]  + key[5];
      key[12] = key[8]  + key[7];
      key[13] = key[11] + key[10];
      break;
    case 4:
      key[0]  = key[1]  + 111;
      key[3]  = key[4]  + 71;
      key[4]  = key[5]  + 17;
      key[14] = key[15] + 64;
      break;
    case 5:
      key[2]  = key[2]  + key[10];
      key[4]  = key[5]  + key[12];
      key[6]  = key[8]  + key[14];
      key[8]  = key[11] + key[0];
      break;
    case 6:
      key[9]  = key[11] + key[1];
      key[11] = key[13] + key[3];
      key[13] = key[15] + key[5];
      key[15] = key[9]  + key[7];
      // Fall through!
    default:
      key[1]  = key[9]  + key[5];
      key[2]  = key[10] + key[6];
      key[3]  = key[11] + key[7];
      key[4]  = key[12] + key[8];
      break;
    }
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "gax2png v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.gax> [output.png]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename;

  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  GAXHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = as::get_file_size(fd) - sizeof(hdr);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  if (hdr.type == 0x01000000) {
    unobfuscate(buff, len, hdr.key);
  }

  if (out_filename.empty()) {
    out_filename = as::get_file_prefix(in_filename) + as::guess_file_extension(buff, len);
  }

  as::write_file(out_filename, buff, len);

  delete [] buff;

  close(fd);

  return 0;
}
