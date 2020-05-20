// exar2evd.cpp, v1.0 2009/01/22
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Extracts/rebuilds EVD scripts used by Ar Tonelico 2.

#include "as-util.h"

//#define DEBUG

struct EVDHDR {
  unsigned long entry_count;
};

struct EVDENTRY {
  unsigned long position;
  unsigned long name_id;
  unsigned long portrait_id;   // different expressions, 0 = no portrait
  unsigned long id;
  unsigned long portrait_type; // ??
  unsigned long length;
};

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "exar2evd v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.evd> <script.txt> [output.evd]\n", argv[0]);
    return -1;
  }

  string evd_filename(argv[1]);
  string scr_filename(argv[2]);
  string out_filename;

  if (argc > 3) {
    out_filename = argv[3];
  }

  bool  do_rebuild = !out_filename.empty();
  int   fd         = as::open_or_die(evd_filename, O_RDONLY | O_BINARY);
  int   out_fd     = -1;
  
  // stdio is more convenient for text
  FILE* scr_fh = fopen(scr_filename.c_str(), do_rebuild ? "r" : "w");
  if (!scr_fh) {
    fprintf(stderr, "Could not open %s\n", scr_filename.c_str());
    return -1;
  }

  unsigned long trl_len = as::get_file_size(fd);

  EVDHDR hdr;
  trl_len -= read(fd, &hdr, sizeof(hdr));

  if (do_rebuild) {
    out_fd = as::open_or_die(out_filename, 
                             O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                             S_IREAD | S_IWRITE);

    write(out_fd, &hdr, sizeof(hdr));
  }

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    EVDENTRY entry;
    trl_len -= read(fd, &entry, sizeof(entry));

    unsigned long text_len  = (entry.length + 3) & ~3;
    char*         text_buff = new char[text_len];
    trl_len -= read(fd, text_buff, text_len);

    string text(text_buff, text_len);

#ifdef DEBUG
    fprintf(out_fh, 
            "0x%08X 0x%08X 0x%08X 0x%08X 0x%08X [%s]",
            entry.position,
            entry.name_id,
            entry.portrait_id,
            entry.id,
            entry.portrait_type,
            text.c_str());
#endif

    if (do_rebuild) {
      char buff[4096] = { 0 };
      fgets(buff, sizeof(buff), scr_fh);

      unsigned long new_id         = 0;
      char          new_text[4096] = { 0 };
      sscanf(buff, "%d %s", &new_id, new_text);

      if (new_id != entry.id) {
        fprintf(stderr, "Error: mismatched text id (expected %d, got %d)\n", entry.id, new_id);
        return -1;
      }

      entry.length = strlen(new_text) + 1;

      unsigned long new_text_len = (entry.length + 3) & ~3;

      write(out_fd, &entry, sizeof(entry));
      write(out_fd, new_text, new_text_len);
    } else {
      fprintf(scr_fh, "%d %s\n", entry.id, text.c_str());
    }
  }

  if (do_rebuild) {
    // This data was the same between JP/EN so I presume we can just copy it here too...
    unsigned char* trl_buff = new unsigned char[trl_len];
    read(fd, trl_buff, trl_len);

    write(out_fd, trl_buff, trl_len);
    delete [] trl_buff;

    close(out_fd);
  }

  close(fd);

  return 0;
}