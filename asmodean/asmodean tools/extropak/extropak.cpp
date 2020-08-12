// extropak.cpp, v1.02 2009/09/25
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts PACK (*.pak) archives used by ÉgÉçÉsÉJÉãKISS.

#include "as-util.h"

struct PAKHDR {
  unsigned char signature[4]; // "PACK"
  unsigned long toc_length;
};

struct PAKENTRY {
  char          name[32];
  unsigned long offset;
  unsigned long length;
};

void unobfuscate(unsigned char* buff, unsigned long len) {
  for (unsigned long i = 0; i < len; i++) {
    buff[i] = 0 - buff[i];
  }
}

void process_dir(int            fd,
                 unsigned char* toc_buff,
                 unsigned long  toc_len,
                 unsigned long  toc_base,
                 unsigned long  data_base,
                 const string&  path)
{
  unsigned char* toc_p = toc_buff + toc_base;

  unsigned long dir_count = *(unsigned long*) toc_p;
  toc_p += 4;

  if (dir_count) {
    PAKENTRY* dirs = (PAKENTRY*) toc_p;
    toc_p += sizeof(PAKENTRY) * dir_count;

    for (unsigned long i = 0; i < dir_count; i++) {
      process_dir(fd, 
                  toc_buff,
                  toc_len,
                  dirs[i].offset,
                  dirs[i].length,
                  path + dirs[i].name + "/");
    }
  }

  unsigned long file_count = *(unsigned long*) toc_p;
  toc_p += 4;

  PAKENTRY* files = (PAKENTRY*) toc_p;

  for (unsigned long i = 0; i < file_count; i++) {
    unsigned long  len  = files[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, data_base + files[i].offset, SEEK_SET);
    read(fd, buff, len);

    string out_filename = path + files[i].name;

    if (out_filename.find(".hse") != string::npos) {
      unobfuscate(buff, len);

      out_filename = as::get_file_prefix(out_filename) + as::guess_file_extension(buff, len);
    }

    as::make_path(out_filename);
    as::write_file(out_filename, buff, len);

    delete [] buff;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "extropak v1.02, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PAKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = hdr.toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];
  lseek(fd, 0, SEEK_SET);
  read(fd, toc_buff, toc_len);

  process_dir(fd,
              toc_buff,
              toc_len,
              sizeof(hdr),
              0,
              "./");   

  delete [] toc_buff;

  return 0;
}