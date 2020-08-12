// merge_mpark2.cpp, v1.01 2013/05/14
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges delta images from Monster Park 2.
// Convert jpeg bases to bitmaps first...

#include "as-util.h"

struct game_info_t {
  string key;
  string name;
};

std::array<game_info_t, 3> GAME_INFO = {{ 
  { "isyukanin", "MONSTER PARK 2～神々を宿した乙女～" },
  { "ikawork4441", "つぼい君のスイッチ！" },
  { "MONS_GA_IPPAI", "モンスター･ハザード" },
}};

static const auto GAME_CHOICES = as::choices_by_index_t<game_info_t>::init(GAME_INFO);

struct SCRHDR {
  uint32_t length;
  uint32_t unknown1;
  uint32_t entry_count;
  uint32_t unknown2;
};

void unobfuscate(const game_info_t& info,
                 uint8_t*           buff, 
                 uint32_t           len)
{
  uint32_t key_len  = info.key.length();
  auto     key_buff = (uint8_t*) info.key.c_str();

  for (unsigned long j = 0; j < len; j++) {
    buff[j] += key_buff[(j + 16) % key_len];
  }
}

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "merge_mpark2 v1.01 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input> <choice> [-single]\n\n", argv[0]);
    fprintf(stderr, "\t<input> should be _BGSET or _SPRSET extracted from md_scr.med.\n\n");
    GAME_CHOICES.print();
    return -1;
  }

  string      in_filename = argv[1];
  const auto& info        = GAME_CHOICES.get(argv[2]);
  bool        single      = argc > 3 && !strcmp(argv[3], "-single");

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  SCRHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  uint32_t len  = hdr.length;
  auto     buff = new uint8_t[len];
  read(fd, buff, len);
  unobfuscate(info, buff, len);

  as::make_path("used/");
  as::make_path("merged/");

  typedef set<string> used_t;
  used_t used;

  auto p = (char*) buff;

  for (unsigned long i = 0; i < hdr.entry_count;) {
    if (p[0] == '{') {
      p += strlen(p) + 1;
      i++;

      typedef map<string, list<string>> data_t;
      data_t data;

      while (p[0] != '}') {
        auto fields = as::splitstr(p, ":");
        data[fields[0]].push_back(fields[1]);

        p += strlen(p) + 1;
        i++;
      }

      auto   fields = as::splitstr(*data["PREFIX"].begin(), ",");
      string prefix = fields[1];

      data["PARTS"].splice(data["PARTS"].end(), data["FACE"]);

      for (const auto& base : data["BODY"]) {
        fields = as::splitstr(base, ",");
        
        string out_prefix    = prefix + fields[1];
        string base_filename = out_prefix + ".bmp";

        for (const auto& part : data["PARTS"]) {
          fields = as::splitstr(part, ",");

          string part_prefix   = prefix + fields[1];
          string part_filename = part_prefix + ".bmp";
          string out_filename  = "merged/" + out_prefix + "+" + part_prefix + ".bmp";

          uint32_t offset_x = 0;
          uint32_t offset_y = 0;

          if (fields.size() > 2) {
            offset_x = atol(fields[2].c_str());
            offset_y = atol(fields[3].c_str());
          }

          if (!as::is_file_readable(base_filename) || !as::is_file_readable(base_filename)) {
            continue;
          }

          if (single && used.find(base_filename) != used.end()) {
            continue;
          }

          as::image_t base_image(base_filename);
          as::image_t part_image(part_filename);
          part_image.set_offset(offset_x, offset_y);
          base_image.blend(part_image);

          base_image.write(out_filename);

          used.insert(base_filename);
          used.insert(part_filename);
        }        
      }
    }
    
    p += strlen(p) + 1;
    i++;
  }

  for (used_t::iterator i = used.begin();
       i != used.end();
       ++i) {
    rename(i->c_str(), ("used/" + *i).c_str());
  }

  delete [] buff;

  close(fd);
  
  return 0;
}
