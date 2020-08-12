// excandydat.cpp, v1.01 2010/10/31
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts filelist.db+*.dat archives used in Candy Doll.

#include "as-util.h"
#include <map>

// Crypto++
#include "aes.h"
#include "modes.h"

// Want SQLite build from phxsoftware
#include "sqlite3.h"
extern "C" SQLITE_API int sqlite3_key(sqlite3 *db, const void *pKey, int nKey);

#define KEY_VERSION 2

void pack_seed(unsigned char* buff, unsigned long len, const string& seed_str) {
  const unsigned char* seed     = (const unsigned char*) seed_str.c_str();
  const unsigned long  seed_len = seed_str.length();

  memset(buff, 0, len);

  if (seed_len <= len) {
    memcpy(buff, seed, seed_len);
  } else {
    for (unsigned long i = 0; i < seed_len; i++) {
      buff[i % len] ^= seed[i];
    }
  }
}

void read_decrypt(int fd, unsigned char* buff, unsigned long len, const string& seed) {
  read(fd, buff, len);

  unsigned char key[32] = { 0 };
  pack_seed(key, sizeof(key), seed);

  unsigned char iv[16]  = { 0 };
  pack_seed(iv, sizeof(iv), seed);

  CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor;    
  decryptor.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
  decryptor.ProcessData(buff, buff, len);
}

typedef std::map<int, int> arch_to_fd_t;
static arch_to_fd_t arch_to_fd;

int archive_callback(void* unused, int argc, char** argv, char** col_name) {
  int id      = atoi(argv[0]);
  string name = argv[1];

  arch_to_fd[id] = as::open_or_die(name + ".dat", O_RDONLY | O_BINARY);

  return 0;
}

int file_callback(void* unused, int argc, char** argv, char** col_name) {
  static const unsigned long ENCRYPTED_LEN = 256;

  string        filepath  = argv[2];
  unsigned long size      = atol(argv[3]);
  unsigned long offset    = atol(argv[4]);
  int           archiveid = atoi(argv[6]);

  string out_filename = as::convert_utf8(filepath);
  int    fd           = arch_to_fd[archiveid];

  if (fd == 0) {
    printf("%s: unknown archiveID (%d), file skipped.\n", out_filename.c_str(), archiveid);
    return 0;
  }

  unsigned long  len  = size;
  unsigned char* buff = new unsigned char[std::max(len, ENCRYPTED_LEN + 16)];

  lseek(fd, offset, SEEK_SET);
  read_decrypt(fd, buff, ENCRYPTED_LEN, filepath);

  if (len > ENCRYPTED_LEN) {
    lseek(fd, 16, SEEK_CUR);
    read(fd, buff + ENCRYPTED_LEN, len - ENCRYPTED_LEN);
  }

  as::make_path(out_filename);
  as::write_file(out_filename, buff, len);

  delete [] buff;

  return 0;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "excandydat v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <filelist.db>\n", argv[0]);
    return -1;
  }

  string db_filename(argv[1]);

  sqlite3* db      = NULL;
  char*    err_msg = NULL;

  if (sqlite3_open_v2(db_filename.c_str(), &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
    fprintf(stderr, "%s: error opening database\n", db_filename.c_str());
    return 0;
  }

#if KEY_VERSION >= 2
  // Haha, cute,
  sqlite3_key(db, "igs samp1e", 10);  
#else
  sqlite3_key(db, "igs sample", 10);  
#endif

  if (sqlite3_exec(db, "select * from archives", archive_callback, 0, &err_msg) != SQLITE_OK ||
      sqlite3_exec(db, "select * from file_infos", file_callback, 0, &err_msg) != SQLITE_OK)
  {
    fprintf(stderr, "%s: %s\n", db_filename.c_str(), err_msg);
  }

  sqlite3_close(db);

  return 0;
}
