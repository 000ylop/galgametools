// exxarc.cpp, v1.0 2007/08/04
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts XARC (*.xarc) archives used by Xuse's Spirit of Eternity
// Sword 2.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdio>
#include <string>

using std::string;

#pragma pack(1)
struct XARCHDR {
  unsigned char  signature[4]; // "XARC"
  unsigned long  unknown1;
  unsigned long  unknown2;
  unsigned long  unknown3;
  unsigned long  entry_count;
  unsigned short junk;
};

struct XARCDFNMTAG {
  unsigned char  signature[4]; // "DFNM"
  unsigned long  length;       // including file header
  unsigned long  unknown;
  unsigned short junk;
};

struct XARCTAG {
  unsigned char signature[4]; // "NDIX", "EDIX", "CTIF", "CADR"
};

struct XARCDFNMENTRY {
  unsigned short unknown;
  unsigned long  offset;
  unsigned short junk;
};

struct XARCFNENTRY {
  unsigned short unknown1;
  unsigned long  unknown2;
  unsigned short length;
  unsigned short junk;
  unsigned char  filename[1]; // variable length
};

struct XARCCADRENTRY {
  unsigned short unknown1;
  unsigned long  offset;
  unsigned long  unknown2;
  unsigned short junk;  
};

struct XARCDATAHDR {
  unsigned char  signature[4]; // "DATA"
  unsigned char  unknown[20];
  unsigned long  length;
  unsigned short junk;
};
#pragma pack()

void unobfuscate(unsigned char* buff, unsigned long  len) {
  unsigned char* end = buff + len;

  while (buff < end) {
    *buff++ ^= 0x56;
  }
}

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

unsigned long get_file_size(int fd) {
  struct stat file_stat;
  fstat(fd, &file_stat);
  return file_stat.st_size;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exxarc v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.xarc>\n\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  XARCHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  XARCDFNMTAG dfnm;
  read(fd, &dfnm, sizeof(dfnm));

  XARCTAG tag;  

  unsigned long  entries_len  = sizeof(XARCDFNMENTRY) * hdr.entry_count;

  XARCDFNMENTRY* ndix_entries = new XARCDFNMENTRY[hdr.entry_count];
  read(fd, &tag, sizeof(tag));
  read(fd, ndix_entries, entries_len);

  XARCDFNMENTRY* edix_entries = new XARCDFNMENTRY[hdr.entry_count];
  read(fd, &tag, sizeof(tag));
  read(fd, edix_entries, entries_len);

  unsigned long  filenames_offset = sizeof(hdr) + sizeof(dfnm) + sizeof(tag) * 3 + entries_len * 2;
  unsigned long  filenames_len    = dfnm.length - filenames_offset;
  unsigned char* filenames        = new unsigned char[filenames_len];
  read(fd, &tag, sizeof(tag));
  read(fd, filenames, filenames_len);

  XARCCADRENTRY* cadr_entries = new XARCCADRENTRY[hdr.entry_count];
  read(fd, &tag, sizeof(tag));
  read(fd, cadr_entries, sizeof(XARCCADRENTRY) * hdr.entry_count);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    XARCFNENTRY* fn_entry = (XARCFNENTRY*) (filenames + ndix_entries[i].offset - filenames_offset);
    unobfuscate(fn_entry->filename, fn_entry->length);

    string filename((char*) fn_entry->filename, fn_entry->length);

    XARCDATAHDR datahdr;
    lseek(fd, cadr_entries[i].offset, SEEK_SET);
    read(fd, &datahdr, sizeof(datahdr));

    unsigned long  len  = datahdr.length;
    unsigned char* buff = new unsigned char[len];    
    read(fd, buff, len);

    int out_fd = open_or_die(filename,
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  delete [] cadr_entries;
  delete [] edix_entries;
  delete [] ndix_entries;

  close(fd);

  return 0;
}
