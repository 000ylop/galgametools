// exhecdat.cpp, v1.0 2010/03/04
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.lib+*.dat archives used by Hecate.

#include "as-util.h"

 // Crypto++
#include "aes.h"
#include "modes.h"

struct LIBHDR {
  unsigned long long entry_count;
};

struct LIBENTRY {
  char               filename[512];
  unsigned long long length;
  unsigned long long encrypted_length;
  unsigned long long remain_length;
  unsigned long long offset;
  unsigned long      width;
  unsigned long      height;
};

void pack_seed(unsigned char* buff, unsigned long len) {
  // UTF8 プッチンプリン食べたいなー
  static const unsigned char SEED[] = {    
    0xE3, 0x83, 0x97, 0xE3, 0x83, 0x83, 0xE3, 0x83, 0x81, 0xE3, 0x83, 0xB3, 0xE3, 0x83, 0x97, 0xE3,
    0x83, 0xAA, 0xE3, 0x83, 0xB3, 0xE9, 0xA3, 0x9F, 0xE3, 0x81, 0xB9, 0xE3, 0x81, 0x9F, 0xE3, 0x81,
    0x84, 0xE3, 0x81, 0xAA, 0xE3, 0x83, 0xBC, 
  };

  static const unsigned long SEED_LEN = sizeof(SEED);

  memset(buff, 0, len);

  if (SEED_LEN <= len) {
    memcpy(buff, SEED, SEED_LEN);
  } else {
    for (unsigned long i = 0; i < SEED_LEN; i++) {
      buff[i % len] ^= SEED[i];
    }
  }
}

void read_decrypt(int fd, unsigned char* buff, unsigned long len) {
  read(fd, buff, len);

  unsigned char key[32] = { 0 };
  pack_seed(key, sizeof(key));

  unsigned char iv[16]  = { 0 };
  pack_seed(iv, sizeof(iv));

  CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor;    
  decryptor.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
  decryptor.ProcessData(buff, buff, len);
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "exhecdat v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.lib> <input.dat\n", argv[0]);
    return -1;
  }

  string lib_filename(argv[1]);
  string dat_filename(argv[2]);

  int fd = as::open_or_die(lib_filename, O_RDONLY | O_BINARY);
  
  unsigned long  toc_len  = as::get_file_size(fd);
  unsigned char* toc_buff = new unsigned char[toc_len];
  read_decrypt(fd, toc_buff, toc_len);
  close(fd);

  fd = as::open_or_die(dat_filename, O_RDONLY | O_BINARY);

  LIBHDR*   hdr     = (LIBHDR*) toc_buff;
  LIBENTRY* entries = (LIBENTRY*) (hdr + 1);

  for (unsigned long i = 0; i < hdr->entry_count; i++) {
    unsigned long  len        = (unsigned long) entries[i].length;    
    unsigned long  decr_len   = (unsigned long) entries[i].encrypted_length;
    unsigned long  remain_len = (unsigned long) entries[i].remain_length;
    unsigned long  max_len    = decr_len + remain_len;
    unsigned char* buff       = new unsigned char[max_len];

    _lseeki64(fd, entries[i].offset, SEEK_SET);
    read_decrypt(fd, buff, decr_len);
    read(fd, buff + (len - remain_len), remain_len);

    as::write_file(entries[i].filename + as::guess_file_extension(buff, len), 
                   buff,
                   len);

    delete [] buff;
  }

  delete [] toc_buff;

  close(fd);

  return 0;
}
