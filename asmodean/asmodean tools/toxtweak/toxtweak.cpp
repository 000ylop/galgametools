// toxtweak.cpp, v1.0 2012/12/29
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool tweaks data in Tales of Xillia & Tales of Xillia 2 to disable
// the annoying minimap and other overlays.

#include "as-util.h"

struct HDR {
  uint32_t unknown1;
  uint32_t unknown2;
  uint32_t unknown3;
  uint32_t entry_count;
  uint32_t toc2_offset;

  void flip_endian(void) {
    as::flip_endian_multi(&entry_count,
                          &toc2_offset,
                          NULL);
  }
};

struct ENTRY2 {
  uint64_t original_length;
  uint64_t length;
  uint64_t offset;
  uint32_t id;
  uint8_t  type[8];
  uint32_t unknown1;

  void flip_endian(void) {
    as::flip_endian_longlong_multi(&length,
                                   &original_length,
                                   &offset,
                                   NULL);

    as::flip_endian_multi(&id,
                          NULL);
  }
};

struct MTEXHDR {
  uint8_t  signature[4]; // "MTEX"
  uint32_t unknown1;
  uint32_t unknown2;
  uint16_t unknown3;
  uint16_t width;
  uint16_t height;
  uint16_t unknown4;
  uint8_t  type;
  uint8_t  unknown5;
  uint16_t unknown6;
  uint32_t unknown7;

  void flip_endian(void) {
    as::flip_endian_short_multi(&width,
                                &height,
                                NULL);
  }
};

int main(int argc, char** argv) {
  if (argc != 4) {
    fprintf(stderr, "toxtweak v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <FILEHEADER.TOFHDB> <TLFILE.TLDAT> <tweaks.txt>\n\n", argv[0]);
    fprintf(stderr, "i.e., toxtweak FILEHEADER.TOFHDB TLFILE.TLDAT disable_minimap.txt\n\n");
    fprintf(stderr, "WARNING: TLFILE.TLDAT WILL BE MODIFIED!\n");
    return -1;
  }

  string toc_filename   = argv[1];
  string dat_filename   = argv[2];
  string tweak_filename = argv[3];

  struct tweak_t {
    uint32_t id;
    uint16_t width;
    uint16_t height;

    bool operator<(const tweak_t& rhs) const {
      return id < rhs.id;
    }
  };

  typedef set<tweak_t> tweaks_t;
  tweaks_t tweaks;

  FILE* fh = as::open_or_die_file(tweak_filename);

  while (true) {
    string line = as::read_line(fh, as::READ_LINE_SKIP_JUNK);

    if (line.empty()) break;
  
    tweak_t tweak;
    if (sscanf(line.c_str(), "0x%X %hu %hu", &tweak.id, &tweak.width, &tweak.height) != 3) {
      fprintf(stderr, "Cannot parse: [%s]\n", line.c_str());
      return -1;
    }

    tweaks.insert(tweak);
  }

  fclose(fh);

  int fd = as::open_or_die(toc_filename, O_RDONLY | O_BINARY); 

  HDR hdr;
  read(fd, &hdr, sizeof(hdr));
  hdr.flip_endian();

  lseek(fd, hdr.toc2_offset + 16, SEEK_SET);

  auto entries = new ENTRY2[hdr.entry_count];
  read(fd, entries, sizeof(ENTRY2) * hdr.entry_count);

  close(fd);

  fd = as::open_or_die(dat_filename, O_RDWR | O_BINARY);

  for (uint32_t i = 0; i < hdr.entry_count; i++) {
    entries[i].flip_endian();

    char temp[9] = { 0 };
    memcpy(temp, entries[i].type, 8);
    string type = temp;

    for (auto& tweak : tweaks) {
      if (tweak.id == entries[i].id) {
        if (type != "TOTEXB" || entries[i].length != sizeof(MTEXHDR)) {
          fprintf(stderr, "0x%08X is not a TOTEXB (skipped)\n", entries[i].id);
          continue;
        }

        MTEXHDR mtexhdr;
        _lseeki64(fd, entries[i].offset, SEEK_SET);
        read(fd, &mtexhdr, sizeof(mtexhdr));
        
        mtexhdr.flip_endian();
        mtexhdr.width  = tweak.width;
        mtexhdr.height = tweak.height;
        mtexhdr.flip_endian();

        _lseeki64(fd, entries[i].offset, SEEK_SET);
        write(fd, &mtexhdr, sizeof(mtexhdr));
      }
    }
  }

  delete [] entries;

  close(fd);

  return 0;
}
