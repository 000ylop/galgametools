// pkxarc.cpp, v1.01 2007/12/19
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool repacks data into XARC (*.xarc) archives.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdio>
#include <string>
#include <algorithm>

using std::string;
using std::max;

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
  unsigned short crc;
};

struct XARCTAG {
  unsigned char signature[4]; // "NDIX", "EDIX", "CTIF", "CADR"
};

struct XARCDFNMENTRY {
  unsigned short unknown;
  unsigned long  offset;
  unsigned short crc;
};

struct XARCFNENTRY {
  unsigned short unknown1;
  unsigned long  unknown2;
  unsigned short length;
  unsigned short crc;
  unsigned char  filename[1]; // variable length
};

struct XARCCADRENTRY {
  unsigned short unknown1;
  unsigned long  offset;
  unsigned long  unknown2;
  unsigned short crc;  
};

struct XARCDATAHDR {
  unsigned char  signature[4]; // "DATA"
  unsigned char  unknown[20];
  unsigned long  length;
  unsigned short crc;
};
#pragma pack()

// Originally I wrote this before I realized it was just a CRC, but that
// turns out convenient to have now :)
unsigned short crc16(unsigned char* buff, unsigned long len) { 
  unsigned short work1 = 0;

  for (unsigned long i = 0; i < len; i++) {
    work1 ^= ((unsigned short)buff[i]) << 8;  

    for (unsigned long i = 0; i < 8; i++) {
      unsigned short work2 = (unsigned short) work1;
      
      _asm {
        sar work2, 0x0F;
      }

      work2 &= 0x1021;

      work1 += work1;
      work1 ^= work2;
    }
  }

  return (work1 << 8) | (work1 >> 8);
}


void unobfuscate(unsigned char* buff, unsigned long  len) {
  unsigned char* end = buff + len;

  while (buff < end) {
    *buff++ ^= 0x56;
  }
}

void read_check_crc16(int fd, unsigned char* buff, unsigned long len) {
  read(fd, buff, len);

  unsigned short old_crc = 0;
  read(fd, (unsigned char*) old_crc, sizeof(old_crc));

  if (crc16(buff, len) != old_crc) {
    fprintf(stderr, "read_check_crc16() mismatch\n");
    exit(-1);
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
  if (argc != 3) {
    fprintf(stderr, "pkxarc v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.xarc> <output.xarc>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string out_filename(argv[2]);

  int fd     = open_or_die(in_filename, O_RDONLY | O_BINARY);
  int out_fd = open_or_die(out_filename,
                           O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                           S_IREAD | S_IWRITE);

  XARCHDR hdr;
  read(fd, &hdr, sizeof(hdr));
  write(out_fd, &hdr, sizeof(hdr));

  XARCDFNMTAG dfnm;
  read(fd, &dfnm, sizeof(dfnm));
  write(out_fd, &dfnm, sizeof(dfnm));

  XARCTAG tag;  

  unsigned long  entries_len  = sizeof(XARCDFNMENTRY) * hdr.entry_count;

  XARCDFNMENTRY* ndix_entries = new XARCDFNMENTRY[hdr.entry_count];
  read(fd, &tag, sizeof(tag));
  read(fd, ndix_entries, entries_len);
  write(out_fd, &tag, sizeof(tag));
  write(out_fd, ndix_entries, entries_len);  

  XARCDFNMENTRY* edix_entries = new XARCDFNMENTRY[hdr.entry_count];
  read(fd, &tag, sizeof(tag));
  read(fd, edix_entries, entries_len);
  write(out_fd, &tag, sizeof(tag));
  write(out_fd, edix_entries, entries_len);

  unsigned long  filenames_offset = sizeof(hdr) + sizeof(dfnm) + sizeof(tag) * 3 + entries_len * 2;
  unsigned long  filenames_len    = dfnm.length - filenames_offset;
  unsigned char* filenames        = new unsigned char[filenames_len];
  read(fd, &tag, sizeof(tag));
  read(fd, filenames, filenames_len);
  write(out_fd, &tag, sizeof(tag));
  write(out_fd, filenames, filenames_len);

  unsigned long  cadr_len     = sizeof(XARCCADRENTRY) * hdr.entry_count;
  XARCCADRENTRY* cadr_entries = new XARCCADRENTRY[hdr.entry_count];
  read(fd, &tag, sizeof(tag));  
  read(fd, cadr_entries, cadr_len);
  write(out_fd, &tag, sizeof(tag));  
  write(out_fd, cadr_entries, cadr_len);

  unsigned long out_offset  = tell(out_fd);
  unsigned long cadr_offset = out_offset - cadr_len;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    XARCFNENTRY* fn_entry = (XARCFNENTRY*) (filenames + ndix_entries[i].offset - filenames_offset);
    unobfuscate(fn_entry->filename, fn_entry->length);

    string filename((char*) fn_entry->filename, fn_entry->length);

    XARCDATAHDR datahdr;
    lseek(fd, cadr_entries[i].offset, SEEK_SET);
    read(fd, &datahdr, sizeof(datahdr));

    int            new_fd = open_or_die(filename, O_RDONLY | O_BINARY);
    unsigned long  len    = get_file_size(new_fd);
    unsigned char* buff   = new unsigned char[len];    
    read(new_fd, buff, len);
    close(new_fd);

    datahdr.length = len;
    datahdr.crc    = crc16((unsigned char*)&datahdr, sizeof(datahdr) - 2);

    cadr_entries[i].offset = out_offset;
    cadr_entries[i].crc    = crc16((unsigned char*)&cadr_entries[i], sizeof(cadr_entries[i]) - 2);

    write(out_fd, &datahdr, sizeof(datahdr));
    write(out_fd, buff, len);

    unsigned short new_crc = crc16(buff, len);
    write(out_fd, &new_crc, sizeof(new_crc));
    
    out_offset += sizeof(datahdr) + len + sizeof(new_crc);

    delete [] buff;
  }

  // Write updated TOC information
  lseek(out_fd, cadr_offset, SEEK_SET);
  write(out_fd, cadr_entries, cadr_len);

  delete [] cadr_entries;
  delete [] edix_entries;
  delete [] ndix_entries;

  close(out_fd);
  close(fd);

  return 0;
}
