// exs4alf.cpp, v1.1 2009/04/26
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts S4IC413 (sys4ini.bin + *.ALF) and S4AC422 (*.AAI + *.ALF)
// archives.

#include "as-util.h"
#include "as-lzss.h"

struct S4HDR {
  char          signature_title[240]; // "S4IC413 <title>", "S4AC422 <title>"
  unsigned char unknown[60];
};

struct S4SECTHDR {
  unsigned long original_length;
  unsigned long original_length2; // why?
  unsigned long length;
};

struct S4TOCARCHDR {
  unsigned long entry_count;
};

struct S4TOCARCENTRY {
  // There's a bunch of junk following the name which I assume is
  // uninitialized memory...
  char filename[256];
};

typedef S4TOCARCHDR S4TOCFILHDR;

struct S4TOCFILENTRY {
  char          filename[64];
  unsigned long archive_index;
  unsigned long file_index; // within archive?
  unsigned long offset;
  unsigned long length;
};

void read_sect(int fd, unsigned char*& out_buff, unsigned long& out_len) {
  S4SECTHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  len  = hdr.length;
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);

  out_len  = hdr.original_length;
  out_buff = new unsigned char[out_len];
  as::unlzss(buff, len, out_buff, out_len);

  delete [] buff;
}

struct arc_info_t {
  int    fd;
  string dir;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exs4alf v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <sys4ini.bin>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  S4HDR hdr;
  read(fd, &hdr, sizeof(hdr));

  // Hack for addon archives
  if (!memcmp(hdr.signature_title, "S4AC", 4)) {
    lseek(fd, 268, SEEK_SET);
  }

  unsigned long  toc_len  = 0;
  unsigned char* toc_buff = NULL;
  read_sect(fd, toc_buff, toc_len);
  close(fd);

  S4TOCARCHDR*   archdr     = (S4TOCARCHDR*) toc_buff;
  S4TOCARCENTRY* arcentries = (S4TOCARCENTRY*) (archdr + 1);

  S4TOCFILHDR*   filhdr     = (S4TOCFILHDR*) (arcentries + archdr->entry_count);
  S4TOCFILENTRY* filentries = (S4TOCFILENTRY*) (filhdr + 1);

  arc_info_t* arc_info = new arc_info_t[archdr->entry_count];

  for (unsigned long i = 0; i < archdr->entry_count; i++) {
    arc_info[i].fd = open(arcentries[i].filename, O_RDONLY | O_BINARY);
    if (arc_info[i].fd != -1) {
      arc_info[i].dir = as::get_file_prefix(arcentries[i].filename) + "/";
      as::make_path(arc_info[i].dir);
    } else {
      fprintf(stderr, "%s: could not open (skipped!)\n", arcentries[i].filename);
    }
  }

  for (unsigned long i = 0; i < filhdr->entry_count; i++) {
    arc_info_t& arc = arc_info[filentries[i].archive_index];

    if (arc.fd == -1 || !filentries[i].length) {
      continue;
    }

    unsigned long  len  = filentries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(arc.fd, filentries[i].offset, SEEK_SET);
    read(arc.fd, buff, len);

    int out_fd = as::open_or_die(arc.dir + filentries[i].filename,
                                 O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                                 S_IREAD | S_IWRITE);
    write(out_fd, buff, len);
    close(out_fd);

    delete [] buff;
  }

  delete [] arc_info;
  delete [] toc_buff;

  return 0;
}
