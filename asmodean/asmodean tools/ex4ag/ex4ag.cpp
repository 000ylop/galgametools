// ex4ag.cpp, v1.03 2010/06/24
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts GAF4 (*.4ag) and WAG@ (*.wag) archives used by Xuse.

#include "as-util.h"

// Handle 2 byte crud in Spirit of Eternity Sword 2
#define SES2_CRUD

using std::string;

#pragma pack(1)
struct GAFHDR {
  unsigned char  signature[4]; // "GAF4"
  unsigned short unknown1;
  char           title[64];
  unsigned long  entry_count;
};

struct GAFENTRY {
  unsigned long offset;
};

struct GAFTAG {
  unsigned char  signature[4];
  unsigned long  length;
  unsigned short unknown;
};
#pragma pack()

unsigned long gen_key(const char*    buff,
                      unsigned long  len,
                      unsigned char* out_buff) 
{
  unsigned long work1 = 0;

  for (unsigned long i = 0; i < len; i++) {
    unsigned long c = buff[i];

    c += i;
    c ^= work1;
    c += len;

    work1 = c;
  }

  unsigned long out_len = (work1 & 0xFF) + 0x40;  

  for (unsigned long i = 0; i < len; i++) {
    work1 += buff[i];
  }

  unsigned char work2 = *out_buff++ = (unsigned char) (work1 & 0x0F);

  work1 >>= 8;

  *out_buff++ = (unsigned char) (work1 & 0xFF);
  *out_buff++ = 0x46;
  *out_buff++ = 0x88;

  for (unsigned long i = 4; i < out_len - 1; i++) {
    unsigned char c = buff[i % len];
    
    c ^= work2;
    c += (unsigned char) i;

    work2 += c;

    *out_buff++ = work2;
  }

  *out_buff++ = 0x00;

  return out_len;
}

void wtf(unsigned char* buff,
         unsigned long  len,
         unsigned long& val1,
         unsigned long& val2)
{
  unsigned long work1 = 0x200;
  unsigned long work2 = 0x200;

  for (unsigned long i = 0; i < len; i++) {
    work1 += buff[len - i - 1];
    work2 -= buff[len - i - 1];
  }

  for (unsigned long i = 0; i < len; i++) {
    unsigned long temp1 = buff[i];

    work1 ^= temp1;

    temp1 &= 0x1F;

    unsigned long temp2 = temp1;

    temp1 = (0 - temp1) & 0x1F;

    work1 = (work1 << 0x1F)  | (work1 >> 1);
    work2 = (work2 << temp1) | (work2 >> temp2);
  }

  for (unsigned long i = 0; i < len; i++) {
    work1 ^=  buff[i];
    work2 ^= ~buff[i];

    work1 = (work1 << 0x1F) | (work1 >> 1);
    work2 = (work2 << 0x1F) | (work2 >> 1);
  }

  val1 = work1 % 0x401;
  val2 = work2 % 0x401;

  return;
}

void gen_key2(unsigned char* buff,
              unsigned long  len,
              unsigned char* out_buff,
              unsigned long  out_len,
              unsigned long  seed)
{
  for (unsigned long i = 0; i < out_len; i++) {
    unsigned char c = buff[i % len];

    c += (unsigned char) (i & 0xFF);
    c ^= buff[(i + 1) % len];
    c += (unsigned char) (seed & 0xFF);

    *out_buff++ = c;
  }
}

