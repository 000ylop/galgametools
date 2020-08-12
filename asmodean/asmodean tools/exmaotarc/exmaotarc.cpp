// exmaotarc.cpp, v1.04 2013/05/10
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts TACTICS_ARC_FILE (*.arc) archives used
// by 魔王のくせに生イキだっ！ and others.

#include "as-util.h"
#include "as-lzss.h"

struct ARCHDR {
  uint8_t signature[16]; // "TACTICS_ARC_FILE"
};

struct ARCENTRY {
  uint32_t length;
  uint32_t original_length;
  uint32_t filename_length;
  uint8_t  unknown[8]; // hash probably
  // char  filename[filename_length];
};

struct game_info_t {
  string name;
  string key;
};

static const std::array<game_info_t, 4> GAME_INFO = {{ 
  { "魔王のくせに生イキだっ！", "Puni0r4p" },
  { "あっぱれ！天下御免［祭］ ～恋と嵐は大江戸の華～", "JyEeJxwQ" },
  { "恋せよ!! 妹番長", "Klw79n6f" },
  { "幻奏童話 ALICETALE", "zv7jpFTV" },
}};    

static const auto GAME_CHOICES = as::choices_by_index_t<game_info_t>::init(GAME_INFO);

void unobfuscate(uint8_t*      buff,
                 uint32_t      len,
                 const string& key)
{
  auto     key_buff = (uint8_t*) key.c_str();
  uint32_t key_len  = key.length();

  auto end = buff + len;

  for (uint32_t i = 0; i < len; i++) {
    buff[i] ^= key[i % key_len];
  }
}

bool guess_key(const string& filename, string& key) {
  static const uint8_t MATCH[] = "log.txt";

  int      fd   =  as::open_or_die(filename, O_RDONLY | O_BINARY);
  uint32_t len  = as::get_file_size(fd);
  uint8_t* buff = new uint8_t[len];
  read(fd, buff, len);
  close(fd);

  bool found = false;

  auto end = buff + len;

  for (auto p = buff; p + sizeof(MATCH) < end; p++) {
    if (!memcmp(p, MATCH, sizeof(MATCH))) {
      p += sizeof(MATCH);
      
      while (!*p) p++;

      key   = (char*)p;
      found = true;
      break;
    }
  }

  delete [] buff;

  return found;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "exmaotarc v1.04 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.arc> <choice|game.exe>\n", argv[0]);
    GAME_CHOICES.print();

    fprintf(stderr, "\n\tchoice = game.exe -> auto\n");

    return -1;
  }

  string in_filename = argv[1];
  string game        = argv[2];

  string key;

  if (as::stringtol(game).find(".exe") != string::npos) {
    if (!guess_key(game, key)) {
      fprintf(stderr, "%s: failed to guess key\n", game.c_str());
      return -1;
    }

    fprintf(stderr, "Using auto key [%s]\n", key.c_str());
  } else {
    key = GAME_CHOICES.get(game).key;
  }

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  ARCHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  ARCENTRY entry;
  while (read(fd, &entry, sizeof(entry)) == sizeof(entry)) {
    if (!entry.length) {
      continue;
    }

    auto filename = new char[entry.filename_length + 1];
    memset(filename, 0, entry.filename_length + 1);
    read(fd, filename, entry.filename_length);

    uint32_t len  = entry.length;
    auto     buff = new uint8_t[len];
    read(fd, buff, len);
    unobfuscate(buff, len, key);

    if (entry.original_length) {
      uint32_t temp_len  = entry.original_length;
      auto     temp_buff = new uint8_t[temp_len];
      as::unlzss(buff, len, temp_buff, temp_len);

      delete [] buff;

      len  = temp_len;
      buff = temp_buff;
    }

    as::make_path(filename);
    as::write_file(filename, buff, len);

    delete [] buff;
    delete [] filename;
  }

  close(fd);

  return 0;
}
