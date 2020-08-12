// alb2png.cpp, v1.01 2007/08/19
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool decompresses Gesen 18's *.alb data (PNG and DDS).

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdio>
#include <string>

using std::string;

struct ALBHDR {
  unsigned char signature[4]; // "ALB1"
  unsigned long unknown1;
  unsigned long length;
  unsigned long unknown2;
};

struct PHHDR {
  unsigned char  signature[2]; // "PH"
  unsigned short table_length;
  unsigned short data_length;
  unsigned char  table_compressed;
  unsigned char  table_marker;
};

struct PHTBLENTRY {
  unsigned char first;
  unsigned char second;
};

// encodes multiple sub-ranges of literals
void expand_table(unsigned char* buff, 
                  unsigned long  len, 
                  PHTBLENTRY*    table,
                  unsigned char  marker) 
{
  unsigned char* end = buff + len;
  unsigned char  i   = 0;

  while (buff < end) {
    unsigned char c = *buff++;
    unsigned char n = *buff++;

    if (c == marker) {
      while (n--) {
        table[i].first  = i;
        table[i].second = 0;
        i++;
      }
    } else {
      table[i].first  = c;
      table[i].second = n;
      i++;
    }
  }
}

// byte-pair encoding
unsigned long unbpe(unsigned char*    buff, 
                    unsigned long     len, 
                    unsigned char*    out_buff,
                    const PHTBLENTRY* table)
{
  unsigned char* end              = buff + len;
  unsigned long  out_len          = 0;

  unsigned char  stack_buff[4096] = { 0 };
  unsigned char  stack_len        = 0;

  while (buff < end || stack_len) {
    unsigned char c = 0;
    
    if (stack_len) {
      c = stack_buff[--stack_len];
    } else {
      c = *buff++;
    }

    if (table[c].first == c) {
      *out_buff++ = c;
      out_len++;
    } else {
      stack_buff[stack_len++] = table[c].second;
      stack_buff[stack_len++] = table[c].first;
    }
  }

  return out_len;
}

int open_or_die(const string& filename, int flags, int mode = 0) {
  int fd = open(filename.c_str(), flags, mode);

  if (fd == -1) {
    fprintf(stderr, "Could not open %s (%s)\n", filename.c_str(), strerror(errno));
    exit(-1);
  }

  return fd;
}

string get_file_prefix(const std::string& filename) {
  string temp(filename);

  string::size_type pos = temp.find_last_of(".");
  if (pos != string::npos) {
    temp = temp.substr(0, pos);
  }

  return temp;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "alb2png v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.alb>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  
  string prefix(get_file_prefix(in_filename));

  int fd = open_or_die(in_filename, O_RDONLY | O_BINARY);

  ALBHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  out_len  = hdr.length;
  unsigned char* out_buff = new unsigned char[out_len];

  unsigned char* p       = out_buff;
  unsigned char* out_end = out_buff + out_len;

  while (p < out_end) {
    PHHDR ph;
    read(fd, &ph, sizeof(ph));

    unsigned long  table_len  = ph.table_length;
    unsigned char* table_buff = new unsigned char[table_len];
    read(fd, table_buff, table_len);

    PHTBLENTRY table[256] = { 0 };

    if (ph.table_compressed) {
      expand_table(table_buff, table_len, table, ph.table_marker);
    } else {
      memcpy(table, table_buff, sizeof(table));
    }

    unsigned long  len  = ph.data_length;
    unsigned char* buff = new unsigned char[len];
    read(fd, buff, len);

    unsigned long used_len = unbpe(buff, len, p, table);
    p += used_len;

    delete [] buff;
    delete [] table_buff;
  }

  close(fd);

  string out_filename = prefix;

  if (!memcmp(out_buff, "\x89PNG", 4)) {
    out_filename += ".png";
  } else if (!memcmp(out_buff, "DDS", 3)) {
    out_filename += ".dds";
  }

  fd = open_or_die(out_filename,
                   O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                   S_IREAD | S_IWRITE);
  write(fd, out_buff, out_len);
  close(fd);

  delete [] out_buff;

  return 0;
}