void unobfuscate(unsigned char* buff,
                 unsigned long  len,
                 unsigned char* key,
                 unsigned long  key_len,
                 unsigned long  seed)
{
  key_len--;

  seed %= key_len;

  for (unsigned long i = 0; i < len; i++) {
    buff[i] ^= key[(seed + i) % key_len];
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "ex4ag v1.03 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.4ag> [-exact_filename]\n\n", argv[0]);
    fprintf(stderr, "\t-exact_filename = use exact filename supplied by user (including path!)");
    return -1;
  }

  string in_filename(argv[1]);
  bool   exact_filename = argc > 2 && !strcmp(argv[2], "-exact_filename");
  string name_only = in_filename;
  
  if (!exact_filename) {
    name_only = as::stringtol(as::get_filename(in_filename));
  }
  
  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  GAFHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  unsigned char key[4096] = { 0 };
  unsigned long key_len = gen_key(name_only.c_str(), 
                                  (unsigned long)name_only.length(),
                                  key);

  unsigned char data_key[4096] = { 0 };
  unsigned long data_key_len = gen_key(hdr.title, 
                                       (unsigned long)strlen(hdr.title),
                                       data_key);

  unsigned long junk1_len = 0;
  unsigned long junk2_len = 0;
  wtf(key, key_len, junk1_len, junk2_len);

  unsigned long entries_len = sizeof(GAFENTRY) * hdr.entry_count;
  GAFENTRY*     entries     = new GAFENTRY[hdr.entry_count];
  lseek(fd, junk1_len, SEEK_CUR);
  read(fd, entries, entries_len);

  unsigned long  toc_key_len = entries_len;
  unsigned char* toc_key     = new unsigned char[entries_len];
  gen_key2(key, key_len, toc_key, toc_key_len, hdr.entry_count);

  unobfuscate((unsigned char*)entries,
              entries_len,
              toc_key,
              toc_key_len,
              sizeof(hdr) + junk1_len);

  for (unsigned long i = 0; i < hdr.entry_count; i++) {
    unsigned long  len  = (i < hdr.entry_count - 1 ?
                             entries[i + 1].offset : 
                             as::get_file_size(fd)) - entries[i].offset;
    unsigned char* buff = new unsigned char[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);
    unobfuscate(buff, len, data_key, data_key_len, entries[i].offset);

    GAFTAG* dset_tag = (GAFTAG*) buff;
    GAFTAG* tag      = NULL;

    if (!memcmp(dset_tag, "DSET", 4)) {      
      // Presumably there's always a filename, but if not...
      string name = as::stringf("%05d.dset", i);;

      // Have to search for a filename tag...
      unsigned char* p = buff + sizeof(*dset_tag);
      for (unsigned long j = 0; j < dset_tag->length; j++, p += tag->length) {
        tag = (GAFTAG*) p;
        p += sizeof(*tag);

        if (memcmp(tag->signature, "FTAG", 4))
          continue;

#ifdef SES2_CRUD
        name.assign((char*)p, tag->length - 2);
#else
        name.assign((char*)p, tag->length);
#endif
      }

      p = buff + sizeof(*dset_tag);
      for (unsigned long j = 0; j < dset_tag->length; j++, p += tag->length) {
        tag = (GAFTAG*) p;
        p += sizeof(*tag);

        if (!memcmp(tag->signature, "FTAG", 4))
          continue;

        unsigned long  out_len  = tag->length;
        unsigned char* out_buff = p;

        // Honestly it's kindof a waste of effort to extract non-PICT data
        // at all but ... why not.
        if (!memcmp(tag->signature, "PICT", 4)) {
#ifdef SES2_CRUD
          out_len  -= 6;
          out_buff += 6;
#else
          out_len  -= 4;
          out_buff += 4;
#endif
        }

        string filename = name;

        if (j > 0) {
          filename = as::stringf("%s+%03d", as::get_file_prefix(name).c_str(), j);
        }

        // Make all paths relative
        filename = ".\\" + filename;

        filename = as::tr(filename, ":", "_");

        as::make_path(filename);
        as::write_file(filename, out_buff, out_len);
      }
    } else {
      fprintf(stderr, "%s: unknown data at index %d\n", in_filename.c_str(), i);
    }

    delete [] buff;
  }

  delete [] toc_key;
  delete [] entries;

  close(fd);

  return 0;
}
