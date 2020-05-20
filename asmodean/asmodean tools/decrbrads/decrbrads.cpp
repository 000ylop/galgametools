// decrbrads.cpp, v1.01 2010/01/29
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool decrypt's BLACKRAINBOW's encrypted *.ads archives.
// Use exbrdat to extract afterwards.

#include "as-util.h"
#include "openssl/md5.h"
#include "openssl/sha.h"
#include "openssl/rc4.h"
#include "openssl/hmac.h"

struct key_data_t {
  string        game_name;
  unsigned char key[256];
};

// Do we care how/if the keys are generated?  Not really.
static const key_data_t KEY_DATA[] = {
  { "�����V�Y2",
    { 0x3F, 0xFB, 0xCE, 0x67, 0x39, 0x90, 0x4A, 0xE6, 0xBA, 0x03, 0x46, 0xF2, 0xB2, 0x2A, 0x46, 0x9F, 
      0x1C, 0x68, 0xE8, 0x93, 0x74, 0x6F, 0x86, 0xB6, 0x17, 0x68, 0x00, 0x50, 0xD1, 0xC5, 0x3D, 0x64, 
      0xC9, 0x2D, 0x8E, 0x96, 0x2C, 0xE4, 0x8E, 0x09, 0x7A, 0xD1, 0xFE, 0x60, 0xEE, 0x14, 0x9A, 0xAA, 
      0x25, 0x56, 0xD9, 0x3A, 0x20, 0xE3, 0xF3, 0x31, 0x44, 0x95, 0xB5, 0x6F, 0x0A, 0xFC, 0xE4, 0x17, 
      0x1F, 0xF7, 0xFD, 0x9A, 0x9C, 0xBB, 0xE9, 0x62, 0x82, 0xB2, 0xD2, 0x07, 0xA2, 0x73, 0xDB, 0x99, 
      0x03, 0x2C, 0xFA, 0xFC, 0x30, 0x07, 0x88, 0xD7, 0x78, 0xBA, 0xA7, 0xEA, 0xD6, 0x1F, 0x22, 0xE5, 
      0x57, 0x1F, 0xC6, 0xEB, 0x22, 0xCE, 0x4F, 0x36, 0xE8, 0xCA, 0x17, 0x37, 0xB3, 0xEF, 0x12, 0x94, 
      0xC6, 0x25, 0x3C, 0x9C, 0xD3, 0x7C, 0xE1, 0x4D, 0x5F, 0xB6, 0x2E, 0x4C, 0x59, 0xF0, 0xA6, 0x2C, 
      0x52, 0xD5, 0xA6, 0x2A, 0x9D, 0x46, 0x65, 0x0D, 0x83, 0x76, 0x70, 0x8F, 0xEC, 0xEF, 0x81, 0x30, 
      0xF4, 0xC2, 0x4B, 0x34, 0xD3, 0xFD, 0x5C, 0x4A, 0x23, 0x47, 0x55, 0x1F, 0x34, 0xE9, 0xD9, 0x8A, 
      0x2F, 0x36, 0xC5, 0xE8, 0xF3, 0x1F, 0x04, 0xEC, 0xEE, 0x13, 0x85, 0x7A, 0xFD, 0x46, 0x03, 0xF1, 
      0xFA, 0x86, 0x0D, 0x91, 0x78, 0x67, 0xC8, 0x07, 0x1D, 0x55, 0xFD, 0xAF, 0x02, 0x4B, 0x5D, 0xF6, 
      0xE0, 0x24, 0x95, 0x85, 0xA0, 0x73, 0x4A, 0x8D, 0xBB, 0x7A, 0x8C, 0x86, 0x74, 0xDA, 0x86, 0x3E, 
      0xA4, 0x81, 0x10, 0x7D, 0x26, 0x1A, 0xF0, 0xF7, 0x0E, 0x0B, 0xF0, 0x9B, 0x01, 0x2D, 0xF4, 0x56, 
      0xB3, 0xEB, 0x17, 0x93, 0x8F, 0x6A, 0x7D, 0xF4, 0xC0, 0x06, 0x16, 0xF2, 0x4A, 0x13, 0x20, 0xC7, 
      0xDE, 0x7B, 0x7B, 0x9E, 0x40, 0x22, 0xBF, 0x9D, 0x53, 0x67, 0x83, 0x9D, 0xA4, 0x31, 0x74, 0x4F, } },

  { "�[�ԍÖ� �̌���",
    { 0xE0, 0xFE, 0xB3, 0xA0, 0x67, 0x7B, 0x9A, 0x8C, 0x20, 0x32, 0xCA, 0x6C, 0x63, 0xEF, 0x97, 0x84, 
      0xEF, 0x76, 0xFD, 0x80, 0x02, 0xD3, 0x5E, 0xA4, 0x18, 0x1B, 0xB2, 0xAC, 0x47, 0xA0, 0xE3, 0xBC, 
      0x16, 0xE8, 0xE5, 0x18, 0x19, 0xF5, 0x14, 0x5C, 0x87, 0x2D, 0xAC, 0x7C, 0x5E, 0x8A, 0x88, 0x34, 
      0x3A, 0xC2, 0xDE, 0x58, 0x27, 0x70, 0x28, 0xAC, 0xE9, 0x5F, 0x14, 0x83, 0xE4, 0x82, 0xCC, 0xE0, 
      0x3C, 0x19, 0xDA, 0xAC, 0x02, 0x7F, 0x53, 0x8F, 0x93, 0x68, 0x23, 0xA0, 0xE3, 0x5D, 0x28, 0x2B, 
      0x07, 0xD5, 0xFD, 0x27, 0x4A, 0xD7, 0xBF, 0x23, 0xC0, 0x9F, 0x54, 0x2A, 0xA4, 0x51, 0x4B, 0xAA, 
      0xBB, 0x59, 0xA5, 0xA6, 0x5D, 0xFB, 0xA7, 0x01, 0x07, 0xD3, 0x7E, 0x00, 0xCF, 0xF9, 0x3B, 0x08, 
      0xCA, 0xB1, 0x9E, 0x0F, 0xE7, 0x53, 0x58, 0xF2, 0x0A, 0x0B, 0xF5, 0xD9, 0x66, 0x21, 0x9C, 0xD9, 
      0x6F, 0x28, 0x15, 0xD5, 0x49, 0xE9, 0xAF, 0xA1, 0x7A, 0x50, 0xAC, 0x82, 0x4A, 0x6A, 0xA8, 0xDB, 
      0x5F, 0x22, 0xBD, 0x3C, 0x3D, 0x80, 0xD3, 0x51, 0x3A, 0x8B, 0x84, 0x39, 0xFE, 0xC0, 0x24, 0x0A, 
      0xDF, 0x58, 0x75, 0x87, 0x81, 0x79, 0x77, 0xC2, 0xCA, 0x31, 0x1D, 0xE5, 0xE2, 0x0E, 0x11, 0xEA, 
      0x51, 0x07, 0xA2, 0x75, 0x48, 0xD5, 0x18, 0x13, 0x6B, 0x6D, 0x8B, 0xBA, 0xFE, 0x44, 0x23, 0x1B, 
      0xB5, 0xE3, 0x4B, 0x76, 0xBB, 0x02, 0x10, 0xF3, 0xEA, 0x89, 0x11, 0x62, 0xCD, 0x90, 0xD5, 0x2C, 
      0x1B, 0xC8, 0xF2, 0xA2, 0xAA, 0xBB, 0xFC, 0x05, 0x0E, 0xE2, 0xD9, 0x52, 0x2D, 0xF5, 0xD9, 0x0C, 
      0x6F, 0xFE, 0x39, 0x11, 0xA9, 0xBF, 0x5A, 0x03, 0xB1, 0xA6, 0x75, 0x50, 0xA8, 0xED, 0x5A, 0x9A, 
      0x25, 0xFA, 0x86, 0x2C, 0x6C, 0xD9, 0x9D, 0x22, 0x69, 0xD6, 0x9B, 0x3B, 0xCC, 0xAE, 0x1C, 0xD8, } },
  
  { "�[�ԍÖ�",
    { 0x88, 0xB1, 0x57, 0xCE, 0xA6, 0x1F, 0x5A, 0xBB, 0xAA, 0x51, 0x55, 0xC2, 0x8F, 0xA2, 0x1D, 0x71, 
      0xE5, 0x6D, 0x19, 0x91, 0x88, 0x44, 0x3E, 0x33, 0xD3, 0x80, 0x84, 0x4B, 0xF0, 0xD4, 0xA7, 0x64, 
      0x58, 0xDB, 0x66, 0xB5, 0x83, 0x0B, 0x46, 0xB5, 0x39, 0x44, 0xCA, 0xF3, 0x8E, 0x81, 0x3F, 0xB3, 
      0xC3, 0x5B, 0x35, 0x9D, 0xDA, 0x61, 0xA0, 0x9A, 0xD6, 0x6E, 0x03, 0x97, 0xFB, 0x4C, 0x7B, 0xB5, 
      0xA6, 0xC3, 0x04, 0x71, 0xAA, 0x3E, 0x59, 0x12, 0xAB, 0xFF, 0x50, 0x8F, 0xE1, 0x37, 0x02, 0xD2, 
      0xF6, 0x4B, 0x05, 0x38, 0xF4, 0x4F, 0x25, 0x9C, 0x56, 0x26, 0xA1, 0xDA, 0xFE, 0xA1, 0x42, 0x17, 
      0xBD, 0xE7, 0x44, 0xBD, 0xBE, 0xC9, 0x8A, 0x26, 0xB3, 0x5A, 0x0B, 0xE7, 0xF9, 0xB5, 0x63, 0x69, 
      0x1C, 0x34, 0xE8, 0xC1, 0x50, 0x97, 0x8D, 0xA8, 0x79, 0x5B, 0x84, 0xA8, 0x2F, 0x76, 0xE2, 0xDE, 
      0x17, 0x21, 0xF3, 0x17, 0xC9, 0xBA, 0xB3, 0x9F, 0xC0, 0xFF, 0x18, 0x61, 0x60, 0x01, 0x15, 0x89, 
      0x6A, 0x7F, 0x03, 0x84, 0xA1, 0xFD, 0x4D, 0x0D, 0xE5, 0x4C, 0x52, 0xB1, 0x88, 0x77, 0x79, 0x8A, 
      0x86, 0x4B, 0x75, 0x3E, 0x8A, 0xC4, 0x7E, 0xAB, 0x9C, 0x1C, 0x40, 0xEE, 0xB1, 0xFC, 0x62, 0x8B, 
      0x91, 0x79, 0x63, 0xCC, 0x92, 0x3C, 0x02, 0xF8, 0xF3, 0x5A, 0x86, 0xA6, 0x7B, 0x54, 0x81, 0xF2, 
      0x5C, 0x0A, 0xAA, 0x93, 0x1D, 0x28, 0xFC, 0x05, 0x89, 0xB6, 0x7D, 0x8D, 0x43, 0x75, 0xB5, 0x8F, 
      0x0E, 0x6C, 0x91, 0x9E, 0x62, 0x75, 0xCA, 0x99, 0xE4, 0xC1, 0x33, 0x3E, 0x06, 0xCC, 0xF1, 0x3F, 
      0x0F, 0xDD, 0xF2, 0x2C, 0x26, 0x9C, 0xD5, 0x5C, 0x09, 0xAE, 0xB9, 0x76, 0x49, 0x94, 0xB5, 0x85, 
      0x22, 0x32, 0xD2, 0xE6, 0x1B, 0x2F, 0xA8, 0x87, 0x97, 0x26, 0x5A, 0xD0, 0x9A, 0x36, 0xED, 0xC2, } },
};

