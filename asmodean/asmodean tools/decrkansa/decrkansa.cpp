// decrkansa.cpp, v1.03 2010/10/02
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool decrypt's KimiAru (and others)'s NSA archive.  Use common NScripter
// archive tools to extract data from it.

#include "as-util.h"
#include "openssl/md5.h"
#include "openssl/sha.h"
#include "openssl/rc4.h"
#include "openssl/hmac.h"

static const unsigned long MAX_HASH_DATA_LENGTH = 1024;

void decrypt_block(unsigned long  index, 
                   unsigned char* buff, 
                   unsigned long  len, 
                   unsigned char* out_buff,
                   unsigned char* hash_data,
                   unsigned long  hash_data_len)
{
  unsigned long seed_data[2] = { index, 0 };

  unsigned char md5hash[MD5_DIGEST_LENGTH]  = { 0 };
  unsigned char sha1hash[SHA_DIGEST_LENGTH] = { 0 };
  unsigned char hash_key[MD5_DIGEST_LENGTH] = { 0 };

  MD5((unsigned char*)&seed_data, sizeof(seed_data), md5hash);
  SHA1((unsigned char*)&seed_data, sizeof(seed_data), sha1hash);

  for (unsigned long i = 0; i < sizeof(md5hash); i++) {
    hash_key[i] = md5hash[i] ^ sha1hash[i];
  }  

  unsigned char rc4key_data[64] = { 0 };
  unsigned int  rc4key_data_len = sizeof(rc4key_data);

  HMAC(EVP_sha512(),
       hash_key, sizeof(hash_key),
       hash_data, hash_data_len,
       rc4key_data, &rc4key_data_len);

  RC4_KEY rc4key;
  RC4_set_key(&rc4key, rc4key_data_len, rc4key_data);

  unsigned char junk[300];
  RC4(&rc4key, sizeof(junk), junk, junk);
  RC4(&rc4key, len, buff, out_buff);
}

void find_hash_data(const string&  filename, 
                    unsigned char* hash_data, 
                    unsigned long& hash_data_len) {
  static const unsigned char MARKER_BLOCK[] = { 0x22, 0x25, 0x73, 0x22, 0x00, 0x00, 0x00, 0x00, 
                                                0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
  static const unsigned long MARKER_LEN     = sizeof(MARKER_BLOCK);

  int fd = as::open_or_die(filename, O_RDONLY | O_BINARY);

  unsigned long  len  = as::get_file_size(fd);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  unsigned char* start_match = buff;
  unsigned char* end         = buff + len;

  while (buff + MARKER_LEN < end) {
    if (!memcmp(buff, MARKER_BLOCK, MARKER_LEN)) {
      buff += MARKER_LEN;

      unsigned char* p = buff;
      
      for (unsigned long i = 0;
           i < MAX_HASH_DATA_LENGTH && p < end && *p;
           i++, p++);

      unsigned long len = (unsigned long) (p - buff);

      if (len) {
        hash_data_len = len;
        memcpy(hash_data, buff, hash_data_len);
        return;
      }
    }
    buff++;
  }

  fprintf(stderr, "Unsupported? Tell asmodean.\n");
  exit(-1);
}

static const unsigned long BLOCK_SIZE = 1024;

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "decrkansa v1.03 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.nsa> <output.nsa> [game.exe]\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);  
  string out_filename(argv[2]);  
  string exe_filename(argc > 3 ? argv[3] : "");
  
  unsigned char hash_data[MAX_HASH_DATA_LENGTH] = { 0 };
  unsigned long hash_data_len                   = 0;

  // I suspect it will confuse people if I change the default parameter behavior, so ...
  if (exe_filename.empty()) {
    static const char KIMIARU_HASH_DATA[] = "kopkl;fdsl;kl;mwekopj@pgfd[p;:kl:;,lwret;kl;kolsgfdio@pdsflkl:,rse;:l,;:lpksdfpo";
    
    hash_data_len = sizeof(KIMIARU_HASH_DATA) - 1;
    memcpy(hash_data, KIMIARU_HASH_DATA, hash_data_len);
  } else {
    string game = as::stringtol(as::get_file_prefix(exe_filename, true));

    // Figure out if there's a new pattern after the next game
    if (game == "arcana") {
      static const char ARCANA_HASH_DATA[] = "alkanasdajklgsdfjldfsjjklasjklsdajkladsdjklssjklgjgfsdadawwqq3dfs-^";

      hash_data_len = sizeof(ARCANA_HASH_DATA) - 1;
      memcpy(hash_data, ARCANA_HASH_DATA, hash_data_len);

    // There is, but feeling lazy... maybe next time.
    } else if (game == "sin3") {
      static const char SIN3_HASH_DATA[] = "sin3;lkdfsm;fdsjlketwlnkgfnvxcjlk:dcjkl:sdgn,mksdfj;lae@pk]aer@pre@pksdfmklsglmk";

      hash_data_len = sizeof(SIN3_HASH_DATA) - 1;
      memcpy(hash_data, SIN3_HASH_DATA, hash_data_len);
    } else {
      find_hash_data(exe_filename, hash_data, hash_data_len);
    }
  }

  int fd     = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  int out_fd = as::open_or_die(out_filename, 
                               O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
                               S_IREAD | S_IWRITE);

  unsigned char* buff     = new unsigned char[BLOCK_SIZE];
  unsigned char* out_buff = new unsigned char[BLOCK_SIZE];
  unsigned long  len  = 0;

  for (unsigned long i = 0; (len = read(fd, buff, BLOCK_SIZE)) != 0; i++) {
    decrypt_block(i, buff, len, out_buff, hash_data, hash_data_len);
    write(out_fd, out_buff, len);
  }

  delete [] out_buff;
  delete [] buff;

  close(fd);

  return 0;
}

