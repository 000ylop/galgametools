// exar2fp.cpp, v1.0 2009/01/28
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Extracts multiple *.fp archives used by Ar Tonelico 2.

#include "as-util.h"

#define EN
//#define JP

// Some files are found in multiple *.fp archives but I think they are true dupes
// #define SEPARATE_DUPLICATES

#ifdef EN 
static const unsigned long FP_TABLE_OFFSET = 2792704;
#endif

#ifdef JP
static const unsigned long FP_TABLE_OFFSET = 2785336;
#endif

struct FPTBL1ENTRY {
  unsigned long filename;
  unsigned long offset;
  unsigned long entry_count;
};

struct FPTBL2ENTRY {
  unsigned long filename;
  unsigned long offset;
  unsigned long length;
};

template<class T> T off2addr(unsigned char* buff, unsigned long offset) {
  return (T) (buff + 4096 + offset - 0x100000);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exar2rpk v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <SLUS_217.88 etc>\n\n", argv[0]);
    fprintf(stderr, "This expects to find fpack/*.fp which come from RPK.BIN.\n\n", argv[0]);
    return -1;
  }

  string elf_filename(argv[1]);

  int            fd       = as::open_or_die(elf_filename, O_RDONLY | O_BINARY);
  unsigned long  elf_len  = as::get_file_size(fd);
  unsigned char* elf_buff = new unsigned char[elf_len];
  read(fd, elf_buff, elf_len);
  close(fd);

  FPTBL1ENTRY* fp1 = (FPTBL1ENTRY*) (elf_buff + FP_TABLE_OFFSET);

  for (unsigned long i = 0; i < 905; i++) {
    char* filename = off2addr<char*>(elf_buff, fp1[i].filename);
    printf("0x%08X [%s] \n", fp1[i].entry_count, filename);

    fd = as::open_or_die(filename, O_RDONLY | O_BINARY);

    FPTBL2ENTRY* fp2 = off2addr<FPTBL2ENTRY*>(elf_buff, fp1[i].offset);

    for (unsigned long j = 0; j < fp1[i].entry_count; j++) {
      char* filename2 = off2addr<char*>(elf_buff, fp2[j].filename);
      printf("\t 0x%08X  0x%08X [%s]\n", fp2[j].offset, fp2[j].length, filename2);

      unsigned long  len  = fp2[j].length;
      unsigned char* buff = new unsigned char[len];
      lseek(fd, fp2[j].offset * 2048, SEEK_SET);
      read(fd, buff, len);

#ifdef SEPARATE_DUPLICATES
      string out_filename = string("fpack_extract/") + as::get_file_prefix(filename) + "/" + filename2;
#else
      string out_filename = string("fpack_extract/") + filename2;
#endif

      as::make_path(out_filename);
      as::write_file(out_filename, buff, len);

      delete [] buff;

    }

    close(fd);
  }

  delete [] elf_buff;

  return 0;
}