// decrddns2.cpp, v1.03 2012/08/31
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool decrypts various *.ns2 archives; use exns2 to extract them
// afterwards.

#include "as-util.h"
#include "openssl/md5.h"

struct game_info_t {  
  string key;
  string name;
};

static const game_info_t GAME_INFO[] = {
  { "kleqrsfksafgwe;mlsdaydream]sad:.]df@;cgp@[rtmnlmgviojfgjiosdmkl:skp@dskp@vb,l;]fdhl;m]dsgkndffgh", "妄想具現化AVG デイドリーム・ビリーバー" },
  { "sdajklfdsknlfkangokudsnmadfsnklasdasdjklsdajklgdnmlasdfnklasasdklmdsajklgvfsdsdasadsadsadwqerdsf", "姦獄島" },
  { "sdaldasjklgfrunedprincesss:;ghfklwernklgfnklsdfkl;asdkasdmlfdsmklgmklgml;gsdl;m,dasf;lkasdkl;asd", "烙印姫ルーンドプリンセス" },
  { "p@yuk;:hfgklgfdjkldsjopdsrserika2ml,dfmk;fdslm;dfsl;fdsldfdwg;k:fg;:fd;:fhlgfl;:[;:ds;:dsfl:fdss", "触装天使セリカ2" },
};

static const unsigned long GAME_COUNT = sizeof(GAME_INFO) / sizeof(GAME_INFO[0]);

void unobfuscate(unsigned char* buff, 
                 unsigned long  len, 
                 unsigned char* out_buff,
                 unsigned char* key_buff)
{
  static const unsigned long BLOCK_SIZE = 32;

  unsigned long block_count = len / BLOCK_SIZE;

  for (unsigned long i = 0; i < block_count; i++) {
    unsigned char* in1  = buff + i * BLOCK_SIZE;
    unsigned char* in2  = in1 + 16;

    unsigned char* out1 = out_buff + i * BLOCK_SIZE;
    unsigned char* out2 = out1 + 16;

    unsigned char* key1 = key_buff;
    unsigned char* key2 = key_buff + 48;

    unsigned char seed[16 + 48];
    memcpy(seed, in2, 16);
    memcpy(seed + 16, key1, 48);

    MD5_CTX        ctx;
    unsigned char* hash = (unsigned char*) &ctx.A;

    MD5_Init(&ctx);
    MD5_Update(&ctx, seed, sizeof(seed));

    unsigned char temp[16];
    for (unsigned long i = 0; i < 16; i++) {
      temp[i] = seed[i] = hash[i] ^ in1[i];
    }

    memcpy(seed + 16, key2, 48);

    MD5_Init(&ctx);
    MD5_Update(&ctx, seed, sizeof(seed));
    
    for (unsigned long i = 0; i < 16; i++) {
      out1[i] = seed[i] = hash[i] ^ in2[i];
    }

    memcpy(seed + 16, key1, 48);

    MD5_Init(&ctx);
    MD5_Update(&ctx, seed, sizeof(seed));

    for (unsigned long i = 0; i < 16; i++) {
      out2[i] = hash[i] ^ temp[i];
    }
  }
}

static const unsigned long BUFFER_SIZE = 1024 * 1024;

int main(int argc, char** argv) {
  if (argc < 4) {
    fprintf(stderr, "decrddns2 v1.03 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.ns2> <output.ns2> [game index]\n", argv[0]);

    for (unsigned long i = 0; i < GAME_COUNT; i++) {
      fprintf(stderr, "\tgame index = %d -> %s\n", i, GAME_INFO[i].name.c_str());
    }

    return -1;
  }

  string        in_filename  = argv[1];  
  string        out_filename = argv[2];  
  unsigned long game_index   = atol(argv[3]);

  if (game_index >= GAME_COUNT) {
    fprintf(stderr, "Unknown game index: %d\n", game_index);
    return -1;
  }

  game_info_t game_info = GAME_INFO[game_index];
  
  int fd     = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  int out_fd = as::open_or_die(out_filename, 
                               O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
                               S_IREAD | S_IWRITE);

  unsigned char* buff     = new unsigned char[BUFFER_SIZE];
  unsigned char* out_buff = new unsigned char[BUFFER_SIZE];
  unsigned long  len  = 0;

  while ((len = read(fd, buff, BUFFER_SIZE)) != 0) {
    unobfuscate(buff, len, out_buff, (unsigned char*) game_info.key.c_str());
    write(out_fd, out_buff, len);
  }

  delete [] out_buff;
  delete [] buff;

  close(out_fd);
  close(fd);

  return 0;
}

