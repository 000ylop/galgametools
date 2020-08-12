// exhud.cpp, v1.0 2009/08/04
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts data from *.hud archives.

#include "as-util.h"

struct HUDHDR {
  unsigned char signaturep[4]; // "DX\03\00" 
  unsigned long trl_length;
  unsigned long unknown2;
  unsigned long trl_offset;
  unsigned long files_offset;
  unsigned long toc_offset;

};

struct HUDNAMEENTRY {
  unsigned short filename_words;
  unsigned short unknown1;
  char           filename[1];
};

struct HUDFILEENTRY {
  unsigned long name_offset;
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long unknown5;
  unsigned long unknown6;
  unsigned long unknown7;
  unsigned long offset;
  unsigned long length;
  unsigned long unknown8;
};

struct HUDTOCENTRY {
  unsigned long file_offset;
  unsigned long flags;
  unsigned long entry_count;
  unsigned long children_offset;
};

string get_name(unsigned char* names, HUDFILEENTRY& file) {
  HUDNAMEENTRY* name = (HUDNAMEENTRY*) (names + file.name_offset);

  char s[1024] = { 0 };
  memcpy(s, name->filename, name->filename_words * 4);

  return s;
}

void read_unobfuscate(int            fd, 
                      off_t          offset, 
                      unsigned char* buff, 
                      unsigned long  len)
{
  static unsigned char KEY[] = { 0x97, 0x57, 0xED, 0x0A, 
                                 0xCF, 0x9C, 0xCE, 0xF2, 
                                 0xAB, 0x18, 0x23, 0xFC };

  lseek(fd, offset, SEEK_SET);
  read(fd, buff, len);

  for (unsigned long i = 0; i < len; i++) {
    buff[i] ^= KEY[(offset + i) % sizeof(KEY)];
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exhud v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.hud>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  HUDHDR hdr;
  read_unobfuscate(fd, 0, (unsigned char*)&hdr, sizeof(hdr));

  unsigned long  trl_len  = hdr.trl_length;
  unsigned char* trl_buff = new unsigned char[trl_len];
  read_unobfuscate(fd, hdr.trl_offset, trl_buff, trl_len);
  
  unsigned char* names       = trl_buff;
  unsigned char* files       = trl_buff + hdr.files_offset; 
  HUDTOCENTRY*   entries     = (HUDTOCENTRY*) (trl_buff + hdr.toc_offset);
  unsigned long  entry_count = (trl_len - hdr.toc_offset) / sizeof(HUDTOCENTRY);
  string         base;

  for (unsigned long i = 0; i < entry_count; i++) {
    HUDFILEENTRY* dir     = (HUDFILEENTRY*) (files + entries[i].file_offset);
    string        dirname = get_name(names, *dir);    

    string path;
    if (entries[i].flags == 0x00000040) {
      path = base + dirname + "/";
    } else {
      base = dirname.empty() ? "" : (dirname + "/");
      path = base;
    }

    HUDFILEENTRY* children = (HUDFILEENTRY*) (files + entries[i].children_offset);

    for (unsigned long j = 0; j < entries[i].entry_count; j++) {
      string out_name = path + get_name(names, children[j]);

      if (children[j].length) {
        unsigned long  len  = children[j].length;
        unsigned char* buff = new unsigned char[len];
        read_unobfuscate(fd, sizeof(hdr) + children[j].offset, buff, len);

        as::make_path(out_name);
        as::write_file(out_name, buff, len);

        delete [] buff;
      }
    }
  }

  return 0;
}