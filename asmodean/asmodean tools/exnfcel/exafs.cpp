// exafs.cpp, v1.0 2007/01/18
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts files from CRI's AFS (*.AFS) archives.  Duplicate file
// names are disambiguated by appending .001 etc to the filenames in the order
// the appear in the TOC.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <set>

#include "as-util.h"

//#define INDEX_ONLY

struct AFSHDR {
  unsigned char signature[4]; // "AFS\0"
  unsigned long entry_count;
};

struct AFSENTRY {
  unsigned long offset;
  unsigned long length;
};

struct AFSFENTRY {
  char          name[32];
  unsigned char date_junk[12];
  unsigned long length;
};

struct compare_nocase_t {
  bool operator()(const std::string& a, const std::string& b) const {
    return stricmp(a.c_str(), b.c_str()) < 0;
  }
};

typedef std::set<std::string, compare_nocase_t> used_filenames_t;

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exafs v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.afs>\n", argv[0]);
    return -1;
  }

  char* in_filename = argv[1];
  
  int fd = open(in_filename, O_RDONLY | O_BINARY);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", in_filename, strerror(errno));
    return -1;
  }

  AFSHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  AFSENTRY* entries = new AFSENTRY[hdr.entry_count];
  read(fd, entries, sizeof(AFSENTRY) * hdr.entry_count);

  AFSENTRY info_entry;
  read(fd, &info_entry, sizeof(info_entry));

  AFSFENTRY* fentries = new AFSFENTRY[hdr.entry_count];
  lseek(fd, info_entry.offset, SEEK_SET);
  read(fd, fentries, sizeof(AFSFENTRY) * hdr.entry_count);

  used_filenames_t used_filenames;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    if (entries[i].length == 0) {
      printf("Skipping null entry %d\n", i);
      continue;
    }

#ifdef INDEX_ONLY
    printf("%s:%d [%s]\n", in_filename, i, fentries[i].name);
#else
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];

    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    char filename[4096] = { 0 };
    strcpy(filename, fentries[i].name);

    // Annoying duplicate filenames in some archives...
    for(unsigned int j = 1; used_filenames.find(filename) != used_filenames.end(); j++) {
      sprintf(filename, "%s.%03d", fentries[i].name, j);
    }

    used_filenames.insert(filename);

    char filename_with_index[4096] = { 0 };
    sprintf(filename_with_index, "%s", filename);
    //sprintf(filename_with_index, "%05d+%s", i, filename);
    //sprintf(filename_with_index, "%05d", i, filename);

    as::make_path(filename_with_index);
    int out_fd = open(filename_with_index, 
                      O_CREAT | O_EXCL | O_WRONLY | O_BINARY,
                      S_IREAD | S_IWRITE);

    if (out_fd == -1) {
      fprintf(stderr, "Could not open %s (%s)\n", filename_with_index, strerror(errno));
      exit(-1);
    }

    write(out_fd, buff, len);
    close(out_fd);    

    delete [] buff;
#endif
  }

  close(fd);

  return 0;
}
