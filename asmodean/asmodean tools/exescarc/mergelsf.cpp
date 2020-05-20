// mergelsf.cpp, v1.0 2010/12/28
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Merges images specified by an LSF (*.lsf) into a multilayer Photoshop image.
// This requires ImageMagick convert on your path.

#include "as-util.h"
#include <map>
#include <list>

struct LSFHDR {
  unsigned char  signature[4]; // "LSF"
  unsigned long  unknown1;
  unsigned short unknown2;
  unsigned short entry_count;
  unsigned long  canvas_width;
  unsigned long  canvas_height;
  unsigned long  unknown3;
  unsigned long  unknown4;
};

struct LSFENTRY {
  char           name[64];
  unsigned long  x;
  unsigned long  y;
  unsigned long  cx;
  unsigned long  cy;
  unsigned long  unknown1;
  unsigned long  unknown2;
  unsigned char  group;
  unsigned char  layer;
  unsigned short id;
};

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "mergelsf v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.lsf> [output.psd]\n", argv[0]);
    return -1;
  }

  string in_filename  = argv[1];
  string out_filename = as::get_file_prefix(in_filename) + ".psd";

  if (argc > 2) {
    out_filename = argv[2];
  }

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY); 

  LSFHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  LSFENTRY* entries = new LSFENTRY[hdr.entry_count];
  read(fd, entries, sizeof(LSFENTRY) * hdr.entry_count);

  typedef std::list<LSFENTRY> entry_list_t;
  typedef std::map<unsigned long, entry_list_t> layer_to_entries_t;
  typedef std::map<unsigned long, layer_to_entries_t> group_to_layers_t;

  group_to_layers_t groups;

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    groups[entries[i].group][entries[i].layer].push_back(entries[i]);
  }

  string cmd = "convert -background transparent rose:";

  for (group_to_layers_t::iterator i = groups.begin();
       i != groups.end();
       ++i) 
  {
    for (layer_to_entries_t::iterator j = i->second.begin();
         j != i->second.end();
         ++j)
    {
      for (entry_list_t::iterator k = j->second.begin();
           k != j->second.end();
           ++k)
      {
        cmd += as::stringf(" ( -set label %d_%d_%02d -page +%d+%d %s.png -mosaic )", k->group, k->layer, k->id, k->x, k->y, k->name);
      }
    }
  }

  cmd += as::stringf(" -extent %dx%d %s", hdr.canvas_width, hdr.canvas_height, out_filename.c_str());

  system(cmd.c_str());

  delete [] entries;

  close(fd);

  return 0;
}