// exarcw.cpp, v1.0 2007/05/17
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from ARCW (*.arc) archives.

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>
#include "lzss.h"

using std::string;

struct ARCWHDR {
  unsigned char signature[4]; // "ARCW"
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long data_offset;
};

struct ARCWDATAHDR {
  unsigned long length;
  unsigned long type;
  unsigned long original_length;
};

struct ARCWTOCHDR1 {
  unsigned long entry_count;
};

struct ARCWTOCHDR2 {
  unsigned long type;
  unsigned long unknown;
  unsigned long entry_count;
  unsigned long toc_length;
};

// Used by generic TOC
struct ARCWTOCENTRY {
  unsigned long offset;
  unsigned long length;
  unsigned long type;
  unsigned long original_length;
};

// Used by DDS TOC
struct ARCWTOCENTRY2 {
  unsigned long offset;
  unsigned long length;
  unsigned long type;
  unsigned long original_length;
  unsigned long info_index;
};

// More special stuff for the DDS TOC
struct ARCWDDSTOC {
  unsigned long entry_count;
};

struct ARCWDDSENTRY {
  unsigned long unknown;
  unsigned long width;
  unsigned long height;
};

void unobfuscate(unsigned char* buff, unsigned long len) {
  unsigned char* end = buff + len;

  while (buff < end) {
    *buff++ ^= 0xFF;
  }
}

void read_uncompress(int             fd, 
                     unsigned long   offset,
                     unsigned long   len,
                     unsigned char*& out_buff, 
                     unsigned long   out_len) 
{
  unsigned char* buff = new unsigned char[len];
  lseek(fd, offset, SEEK_SET);
  read(fd, buff, len);
  unobfuscate(buff, len);

  unlzss(buff, len, out_buff, out_len);    

  delete [] buff;
}

