// exyatpkg2.cpp, v1.0 2013/02/22
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.pkg archives used by 星彩のレゾナンス.

#include "as-util.h"

struct game_info_t {
  string   name;
  uint32_t key[8];
};

std::array<game_info_t, 1> GAME_INFO = {{ 
  { "星彩のレゾナンス ", { 0xA13BB527, 0x879FDA11, 0x72FDADBC, 0x1004A4D3, 0x03A0FFB2, 0x21CC32BA, 0x973A2B1C, 0xF7E8E667 } }
}};

static const auto GAME_CHOICES = as::choices_by_index_t<game_info_t>::init(GAME_INFO);

struct PKGHDR {
  uint32_t file_length;
  uint32_t entry_count;
};

struct PKGENTRY {
  char     filename[116];
  uint32_t length;
  uint32_t offset;
  uint32_t obfuscated_length;
};

void unobfuscate_toc(uint8_t*           buff,
                     uint32_t           len,
                     const game_info_t& info)
{
  auto key_buff = (uint8_t*) &info.key;
  uint32_t key_len  = sizeof(info.key);

  for (uint32_t i = 0; i < len; i++) {
    buff[i] ^= key_buff[i % key_len];
  }
}

void unobfuscate_data(uint8_t*           buff,
                      uint32_t           len,
                      uint32_t           obfuscated_len,
                      const game_info_t& info)
{
  if (obfuscated_len == 0 || obfuscated_len > len) {
    obfuscated_len = len;
  }

  auto     words     = (uint32_t*) buff;
  // Caller pads so we can avoid remainder hassle
  uint32_t words_len = (obfuscated_len + 3) / 4;
  uint32_t retarded  = (len / 4) & 7;

  for (uint32_t i = 0; i < words_len; i++) {
    words[i] ^= info.key[i & retarded];
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "exyatpkg2 v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pkg> <choice>\n", argv[0]);
    GAME_CHOICES.print();

    return -1;
  }

  string      in_filename = argv[1];
  const auto& info        = GAME_CHOICES.get(argv[2]);
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  PKGHDR hdr;
  read(fd, &hdr, sizeof(hdr));
  hdr.file_length ^= info.key[0];
  hdr.entry_count ^= info.key[0];

  auto     entries     = new PKGENTRY[hdr.entry_count];
  uint32_t entries_len = sizeof(PKGENTRY) * hdr.entry_count;
  read(fd, entries, entries_len);
  unobfuscate_toc((uint8_t*)entries, entries_len, info);

  for (uint32_t i = 0; i < hdr.entry_count; i++) {
    uint32_t len        = entries[i].length;
    auto     buff       = new uint8_t[(len + 3) & ~3];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);
    unobfuscate_data(buff, len, entries[i].obfuscated_length, info);

    as::write_file(entries[i].filename, buff, len);

    delete [] buff;
  }

  delete [] entries;

  close(fd);

  return 0;
}
