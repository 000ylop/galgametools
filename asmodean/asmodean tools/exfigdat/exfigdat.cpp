// exfigdat.cpp, v1.0 2007/04/06
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from FILECMB-DATA-LIST-IN (*.dat) archives used
// by Figurehead and others...

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>

using std::string;

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exfigdat v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  // Open the file twice, once for random access and the other
  // for sequential line-based I/O.
  int   fd = open_or_die(in_filename, O_RDONLY | O_BINARY);
  FILE* fh = fopen(in_filename.c_str(), "r");

  // Skip start crud
  char line[4096] = { 0 };
  fgets(line, sizeof(line), fh);

  while (true) {  
    fgets(line, sizeof(line), fh);

    char          filename[4096] = { 0 };
    unsigned long start_offset   = 0;
    unsigned long end_offset     = 0;

    // Let's not rely on the particular signature for end of list
    if (sscanf(line, "%s %lu %lu", filename, &start_offset, &end_offset) != 3) {
      break;
    }

    unsigned long  len  = end_offset - start_offset;
    unsigned char* buff = new unsigned char[len];

    lseek(fd, start_offset, SEEK_SET);
    read(fd, buff, len);

    int out_fd = open_or_die(filename, 
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 
                             S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  fclose(fh);
  close(fd);

  return 0;
}
