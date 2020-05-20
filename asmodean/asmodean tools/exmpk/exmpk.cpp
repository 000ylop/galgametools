// exmpk.cpp, v1.0 2007/07/25
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts propeller's *.mpk archives.  Single and multi-frame *.mgr
// images are decompressed.

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <cerrno>
#include <cstdio>
#include <string>

using std::string;

struct MPKHDR {
  unsigned long toc_offset;
  unsigned long entry_count;
};

struct MPKENTRY {
  char          filename[32];
  unsigned long offset;
  unsigned long length;
};

struct MGRHDR {
  unsigned short entry_count;
};

struct MGRENTRY {
  unsigned long offset;
};

struct MGRDATAHDR {
  unsigned long original_length;
  unsigned long length;
};

// LZ-like mishmash.
unsigned long unlzish(unsigned char* buff, 
                      unsigned long  len,
                      unsigned char* out_buff, 
                      unsigned long  out_len) 
{
  unsigned char* end        = buff + len;
  unsigned char* out_end    = out_buff + out_len;

  while (buff < end && out_buff < out_end) {
    unsigned long c = *buff++;

    if (c < 0x20) {
      c++;

      while (c--) {
        *out_buff++ = *buff++;
      }
    } else {
      unsigned long p = ((c & 0x1F) << 8) + 1;
      unsigned long n = c >> 5;

      if (n == 0x07) {
        n += *buff++;
      }

      p += *buff++;
      n += 2;

      unsigned char* src = out_buff - p;

      while (n--) {
        *out_buff++ = *src++;
      }
    }
  }

  return out_len - (out_end - out_buff);
}

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

void save_mgr(const string& prefix, unsigned char* buff, unsigned long len) {
  MGRHDR*   hdr     = (MGRHDR*) buff;
  MGRENTRY* entries = (MGRENTRY*) (hdr + 1);

  for (unsigned long i = 0; i < hdr->entry_count; i++) {
    MGRDATAHDR* datahdr        = NULL;
    char        filename[4096] = { 0 };
    
    // Seems to be an annoying 4-byte optimization...
    if (hdr->entry_count > 1) {
      datahdr = (MGRDATAHDR*) (buff + entries[i].offset);
      sprintf(filename, "%s+%03d.bmp", prefix.c_str(), i);
    } else {
      datahdr = (MGRDATAHDR*) (hdr + 1);
      sprintf(filename, "%s.bmp", prefix.c_str());
    }

    unsigned char* data     = (unsigned char*) (datahdr + 1);  

    unsigned long  out_len  = datahdr->original_length;
    unsigned char* out_buff = new unsigned char[out_len];
    unlzish(data, datahdr->length, out_buff, out_len);

    int fd = open_or_die(filename,
                         O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
                         S_IREAD | S_IWRITE);
    write(fd, out_buff, out_len);
    close(fd);

    delete [] out_buff;
  }
}

void unobfuscate(unsigned char* buff, unsigned long len, unsigned char key) { 
  for (unsigned long i = 0; i < len; i++) {
    buff[i] ^= key;
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

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exmpk v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.mpk>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  MPKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  MPKENTRY*     entries     = new MPKENTRY[hdr.entry_count];
  unsigned long entries_len = sizeof(MPKENTRY) * hdr.entry_count;
  lseek(fd, hdr.toc_offset, SEEK_SET);
  read(fd, entries, entries_len);

  // Maybe it's constant, but it's easy enough to guess
  unsigned char key = entries[0].filename[sizeof(entries[0].filename) - 1];

  unobfuscate((unsigned char*) entries, entries_len, key);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    char* filename = entries[i].filename;

    if (filename[0] == '\\') {
      filename++;
    }

    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    make_path(filename);

    if (strstr(filename, ".mgr")) {
      save_mgr(get_file_prefix(filename), buff, len);
    } else {
      int out_fd = open_or_die(filename,
                               O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 
                               S_IREAD | S_IWRITE);

      write(out_fd, buff, len);
      close(out_fd);
    }

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
