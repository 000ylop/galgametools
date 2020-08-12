// exkoigdat.cpp, v1.0 2009/11/20
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from óˆàﬂ Å`Ç‹Ç∏ÇÕÉRÉåíÖÇƒ!Å`

#include "as-util.h"
#include "as-lzss.h"

struct DATHDR {
  unsigned long entry_count;
  unsigned long unknown1;
};

struct DATENTRY {
  char          filename[32];
  unsigned long original_length;
  unsigned long length;
  unsigned long offset;
};

void unobfuscate(unsigned char* buff, unsigned long len) {
  unsigned char KEY1[] = { 0x89, 0x98, 0x97, 0x94, 0x9b };
  unsigned char KEY2[] = { 0xB5, 0xB9 };

  unsigned long  key_len = 0;
  unsigned char* key     = NULL;

  if (buff[42] == 0x97) {
    key     = KEY1;
    key_len = sizeof(KEY1);
  } else {
    key     = KEY2;
    key_len = sizeof(KEY2);
  }

  for (unsigned long i = 0; i < len; i++) {
    buff[i] -= key[i % key_len];
  }
}

static const unsigned long VIDEO_PREFIX_LEN = 0x32000;

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exkkesui v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.dat>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  unsigned long file_len = as::get_file_size(fd);

  // This is really retarded obfuscation for video.
  if (file_len >= VIDEO_PREFIX_LEN) {
    unsigned long offset = file_len - VIDEO_PREFIX_LEN;

    unsigned long sigcheck = 0;
    lseek(fd, offset, SEEK_SET);
    read(fd, &sigcheck, sizeof(sigcheck));

    if (sigcheck == 0x75B22630) {
      unsigned long  len  = file_len;
      unsigned char* buff = new unsigned char[len];
      lseek(fd, 0, SEEK_SET);
      read(fd, buff, len);
      close(fd);

      fd = as::open_or_die(as::get_file_prefix(in_filename, true) + ".wmv",
                           O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                           S_IREAD | S_IWRITE);
      write(fd, buff + offset, VIDEO_PREFIX_LEN);
      write(fd, buff + VIDEO_PREFIX_LEN, offset);
      close(fd);

      delete [] buff;

      return 0;
    }
  }

  DATHDR hdr;
  lseek(fd, 0, SEEK_SET);
  read(fd, &hdr, sizeof(hdr));

  unsigned long entries_len = hdr.entry_count * sizeof(DATENTRY);
  DATENTRY*     entries     = new DATENTRY[hdr.entry_count];
  read(fd, entries, entries_len);
  unobfuscate((unsigned char*)entries, entries_len);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = entries[i].length;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (len != entries[i].original_length) {
      unsigned long  temp_len  = entries[i].original_length;
      unsigned char* temp_buff = new unsigned char[temp_len];
      as::unlzss(buff, len, temp_buff, temp_len);

      delete [] buff;

      len  = temp_len;
      buff = temp_buff;
    }

    char filename[1024] = { 0 };
    memcpy(filename, entries[i].filename, sizeof(entries[i].filename));

    as::write_file(filename, buff, len);

    delete [] buff;
  }

  delete [] entries;

  return 0;
}
