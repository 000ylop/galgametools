// exwhaledat.cpp, v1.1 2012/10/01
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts Whale's *.dat archives.

#include "as-util.h"
#include "zlib.h"

// 2010/08/22 ~ 2012/01/20
// #define VERSION 1

// 2012/09/28 ~
#define VERSION 2

#if VERSION == 1
typedef uint32_t hash_t;

inline hash_t hash_buff(const void* buff, uint32_t len) {
  return crc32(0, (const Bytef*)buff, len);
}
#else
#include <boost/crc64.h>

typedef uint64_t hash_t;

inline hash_t hash_buff(const void* buff, uint32_t len) {
  return boost::crc(buff, len).crc0;
}
#endif

#pragma pack(1)
struct DATHDR {
  uint32_t entry_count;

  void flip_endian(void) {
#if VERSION == 1
    entry_count = as::flip_endian(entry_count);    
#endif
  }
};

struct DATENTRY {
  hash_t   hash;
  uint8_t  type;
  uint32_t offset;
  uint32_t length;
  uint32_t original_length;

  void flip_endian(void) {
#if VERSION == 1
    hash            = as::flip_endian(hash);
    offset          = as::flip_endian(offset);
    length          = as::flip_endian(length);
    original_length = as::flip_endian(original_length);
#endif
  }
};

static const uint32_t DATENTRY_TYPE_PLAIN      = 0;
static const uint32_t DATENTRY_TYPE_OBFUSCATED = 1;
static const uint32_t DATENTRY_TYPE_COMPRESSED = 2;

#pragma pack()

typedef map<hash_t, string> hash_to_file_t;

void proc_archive(hash_t          script_hash, 
                  hash_to_file_t& files, 
                  const string&   archive_name)
{
  int fd = as::open_or_die(archive_name, O_RDONLY | O_BINARY);

  DATHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  hdr.flip_endian();
  hdr.entry_count ^= 0x26ACA46E;

  auto entries = new DATENTRY[hdr.entry_count];
  read(fd, entries, sizeof(DATENTRY) * hdr.entry_count);

  for (uint32_t i = 0; i < hdr.entry_count; i++) {
    entries[i].flip_endian();

    entries[i].type            ^= entries[i].hash;
    entries[i].offset          ^= entries[i].hash;
    entries[i].length          ^= entries[i].hash;
    entries[i].original_length ^= entries[i].hash;

    if (entries[i].type == DATENTRY_TYPE_COMPRESSED) {
      uint32_t offset = entries[i].offset;
      uint32_t len    = entries[i].length;
      auto     buff   = new uint8_t[len];
      lseek(fd, offset, SEEK_SET);
      read(fd, buff, len);

      if (script_hash) {
        auto p   = (uint32_t*) buff;
        auto end = p + len / 4;

        while (p < end) {
          *p++ ^= entries[i].hash ^ script_hash;
        }
      }

      uLongf   out_len  = entries[i].original_length;
      auto     out_buff = new uint8_t[out_len];
      uncompress(out_buff, &out_len, buff, len);

      string filename = as::stringf("script/%05d.script", i);

      as::make_path(filename);
      as::write_file(filename, out_buff, out_len);

      delete [] out_buff;
      delete [] buff;

      continue;
    }

    if (files.find(entries[i].hash) == files.end()) {
      continue;
    }

    string   filename     = files[entries[i].hash];
    uint32_t filename_len = filename.length();

    files.erase(entries[i].hash);    

    entries[i].offset          ^= (uint8_t) filename[filename.length() >> 1];
    entries[i].length          ^= (uint8_t) filename[filename.length() >> 2];
    entries[i].original_length ^= (uint8_t) filename[filename.length() >> 3];

    uint32_t offset = entries[i].offset;
    uint32_t len    = entries[i].length;
    auto     buff   = new uint8_t[len];
    lseek(fd, offset, SEEK_SET);
    read(fd, buff, len);

    if (entries[i].type == DATENTRY_TYPE_OBFUSCATED) {
      uint32_t block_len = len / filename_len;

      auto p = buff;

      for (uint32_t j = 0; j < filename_len - 1; j++) {
        for (uint32_t k = 0; k < block_len; k++) {
          *p++ ^= filename[j];
        }
      }
    }

    as::make_path(filename);
    as::write_file(filename, buff, len);

    delete [] buff;
  };

  close(fd);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exwhaledat v1.1, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <files.txt>\n", argv[0]);
    return -1;
  }

  string lst_filename(argv[1]);

  as::split_strings_t archives;
  bool                script_hash_done = false;
  hash_t              script_hash      = 0;
  hash_to_file_t      files;

  auto fh = as::open_or_die_file(lst_filename);  

  while (!feof(fh)) {
    string line = as::read_line(fh);

    if (line.empty() || *line.begin() == '#') {
      continue;
    }

    if (!script_hash_done) {
      line             = as::stringtol(line);
      script_hash_done = true;
      script_hash      = hash_buff(line.c_str(), line.length());
    } else if (archives.empty()) {
      archives = as::splitstr(line, ",");
    } else {
      hash_t hash = hash_buff(line.c_str(), line.length());
      files[hash] = line;
    }
  }

  fclose(fh);

  for (uint32_t i = 0; i < archives.size(); i++) {
    proc_archive(script_hash, files, archives[i]);
  }

  for (hash_to_file_t::iterator i = files.begin();
       i != files.end();
       ++i)
  {
    printf("Requested file not found: %s\n", i->second.c_str());
  }

  return 0;
}
