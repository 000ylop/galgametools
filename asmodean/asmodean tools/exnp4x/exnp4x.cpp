// exnp4x.cpp, v1.02 2007/10/13
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from NEKOPACK4A and NEKOPACK4S archives.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstdio>
#include <string>
#include "zlib.h"

using std::string;

#pragma pack(1)
struct NP4XHDR {
  unsigned char signature[8]; // NEKOPACK
  unsigned char type[2];      // "4A" or "4S"
};

struct NP4XENTRY {
  unsigned long offset;
  unsigned long length;
};
#pragma pack()

void unobfuscate_entry(NP4XENTRY&     entry, 
                       char*          buff, 
                       unsigned long  len)
{
  unsigned long key = 0;

  for (unsigned long i = 0; i < len; i++) {
    key += buff[i];
  }

  entry.offset ^= key;
  entry.length ^= key;
}

void unobfuscate(unsigned char* buff, unsigned long len) {
  unsigned char key = (unsigned char) (len >> 3) + 0x22;

  for (unsigned long i = 0; i < 32; i++) {
    buff[i] ^= key;
    key <<= 3;
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

string get_path_prefix(const string& filename) {
  string temp;

  string::size_type pos = filename.find_last_of("/\\");
  if (pos) {
    temp = filename.substr(0, pos + 1);
  }

  return temp;
}

void make_path(const string& filename) {
  char temp[4096] = { 0 };
  strcpy(temp, filename.c_str());

  for (unsigned long i = 0; i < filename.length(); i++) {
    if (temp[i] == '\\' || temp[i] == '/') {
      char t  = temp[i];
      temp[i] = '\0';
      mkdir(temp);
      temp[i] = t;
    }
  }
}

unsigned long get_file_size(int fd) {
  struct stat file_stat;
  fstat(fd, &file_stat);
  return file_stat.st_size;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exnp4x v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  
  string path_prefix = get_path_prefix(in_filename);

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  NP4XHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = 0;  

  if (!memcmp(hdr.type, "4A", 2)) {
    read(fd, &toc_len, sizeof(toc_len));
  } else if (!memcmp(hdr.type, "4S", 2)) {
    unsigned long name_len;
    read(fd, &name_len, sizeof(name_len));
    lseek(fd, -(int)sizeof(name_len), SEEK_CUR);

    toc_len = sizeof(name_len) + name_len + sizeof(NP4XENTRY);
  } else {
    fprintf(stderr, "%s: unknown type (%s)\n", in_filename.c_str(), hdr.type);
    exit(-1);
  }

  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);

  unsigned char* p       = toc_buff;
  unsigned char* toc_end = toc_buff + toc_len;

  while (p < toc_end) {
    unsigned long filename_len = *(unsigned long*) p;
    p += sizeof(filename_len);

    if (!filename_len) {
      break;
    }

    char* filename = (char*) p;
    p += filename_len;

    NP4XENTRY* entry = (NP4XENTRY*) p;
    p += sizeof(*entry);
    unobfuscate_entry(*entry, filename, filename_len);

    unsigned long  len  = entry->length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entry->offset, SEEK_SET);
    read(fd, buff, len);
    unobfuscate(buff, len);

    // original length is at the end of the data..
    unsigned long  out_len    = *(unsigned long*) (buff + len - 4);
    unsigned char* out_buff   = new unsigned char[out_len];
    uncompress(out_buff, &out_len, buff, len - 4);

    string full_name = path_prefix + filename;

    make_path(full_name);

    int out_fd = open_or_die(full_name,
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);
    write(out_fd, out_buff, out_len);
    close(out_fd);

    delete [] out_buff;
    delete [] buff;
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}
