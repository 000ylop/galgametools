// mergelsf2.cpp, v1.0 2012/12/21
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Merges images specified by an LSF (*.lsf) into a multiple separate images.

#include "as-util.h"

struct LSFHDR {
  uint8_t  signature[4]; // "LSF"
  uint32_t unknown1;
  uint16_t unknown2;
  uint16_t entry_count;
  uint32_t canvas_width;
  uint32_t canvas_height;
  uint32_t unknown3;
  uint32_t unknown4;
};

struct LSFENTRY {
  char     name[64];
  uint32_t x;
  uint32_t y;
  uint32_t cx;
  uint32_t cy;
  uint32_t unknown1;
  uint32_t unknown2;
  uint8_t  group;
  uint8_t  layer;
  uint16_t id;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "mergelsf2 v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.lsf>\n", argv[0]);
    return -1;
  }

  string in_filename = argv[1];

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY); 

  LSFHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  LSFENTRY* entries = new LSFENTRY[hdr.entry_count];
  read(fd, entries, sizeof(LSFENTRY) * hdr.entry_count);

  typedef list<LSFENTRY> entry_list_t;
  typedef map<uint32_t, entry_list_t> layer_to_entries_t;
  typedef map<uint32_t, layer_to_entries_t> group_to_layers_t;

  group_to_layers_t groups;

  for (uint32_t i = 0; i < hdr.entry_count; i++) {
    groups[entries[i].group][entries[i].layer].push_back(entries[i]);
  }

  as::used_files_t used("parts");

  for (auto& group_val : groups) {
    auto& group  = group_val.second;
    auto& bases  = group[0];
    auto& parts  = group[1];

    for (auto& base : bases) {
      for (auto& part : parts) {
        string base_prefix  = base.name;
        string part_prefix  = part.name;
        string out_filename = base_prefix + "+" + part_prefix;

        as::image_t base_image(base_prefix + ".png");
        base_image.set_offset(base.x, base.y);

        as::image_t part_image(part_prefix + ".png");
        part_image.set_offset(part.x, part.y);

        as::image_t image(hdr.canvas_width, hdr.canvas_height, 4);
        image.blend(base_image);
        image.blend(part_image);
        image.write(out_filename + ".bmp");

        used.add(part_prefix + ".png");
        used.add(part_prefix + ".bmp");

        for (auto& layers : group) {
          if (layers.first != 0 &&
              layers.first != 1 &&
              layers.first != 255)
          {
            for (auto& part2 : layers.second) {
              string part2_prefix  = part2.name;
              string out_filename2 = out_filename + "+" + part2_prefix;

              as::image_t part2_image(part2_prefix + ".png");
              part2_image.set_offset(part2.x, part2.y);

              as::image_t image2 = image;
              image2.blend(part2_image);
              image2.write(out_filename2 + ".bmp");

              used.add(part2_prefix + ".png");
              used.add(part2_prefix + ".bmp");
            }
          }
        }
      }
    }
  }

  used.done();

  delete [] entries;

  close(fd);

  return 0;
}
