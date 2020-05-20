// excrxb.cpp, v1.02 2012/06/16
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts CRXB (*.crm) archives.

#include "as-util.h"
#include "crx-common.h"
#include <map>

struct CRXBHDR {
  unsigned char  signature[4]; // "CRXB"
  unsigned long  unknown1;
  unsigned long  entry_count;
  unsigned short width;
  unsigned short height;
};

struct CRXBENTRY {
  unsigned long offset;
  unsigned long unknown1; // obfuscated length?
  char          filename[24];
};

struct CRXDHDR {
  unsigned char signature[4]; // "CRXD"
  unsigned long base_index;
  unsigned long base_offset;
  char          base_filename[20];
};

struct CRXJHDR {
  unsigned char signature[4]; // "CRXJ"
  unsigned long index;
  unsigned long offset;
};

typedef std::map<unsigned long, CRXBENTRY> offset_to_entry_t;

unsigned long get_length(offset_to_entry_t&          offset_to_entry,
                         offset_to_entry_t::iterator curr, 
                         unsigned long               max_offset)
{
  offset_to_entry_t::iterator next = curr;
  next++;

  if (next == offset_to_entry.end()) {
    return max_offset - curr->second.offset;
  } else {
    return next->second.offset - curr->second.offset;
  }
}

bool proc_crxd(offset_to_entry_t& offset_to_entry,
               unsigned char*     crxb_buff,
               unsigned long      crxb_len,
               const string&      prefix,
               const string&      filename,
               unsigned char*     buff,
               unsigned long      len,
               string             use_base = "")
{
  if (len < 4 || memcmp(buff, "CRXD", 4)) {
    return false;
  }

  CRXDHDR* hdr = (CRXDHDR*) buff;
  buff += sizeof(*hdr);
  len  -= sizeof(*hdr);

  if (use_base.empty()) {
    use_base = prefix + hdr->base_filename;
  }

  string base_prefix   = as::get_file_prefix(use_base) + "+";
  string base_filename = as::get_file_prefix(use_base) + ".bmp";

  bool done = false;

  if (len < 4 || !memcmp(buff, "CRXJ", 4)) {
    CRXJHDR*       crxj       = (CRXJHDR*) buff;
    unsigned long  delta_len  = get_length(offset_to_entry, offset_to_entry.find(crxj->offset), crxb_len);
    unsigned char* delta_buff = crxb_buff + crxj->offset;

    done = proc_crxd(offset_to_entry,
                     crxb_buff,
                     crxb_len,
                     prefix,
                     filename,
                     delta_buff,
                     delta_len,
                     base_filename);

    done = done || proc_crxg(filename,
                             as::get_file_prefix(filename) + ".bmp",
                             delta_buff,
                             delta_len,
                             base_filename);

  } else if (len < 4 || !memcmp(buff, "CRXG", 4)) {
    done = proc_crxg(filename,
                     as::get_file_prefix(filename) + ".bmp",
                     buff,
                     len,
                     base_filename);
  } 
  
  if (!done) {
    as::write_file(filename, buff, len);
  }

  return true;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "excrxb v1.02 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.crm>\n", argv[0]);

    return -1;
  }

  string in_filename(argv[1]);
  string prefix = as::get_file_prefix(in_filename) + "+";

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  unsigned long  crxb_len  = as::get_file_size(fd);
  unsigned char* crxb_buff = new unsigned char[crxb_len];
  read(fd, crxb_buff, crxb_len);

  CRXBHDR*   hdr     = (CRXBHDR*) crxb_buff;
  CRXBENTRY* entries = (CRXBENTRY*) (hdr + 1);

  if (memcmp(hdr->signature, "CRXB", 4)) {
    fprintf(stderr, "%s: not a CRXB archive (try excircuspck)\n", in_filename.c_str());
    return 0;
  }

  offset_to_entry_t offset_to_entry;

  for (unsigned long i = 0; i < hdr->entry_count; i++) {
    offset_to_entry[entries[i].offset] = entries[i];
  }

  for (offset_to_entry_t::iterator i = offset_to_entry.begin();
       i != offset_to_entry.end();
       ++i)
  {
    string         filename = prefix + i->second.filename;
    unsigned long  len      = get_length(offset_to_entry, i, crxb_len);
    unsigned char* buff     = crxb_buff + i->second.offset;    

    bool done = proc_crxd(offset_to_entry, 
                          crxb_buff, 
                          crxb_len,
                          prefix,
                          filename,
                          buff, 
                          len);

    done = done || proc_crxg(filename,
                             as::get_file_prefix(filename) + ".bmp",
                             buff,
                             len);

    if (!done) {
      as::write_file(filename, buff, len);
    }
  }

  delete [] crxb_buff;

  close(fd);

  return 0;
}
