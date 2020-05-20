// exdebopak.cpp, v1.02 2010/02/01
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts PAK (*.pak) archives used by Ç≈Ç⁄ÇÃëÉêªçÏèä in ê_äyìπíÜãL.

#include "as-util.h"
#include "zlib.h"
#include <limits>

struct PAKHDR {
  unsigned char  signature[4]; // "PAK"
  unsigned short hdr1_length;
  unsigned short unknown1;
  unsigned long  unknown2;
  unsigned long  unknown3;
  unsigned long  hdr2_length;
  unsigned long  unknown5;
  unsigned long  unknown6;
  unsigned long  original_toc_length;
  unsigned long  toc_length;
  unsigned long  unknown7;
};

struct PAKENTRY {
  unsigned long offset;
  unsigned long unknown1;
  unsigned long original_length;
  unsigned long unknown2;
  unsigned long length;
  unsigned long unknown3;
  unsigned long flags;
  unsigned long unknown5;

  unsigned long unknown6;
  unsigned long unknown7;
  unsigned long unknown8;
  unsigned long unknown9;
  unsigned long unknown10;
};

unsigned long inflate_raw(unsigned char* buff, 
                          unsigned long  len, 
                          unsigned char* out_buff, 
                          unsigned long  out_len)
{
  z_stream stream;

  stream.zalloc    = (alloc_func)0;
  stream.zfree     = (free_func)0;
  stream.opaque    = (voidpf)0;
  stream.next_in   = buff;
  stream.avail_in  = len;
  stream.next_out  = out_buff;
  stream.avail_out = out_len; 
     
  if (inflateInit2(&stream, -MAX_WBITS) >= 0) {  
    inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
  }

  return stream.total_out;
}

void process_dir(int             fd,
                 unsigned long   base,
                 unsigned char*& toc, 
                 unsigned char*  toc_end, 
                 unsigned long   entry_count,
                 const string&   path)
{
  while (toc < toc_end && entry_count--) {
    PAKENTRY* entry = (PAKENTRY*) toc;
    toc += sizeof(*entry);

    char* filename = (char*) toc;
    toc += strlen(filename) + 1;

    string full_name = path + filename;

    if (entry->flags & 0xA0) {
      unsigned long  len  = entry->length;
      unsigned char* buff = new unsigned char[len];
      lseek(fd, base + entry->offset, SEEK_SET);
      read(fd, buff, len);

      unsigned long  out_len  = entry->original_length;
      unsigned char* out_buff = new unsigned char[out_len];
      inflate_raw(buff, len, out_buff, out_len);

      as::make_path(full_name);
      as::write_file(full_name, out_buff, out_len);

      delete [] out_buff;
      delete [] buff;
    } else {
      process_dir(fd, base, toc, toc_end, entry->original_length, full_name + "/");
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exdebopak v1.02, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pak>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PAKHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = hdr.toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);

  unsigned long  out_toc_len  = hdr.original_toc_length;
  unsigned char* out_toc_buff = new unsigned char[out_toc_len];
  inflate_raw(toc_buff, toc_len, out_toc_buff, out_toc_len);

  process_dir(fd,
              sizeof(hdr) + toc_len,
              out_toc_buff,
              out_toc_buff + out_toc_len,
              std::numeric_limits<unsigned long>::max(),
              "./");   

  delete [] out_toc_buff;
  delete [] toc_buff;

  return 0;
}