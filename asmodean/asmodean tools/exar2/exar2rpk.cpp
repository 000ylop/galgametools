// exar2rpk.cpp, v1.0 2009/01/22
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Extracts/rebuilds RPK archives used by Ar Tonelico 2

#include "as-util.h"

#define EN
//#define JP

#ifdef EN 
static const unsigned long OFFSET_TABLE_OFFSET = 2591376;
static const unsigned long NAME_TABLE_OFFSET   = 7399024;
#endif

#ifdef JP
static const unsigned long OFFSET_TABLE_OFFSET = 2584720;
static const unsigned long NAME_TABLE_OFFSET   = 7609072;
#endif

struct TOCENTRY {
  unsigned long filename_offset;
  unsigned long offset; // LBA
  unsigned long length; // bytes
};

int main(int argc, char** argv) {
  if (argc != 3 && argc != 5) {
    fprintf(stderr, "exar2rpk v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <SLUS_217.88 etc> <RPK.BIN> [output.elf output.bin]\n", argv[0]);
    return -1;
  }

  string elf_filename(argv[1]);
  string rpk_filename(argv[2]);
  string out_elf_filename;
  string out_rpk_filename;

  if (argc > 3) {
    out_elf_filename = argv[3];
    out_rpk_filename = argv[4];
  }

  bool do_rebuild = !out_elf_filename.empty();

  int fd     = as::open_or_die(elf_filename, O_RDONLY | O_BINARY);
  int out_fd = -1;

  unsigned long  elf_len  = as::get_file_size(fd);
  unsigned char* elf_buff = new unsigned char[elf_len];
  read(fd, elf_buff, elf_len);
  close(fd);

  TOCENTRY* entries = (TOCENTRY*) (elf_buff + OFFSET_TABLE_OFFSET);

  fd = as::open_or_die(rpk_filename, O_RDONLY | O_BINARY);
  unsigned long rpk_len = as::get_file_size(fd);

  if (do_rebuild) {
    out_fd = as::open_or_die(out_rpk_filename,
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);
  }

  long fn_delta = entries[0].filename_offset - NAME_TABLE_OFFSET;
  bool done     = false;

  for (unsigned long i = 0; !done; i++) {
    char*         filename   = (char*) (elf_buff + entries[i].filename_offset - fn_delta);
    unsigned long len_padded = (entries[i].length + 2047) & ~2047;
    
    done = entries[i].offset * 2048 + len_padded == rpk_len;

    printf("%05d: LBA %8d len %10d [%s]\n", i, entries[i].offset, entries[i].length, filename);

    if (do_rebuild) {
      int            new_fd         = as::open_or_die(filename, O_RDONLY | O_BINARY);
      unsigned long  new_len        = as::get_file_size(new_fd);
      unsigned long  new_len_padded = (new_len + 2047) & ~2047;
      unsigned char* new_buff       = new unsigned char[new_len_padded];
      memset(new_buff, 0, new_len_padded);
      read(new_fd, new_buff, new_len);
      close(new_fd);

      entries[i].length = new_len;
      entries[i].offset = tell(out_fd) / 2048;

      write(out_fd, new_buff, new_len_padded);

      delete [] new_buff;
    } else {
      unsigned long  len  = entries[i].length;
      unsigned char* buff = new unsigned char[len];
      lseek(fd, entries[i].offset * 2048, SEEK_SET);
      read(fd, buff, len);

      as::make_path(filename);
      as::write_file(filename, buff, len);

      delete [] buff;
    }

    if (done) {
      break;
    }
  }

  if (do_rebuild) {
    close(out_fd);

    out_fd = as::open_or_die(out_elf_filename,
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);
    write(out_fd, elf_buff, elf_len);
    close(out_fd);
  }
  
  delete [] elf_buff;

  close(fd);

  return 0;
}