static const unsigned long KEY_COUNT = sizeof(KEY_DATA) / sizeof(KEY_DATA[0]);

static const unsigned long BLOCK_SIZE = 1024;

void decrypt_block(unsigned long        index, 
                   unsigned char*       buff, 
                   unsigned long        len, 
                   unsigned char*       out_buff,
                   const unsigned char* hash_data,
                   unsigned long        hash_data_len)
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

int main(int argc, char** argv) {
  if (argc < 4) {
    fprintf(stderr, "decrbrads v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.ads> <output.dat> <key index>\n", argv[0]);

    for (unsigned long i = 0; i < KEY_COUNT; i++) {
      fprintf(stderr, "\tkey index = %2d -> %s\n", i, KEY_DATA[i].game_name.c_str());
    }

    return -1;
  }

  string        in_filename(argv[1]);  
  string        out_filename(argv[2]);
  unsigned long key_index(atol(argv[3]));

  if (key_index >= KEY_COUNT) {
    fprintf(stderr, "Invalid key index: %d\n", key_index);
    return -1;
  }

  const key_data_t& key_data = KEY_DATA[key_index];

  int fd     = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  int out_fd = as::open_or_die(out_filename, 
                               O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
                               S_IREAD | S_IWRITE);

  unsigned char* buff     = new unsigned char[BLOCK_SIZE];
  unsigned char* out_buff = new unsigned char[BLOCK_SIZE];
  unsigned long  len      = 0;

  for (unsigned long i = 0; (len = read(fd, buff, BLOCK_SIZE)) != 0; i++) {
    decrypt_block(i, buff, len, out_buff, key_data.key, sizeof(key_data.key));
    write(out_fd, out_buff, len);
  }

  delete [] out_buff;
  delete [] buff;

  close(fd);

  return 0;
}