void read_hdr_chunk(int fd, unsigned char*& out_buff, unsigned long& out_len) {
  ARCWDATAHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = hdr.length - sizeof(hdr);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  unobfuscate(buff, len);

  out_len  = hdr.original_length;
  out_buff = new unsigned char[out_len];

  unlzss(buff, len, out_buff, out_len);

  delete [] buff;
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

string get_file_prefix(const std::string& filename) {
  string temp(filename);

  string::size_type pos = temp.find_last_of(".");

  if (pos != string::npos) {
    temp = temp.substr(0, pos);
  }

  return temp;
}

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

string get_filename(unsigned char* buff, unsigned long offset) {
  char filename[4096] = { 0 };

  WideCharToMultiByte(CP_ACP,
                      0,
                      (wchar_t*) (buff + offset), -1,
                      filename, sizeof(filename),
                      "+", NULL);

  return filename;
}

void process_generic_toc(int                fd, 
                         const ARCWHDR&     hdr, 
                         const ARCWTOCHDR2& tochdr,
                         unsigned long*     fn_table,
                         unsigned char*&    toc_buff,
                         unsigned char*     filenames) 
{
  ARCWTOCENTRY* entries = (ARCWTOCENTRY*) toc_buff;
  toc_buff += sizeof(ARCWTOCENTRY) * tochdr.entry_count;

  for (unsigned long i = 0; i < tochdr.entry_count; i++) {
    string filename = get_filename(filenames, fn_table[i]);

    unsigned long  len  = entries[i].original_length;
    unsigned char* buff = new unsigned char[len];
    read_uncompress(fd, 
                    hdr.data_offset + entries[i].offset, 
                    entries[i].length, 
                    buff,
                    entries[i].original_length);

    make_path(filename);

    int out_fd = open_or_die(filename,
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }
}

// I tried to squash these two functions together but it was too messy, so
// we'll just repeat a few lines of code instead.
void process_dds_toc(int                fd, 
                     const ARCWHDR&     hdr, 
                     const ARCWTOCHDR2& tochdr,
                     unsigned long*     fn_table,
                     unsigned char*&    toc_buff,
                     unsigned char*     filenames) 
{
  ARCWDDSTOC* ddstoc = (ARCWDDSTOC*) toc_buff;
  toc_buff += sizeof(*ddstoc);

  ARCWDDSENTRY* ddsentries = (ARCWDDSENTRY*) toc_buff;
  toc_buff += sizeof(ARCWDDSENTRY) * ddstoc->entry_count;

  ARCWTOCENTRY2* entries = (ARCWTOCENTRY2*) toc_buff;
  toc_buff += sizeof(ARCWTOCENTRY2) * tochdr.entry_count;

  for (unsigned long i = 0; i < tochdr.entry_count; i++) {
    string filename = get_filename(filenames, fn_table[i]);

    unsigned long  len  = entries[i].original_length;
    unsigned char* buff = new unsigned char[len];
    read_uncompress(fd, 
                    hdr.data_offset + entries[i].offset, 
                    entries[i].length, 
                    buff,
                    len);

    unsigned long width  = ddsentries[entries[i].info_index].width;
    unsigned long height = ddsentries[entries[i].info_index].height;
    unsigned long stride = width * 4;

    unsigned char* flip_buff = new unsigned char[len];

    for (unsigned long y = 0; y < height; y++) {
      memcpy(flip_buff + (height - y - 1) * stride, buff + y * stride, stride);
    }

    // Presumably we'll always have bitmap data here
    filename = get_file_prefix(filename) + ".bmp";

    make_path(filename);

    int out_fd = open_or_die(filename,
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);

    {
      BITMAPFILEHEADER bmf;
      BITMAPINFOHEADER bmi;

      memset(&bmf, 0, sizeof(bmf));
      memset(&bmi, 0, sizeof(bmi));

      bmf.bfType     = 0x4D42;
      bmf.bfSize     = sizeof(bmf) + sizeof(bmi) + len;
      bmf.bfOffBits  = sizeof(bmf) + sizeof(bmi);

      bmi.biSize     = sizeof(bmi);
      bmi.biWidth    = width;
      bmi.biHeight   = height;
      bmi.biPlanes   = 1;
      bmi.biBitCount = 32;
    
      write(out_fd, &bmf, sizeof(bmf));
      write(out_fd, &bmi, sizeof(bmi));
    }

    write(out_fd, flip_buff, len);
    close(out_fd);

    delete [] flip_buff;
    delete [] buff;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exarcw v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.arc>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  ARCWHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  filenames_len = 0;
  unsigned char* filenames     = NULL;
  read_hdr_chunk(fd, filenames, filenames_len);

  unsigned long  toc_len  = 0;
  unsigned char* toc_buff = NULL;
  read_hdr_chunk(fd, toc_buff, toc_len);  

  unsigned char* toc_p = toc_buff;

  ARCWTOCHDR1* tochdr = (ARCWTOCHDR1*) toc_p;
  toc_p += sizeof(*tochdr);

  for (unsigned long i = 0; i < tochdr->entry_count; i++) {
    ARCWTOCHDR2* tochdr2 = (ARCWTOCHDR2*) toc_p;
    toc_p += sizeof(*tochdr2);

    unsigned long* fn_table    = (unsigned long*) toc_p;
    unsigned char* subtoc_buff = toc_p + sizeof(unsigned long) * tochdr2->entry_count;

    if (tochdr2->type == 0x00534444) {
      process_dds_toc(fd, hdr, *tochdr2, fn_table, subtoc_buff, filenames);
    } else if (tochdr2->type == 0) {
      process_generic_toc(fd, hdr, *tochdr2, fn_table, subtoc_buff, filenames);
    } else {
      printf("%s: TOC %d is of unknown type (%d)\n", in_filename.c_str(), i, tochdr2->type);
    }

    toc_p += tochdr2->toc_length;
  }

  delete [] toc_buff;
  delete [] filenames;

  close(fd);

  return 0;
}
