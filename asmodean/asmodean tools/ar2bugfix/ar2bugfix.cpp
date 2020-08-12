// ar2bugfix.cpp, v1.01 2009/01/30
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool fixes a crashing bug encountered when fighting Raki in the
// USA release of Ar Tonelico 2 (SLUS_217.88).

// This is provided as a scanner-patcher rather than xdelta so that it
// will be compatible with various targets.

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <io.h>
#include <windows.h>

static const unsigned char MATCH_DATA[] = { 0x90, 0x58, 0x93, 0x00, 0x90, 0x58, 0x93, 0x00, 
                                            0x02, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 
                                            0xFF, 0xFF, 0xFF, 0xFF, 0x05, 0xFF, 0xFF, 0x00, 
                                            0x09, 0x00, 0x00, 0x00, 0xC8, 0x01, 0x93, 0x00, 
                                            0xC8, 0x01, 0x93, 0x00, 0xC8, 0x01, 0x93, 0x00 };

static const unsigned char PATCH_DATA[] = { 0xA0, 0x48, 0x93, 0x00, 0xA0, 0x48, 0x93, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x37, 0x00, 
                                            0x1F, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00,
                                            0x08, 0x00, 0x00, 0x00, 0xC8, 0x01, 0x93, 0x00, 
                                            0xC8, 0x01, 0x93, 0x00, 0xC0, 0x48, 0x93, 0x00 };

// Stick around long enough for the user to read any messages.
void delay_exit(int rc) {
  Sleep(5000);
  exit(rc);
}

int main(int argc, char** argv) {
  printf("ar2bugfix v1.01, coded by asmodean (http://asmodean.reverse.net)\n\n");

  if (argc < 2) {    
    printf("usage: %s <input>\n\n", argv[0]);
    printf("NOTE: input will be modified!\n");
    delay_exit(-1);
  }

  int fd = open(argv[1], O_RDWR | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", argv[1], strerror(errno));
    delay_exit(-1);
  }

  long          match_len   = sizeof(MATCH_DATA);
  bool          done        = false;
  unsigned char buff[10240] = { 0 };

  while (!done) {
    __int64 offset = _telli64(fd);
    long    len    = read(fd, buff, sizeof(buff));

    if (len < match_len) {
      break;
    }

    for  (long i = 0; i < len - match_len; i++) {
      if (!memcmp(buff + i, MATCH_DATA, match_len)) {
        offset += i + match_len;

        _lseeki64(fd, offset, SEEK_SET);
        write(fd, PATCH_DATA, sizeof(PATCH_DATA)); 

        printf("Patched at offset 0x%08I64X\n", offset);

        done = true;
        break;
      }
    }

    _lseeki64(fd, -match_len + 1, SEEK_CUR);
  }

  close(fd);

  if (!done) {
    printf("Didn't find patch location; is this Ar Tonelico 2 USA (SLUS_217.88)?\n");
  }

  delay_exit(0);
}