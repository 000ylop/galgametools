// exanepak.cpp, v1.02 2011/10/21
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts obfuscated CGPACK.idx/pak archives from OLE-M.

#include "as-util.h"
#include "as-lzss.h"

//#define IDX_VERSION 1
//#define IDX_VERSION 2
#define IDX_VERSION 3

struct IDXENTRY {
  char               filename[16];
  unsigned long      unknown1;
#if IDX_VERSION >= 2
  unsigned long long offset;
  unsigned long long length;
#else
  unsigned long      offset;
  unsigned long      length;
#endif
};

void unobfuscate(unsigned char* buff, 
                 unsigned long  len, 
                 unsigned long  seed)
{
  static const char KEY[] = "1qaz2wsx3edc4rfv5tgb6yhn7ujm8ik,9ol.0p;/-@:^[]";

  for (unsigned long i = 0; i < len; i++) {
    seed *= 0x343FD;
    seed += 0x269EC3;

    unsigned long index = (seed >> 0x10) & 0x7FFF;

    buff[i] ^= KEY[index % (sizeof(KEY) - 1)];
  }
}

// From The Art of Computer Programming Volume 2 (Section 3.6 page 185)
long krand(long* seed) {
#define MM 2147483647    // a Mersenne prime
#define AA 48271         // this does well in the spectral test
#define QQ 44488         // (long)(MM/AA)
#define RR 3399          // MM % AA; it is important that RR<QQ

  *seed = AA*(*seed % QQ) - RR*(unsigned int)(*seed/QQ);
  if (*seed < 0)
    *seed += MM;

  return *seed;
}

#if IDX_VERSION >= 3
  static const char KEY[] = "KF4QHcm2";
#else
  static const char KEY[] = "EAGLS_SYSTEM";
#endif

static const unsigned long KEY_LEN = sizeof(KEY) - 1;

void unobfuscate2(unsigned char* buff, unsigned long len) {
  long seed = 0x75BD924 ^ buff[len - 1];

  for (unsigned long i = 0; i < len && i < 0x174B; i++) {
    long index = long(krand(&seed) * 0.0000000004656612875245797 * 256);

    buff[i] ^= KEY[index % KEY_LEN];
  }
}

void unobfuscate3(unsigned char* buff, unsigned long len) {
  srand((char)buff[len - 1]);

  for (unsigned long i = 0; i < len - 2; i += 2) {
    buff[i] ^= KEY[rand() % KEY_LEN];
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "exanepak, v1.02 by asmodean\n\n");
    fprintf(stderr, "usage: %s <cgpack.idx> <cgpack.pak>\n\n", argv[0]);
    return -1;
  }

  string idx_filename(argv[1]);
  string pak_filename(argv[2]);
  
  int fd = as::open_or_die(idx_filename, O_RDONLY | O_BINARY);

  unsigned long  toc_len  = as::get_file_size(fd);  
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);
  close(fd);

  IDXENTRY*      entries  = (IDXENTRY*) toc_buff;
  unsigned long* toc_seed = (unsigned long*) (toc_buff + toc_len - 4);

  unobfuscate(toc_buff, toc_len, *toc_seed);
  
  fd = as::open_or_die(pak_filename, O_RDONLY | O_BINARY);

  // For some reason the offsets are all shifted by an arbitrary(?) value
  unsigned long long offset_shift = entries[0].offset;

  for (unsigned int i = 0; true; i++) {
    if (entries[i].filename[0] == 0x00) {
      break;
    }

    unsigned long  len  = (unsigned long) entries[i].length;
    unsigned char* buff = new unsigned char[len];
    _lseeki64(fd, entries[i].offset - offset_shift, SEEK_SET);
    read(fd, buff, len);    

    string filename = entries[i].filename;

    if (as::stringtol(filename).find(".gr") != string::npos) {
      unobfuscate2(buff, len);

      // Don't know the expected length, so "big enough" ...
      unsigned long  out_len  = 16 * 1024 * 1024;
      unsigned char* out_buff = new unsigned char[out_len];

      out_len = as::unlzss(buff, len, out_buff, out_len);

      delete [] buff;

      len      = out_len;
      buff     = out_buff;
      filename = as::get_file_prefix(filename) + as::guess_file_extension(out_buff, out_len);
    }

    if (as::stringtol(filename).find(".dat") != string::npos) {
      // Always?
      static const unsigned long TEXT_OFFSET = 3600;

      unsigned char* text_buff = buff + TEXT_OFFSET;
      unsigned long  text_len  = len  - TEXT_OFFSET;

      unobfuscate3(text_buff, text_len);

      as::write_file(as::get_file_prefix(filename) + ".txt", text_buff, text_len - 2);
    }

    as::write_file(filename, buff, len);

    delete [] buff;
  }

  delete [] toc_buff;

  return 0;
}
