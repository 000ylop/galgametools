// exkactive.cpp, v1.0 2006/11/04
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)
//   icq:   55244079

// This tool extracts ACPACK32 (A98*.PAK) archives used by Kururi Active

#include <io.h>       // open() etc on windows
#include <sys/stat.h> // S_IREAD flags
#include <fcntl.h>    // O_RDONLY flags
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>

struct PAKHDR {
  unsigned char signature[8]; // "ACPACK32"
  unsigned long unknown;
  unsigned long entry_count;
};

struct PAKENTRY {
  char          filename[28];
  unsigned long offset;
};

using std::string;

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "exkactive, v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak> [-rename]\n\n", argv[0]);
    fprintf(stderr, "The -rename flag causes EDT, ED8 and SAL image names to be disambiguated.\n");
    return -1;
  }

  bool rename_graphics = argc > 2 && !strncmp(argv[2], "-rename", 7);

  char* in_filename = argv[1];

  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  PAKHDR hdr;

  read(fd, &hdr, sizeof(hdr));

  PAKENTRY* entries = new PAKENTRY[hdr.entry_count];

  read(fd, entries, sizeof(PAKENTRY) * hdr.entry_count);

  // Last entry is bogus
  for (unsigned long i = 0; i < hdr.entry_count - 1; i++) {
    unsigned long  len  = entries[i + 1].offset - entries[i].offset;
    unsigned char* buff = new unsigned char[len];

    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    string out_filename(entries[i].filename);

    if (rename_graphics) {
      string ext;

      string::size_type pos = out_filename.find_last_of(".");

      if (pos != string::npos) {
        ext = out_filename.substr(pos + 1);

        // Idiot std::string doesn't have case-neutral comparison OR a make_lower etc method
        if (!stricmp(ext.c_str(), "edt") ||
            !stricmp(ext.c_str(), "ed8") ||
            !stricmp(ext.c_str(), "sal")) {
          out_filename = out_filename.substr(0, pos) + "_" + ext + "." + ext;
        }
      }
    }

    int out_fd = open(out_filename.c_str(), O_CREAT | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE);

    if (out_fd == -1) {
      fprintf(stderr, "Could not open %s (%s)\n", out_filename.c_str(), strerror(errno));
      return -1;
    }

    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
