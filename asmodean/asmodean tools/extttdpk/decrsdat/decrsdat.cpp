// decrsdat.cpp, v1.0 2010/11/19
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool decrypts PS3 SDAT data.  It must be run on a PS3; the list of source
// and destination files is sepcified in decrsdat.lst.

// Be aware that this is slow and you may be looking at a blank screen with the hdd
// activity light blinking for a while when decrypting large fles.

#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <sys/process.h>
#include <sys/paths.h>

SYS_PROCESS_PARAM(1001, 0x10000)

static const char* FILE_LIST_USB = "/dev_usb/decrsdat.lst";
static const char* FILE_LIST_HDD = "/dev_hdd0/game/ASMO00001/USRDIR/decrsdat.lst";

void decrypt_file(const char* in, const char* out) {
  int fd = -1;
  cellFsSdataOpen(in,
                  CELL_FS_O_RDONLY,
                  &fd,
                  NULL,
                  0);

  if (fd != -1) {
    int out_fd = open(out, 
                      O_CREAT | O_TRUNC | O_WRONLY, 
                      S_IRWXU | S_IRWXG | S_IRWXO);

    if (out_fd != -1) {
      unsigned long  len  = 1024 * 16;
      unsigned char* buff = new unsigned char[len];
      
      while (true) {
        uint64_t actual_len = -1;
        cellFsRead(fd, buff, len, &actual_len);

        write(out_fd, buff, actual_len);

        if (actual_len != len) {
          break;
        }
      }

      delete []  buff;

      close(out_fd);
    }
  }
}

int main(int argc, char **argv) {
  cellSysmoduleLoadModule(CELL_SYSMODULE_FS);

  FILE* fh = fopen(FILE_LIST_USB, "r");

  if (!fh) {
    fh =fopen(FILE_LIST_HDD, "r");
  }

  if (fh) {
    char line[4096] = { 0 };

    while (fgets(line, sizeof(line), fh)) {
      if (line[0] == '#') continue;

      char in[2048]  = { 0 };
      char out[2048] = { 0 };

      if (sscanf(line, "%s %s", in, out) == 2) {
        decrypt_file(in, out);
      }
    }

    fclose(fh);
  }

  return 0;
}
