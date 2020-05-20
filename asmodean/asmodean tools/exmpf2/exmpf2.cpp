// exmpf2.cpp, v1.0 2010/01/30
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts MPF2 (*.dat+*.a01 etc) archives.

#include "as-util.h"
#include <zlib.h>
#include <list>

struct MPF2HDR {
  unsigned char signature[4]; // "MPF2"
  unsigned long unknown1;
  unsigned long toc_length;
  unsigned long original_toc_length;
  unsigned long data_offset;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long unknown4;
};

struct MPF2ENTRY {
  unsigned long      entry_length;
  unsigned long      type;
  unsigned long long offset;
  unsigned long      unknown2;
  unsigned long      length;
  unsigned long      original_length;
  unsigned long      unknown3[125];
};

void read_uncompress(int            fd, 
                     unsigned long  len,
                     unsigned char* out_buff,
                     unsigned long  out_len)
{
  if (len != out_len) {
    unsigned char* buff = new unsigned char[len];
    read(fd, buff, len);    

    uncompress(out_buff, &out_len, buff, len);

    delete [] buff;
  } else {
    read(fd, out_buff, out_len);
  }
}

struct arc_info_t {
  string             filename;
  int                fd;
  unsigned long long start_offset;
  unsigned long long end_offset;
};

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "exmpf2 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat> [input.a01] [input.a02] ...\n", argv[0]);
    return -1;
  }

  typedef std::list<arc_info_t> arcs_t;
  arcs_t arcs;

  unsigned long long max_offset = 0;

  for (int i = 1; i < argc; i++) {
    arc_info_t arc;

    arc.filename     = argv[i];
    arc.fd           = as::open_or_die(arc.filename, O_RDONLY | O_BINARY);
    arc.start_offset = max_offset;
    arc.end_offset   = arc.start_offset + as::get_file_size(arc.fd);

    arcs.push_back(arc);

    max_offset  = arc.end_offset;
  }

  MPF2HDR hdr;
  read(arcs.front().fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = hdr.original_toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];
  read_uncompress(arcs.front().fd, hdr.toc_length, toc_buff, toc_len);

  unsigned char* p   = toc_buff;
  unsigned char* end = toc_buff + toc_len;

  while (p < end) {
    MPF2ENTRY* entry = (MPF2ENTRY*) p;
    p += sizeof(*entry);

    // The toc only has one null at the end so copy it to fix
    unsigned long wc_filename_len   = entry->entry_length - sizeof(*entry);
    wchar_t       wc_filename[4096] = { 0 };
    memcpy(wc_filename, p, wc_filename_len);
    p += wc_filename_len;

    string filename = as::convert_wchar(wc_filename);

    if (entry->type != 0) {
      printf("%s: skipped entry type %d (duplicate reference?)\n", filename.c_str(), entry->type);
      continue;
    }

    unsigned long long offset = entry->offset + hdr.data_offset;
    int                fd     = -1;

    for (arcs_t::iterator i = arcs.begin();
         i != arcs.end();
         ++i)
    {
      if (offset >= i->start_offset && offset < i->end_offset) {
        fd      = i->fd;
        offset -= i->start_offset;
      }
    }

    if (fd == -1) {
      fprintf(stderr, "%s: offset not found in archives (did you supply input.a01 etc?)\n", filename.c_str());
      return -1;
    }

    unsigned long  len  = entry->original_length;
    unsigned char* buff = new unsigned char[len];
    _lseeki64(fd, offset, SEEK_SET);
    read_uncompress(fd, entry->length, buff, len);   

    as::make_path(filename);
    as::write_file(filename, buff, len);

    delete [] buff;
  }

  delete [] toc_buff;

  for (arcs_t::iterator i = arcs.begin();
       i != arcs.end();
       ++i)
  {
    close(i->fd);
  }

  return 0;
}
