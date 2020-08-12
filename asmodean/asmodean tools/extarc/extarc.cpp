// extac.cpp, v1.0 2010/12/17
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts TArc1.00 (*.tac) archives.

#include "as-util.h"
#include "blowfish.h"
#include "zlib.h"

struct TACHDR {
  unsigned char signature[8]; // "TArc1.00"
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long entry_count;
  unsigned long bucket_count;
  unsigned long toc_length;
  unsigned long seed;
};

struct TACBUCKET {
  unsigned short hash16;
  unsigned short entry_count;
  unsigned long  start_index;
};

struct TACENTRY {
  unsigned long long hash48;
  bool               compressed;
  unsigned long      original_length;
  unsigned long      offset;
  unsigned long      length;
};

void decrypt(unsigned char* buff, 
             unsigned long  len, 
             const string&  key)
{
  Blowfish bf;        
  bf.Set_Key((unsigned char*)key.c_str(), key.length());
  bf.Decrypt(buff, (len / 8) * 8);
}

struct TSVHDR {
  unsigned char signature[4]; // "TSV"
  unsigned long original_length;
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long unknown4;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "extac v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.tac>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix = as::get_file_prefix(in_filename, true);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  TACHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned long  toc_len  = hdr.toc_length;
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);
  decrypt(toc_buff, toc_len, "TLibArchiveData");

  // "big enough"
  unsigned long  out_toc_len  = 1024 * 1024 * 10;
  unsigned char* out_toc_buff = new unsigned char[out_toc_len];
  int rc = uncompress(out_toc_buff, &out_toc_len, toc_buff, toc_len);

  TACBUCKET* buckets = (TACBUCKET*) out_toc_buff;
  TACENTRY*  entries = (TACENTRY*) (buckets + hdr.bucket_count);

  unsigned long data_base = sizeof(hdr) + hdr.toc_length;
  unsigned long n         = 0;

  for (unsigned long i = 0; i < hdr.bucket_count; i++) {
    TACBUCKET& bucket = buckets[i];    

    for (unsigned long j = 0; j < bucket.entry_count; j++) {
      unsigned long entry_index = bucket.start_index + j;
      TACENTRY&     entry       = entries[entry_index];

      unsigned long long hash = (entry.hash48 << 16) | bucket.hash16;

      unsigned long  len  = entry.length;
      unsigned char* buff = new unsigned char[len];
      lseek(fd, data_base + entry.offset, SEEK_SET);
      read(fd, buff, len);

      if (entry.compressed) {
        unsigned long  out_len  = entry.original_length;
        unsigned char* out_buff = new unsigned char[out_len];
        uncompress(out_buff, &out_len, buff, len);

        delete [] buff;

        len  = out_len;
        buff = out_buff;
      }      

      // Hm ... this may just be a coincidence in the archives I looked at.
      if (!entry.compressed) {
        string key = as::stringf("%0I64u_tlib_secure_", hash);

        unsigned char check[8];
        memcpy(check, buff, sizeof(check));
        decrypt(check, sizeof(check), key);

        unsigned long decr_len = 10240;

        if (!memcmp(check, "RIFF", 4) || !memcmp(check, "TArc", 4)) {
          decr_len = len;
        } else if (len < 10240) {
          decr_len = (len / 8) * 8;
        }

        decrypt(buff, decr_len, as::stringf("%0I64u_tlib_secure_", hash));
      }

      string ext = as::guess_file_extension(buff, len);
      if (!memcmp(buff, "TArc", 4)) ext = ".tac";

      as::write_file(prefix + as::stringf("+%05d", entry_index) + ext, 
                     buff,
                     len);

      delete [] buff;
    }
  }

  delete [] out_toc_buff;
  delete [] toc_buff;

  close(fd);

  return 0;
}