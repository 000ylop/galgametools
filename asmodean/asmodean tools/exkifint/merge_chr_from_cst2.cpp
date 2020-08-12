// merge_chr_from_cst2.cpp, v1.02 2013/05/26
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// Damnit

#include "as-util.h"
#include "zlib.h"

#include <set>

// For グリザイアの楽園
#define TOLERATE_MISSING

struct CSTHDR1 {
  unsigned char signature[8]; // "CatScene"
  unsigned long length;
  unsigned long original_length;
};

struct CSTHDR2 {
  unsigned long length;
  unsigned long entry_count;
  unsigned long table2_offset; // from end of header
  unsigned long data_offset;   // from end of header
};

struct CSTENTRY1 {
  unsigned long entry_count;
  unsigned long start_index;
};

struct CSTENTRY2 {
  unsigned long offset;
};

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "merge_chr_from_cst2 v1.02 by asmodean\n\n");
    fprintf(stderr, "usage: %s <suffix|none> <input1.cst> [input2.cst ...]\n", argv[0]);
    return -1;
  }

  string suffix = argv[1];

  if (as::stringtol(suffix) == "none") {
    suffix = "";
  }

  typedef std::set<string> done_files_t;
  done_files_t done_files;

  for (int i = 2; i < argc; i++) {
    string in_filename  = argv[i];  
  
    int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

    CSTHDR1 hdr1;
    read(fd, &hdr1, sizeof(hdr1));

    unsigned long  len  = hdr1.length;
    unsigned char* buff = new unsigned char[len];
    read(fd, buff, len);

    unsigned long  out_len  = hdr1.original_length;
    unsigned char* out_buff = new unsigned char[out_len];
    uncompress(out_buff, &out_len, buff, len);

    CSTHDR2*       hdr2     = (CSTHDR2*) out_buff;
    CSTENTRY1*     sections = (CSTENTRY1*) (hdr2 + 1);
    CSTENTRY2*     entries  = (CSTENTRY2*) (out_buff + sizeof(*hdr2) + hdr2->table2_offset);
    char*          data    = (char*) (out_buff + sizeof(*hdr2) + hdr2->data_offset);

    for (unsigned long j = 0; j < hdr2->entry_count; j++) {
      for (unsigned long k = 0; k < sections[j].entry_count; k++) {
        string s = data + entries[sections[j].start_index + k].offset;
        
        // 0x01
        // 0x30 = script, 0x20 = text, 0x21 = name, 0x02 = blank?
        //printf("%02X %02X [%s]\n", s[0], s[1], s + 2);

        string::size_type pos = s.find("cg ");
        
        if (pos != string::npos && s.find("attr_def") == string::npos) {
          s = s.substr(pos);

          as::split_strings_t fields1 = as::splitstr(s, " ");

          if (fields1.size() >= 3) {
            as::split_strings_t fields2 = as::splitstr(fields1[2], ",");

            unsigned long  base_len      = 0;
            unsigned char* base_buff     = NULL;
            unsigned long  base_width    = 0;
            unsigned long  base_height   = 0;
            unsigned long  base_depth    = 0;
            unsigned long  base_stride   = 0;
            unsigned long  base_offset_x = 0;
            unsigned long  base_offset_y = 0;

            string base = fields2[0];

            if (!suffix.empty()) {
              base = base.substr(0, 4);
            }

            string out_filename = "merged/" + base + suffix + "+";
            for (unsigned long q = 1; q < fields2.size(); q++) {
              out_filename += fields2[q];
            }

            if (done_files.find(out_filename) != done_files.end()) {
              continue;
            }

            done_files.insert(out_filename);

            for (unsigned long q = 1; q < fields2.size(); q++) {
              string        filename      = base + suffix + as::stringf("_%0*s", q, fields2[q].c_str()); 

              unsigned long offset_x      = 0;
              unsigned long offset_y      = 0;
              string        full_filename = as::find_filename_with_xy(filename, 
                                                                      &offset_x, 
                                                                      &offset_y,
                                                                      as::FIND_FILENAME_STRICT);

              if (!as::is_file_readable(full_filename)) {
                fprintf(stderr, "Part not found [%s] [%s] [%s]\n", s.c_str(), filename.c_str(), full_filename.c_str());

#ifdef TOLERATE_MISSING
                continue;
#else
                delete [] base_buff;
                base_buff = NULL;
                break;
#endif
              }

              if (base_buff) {
                as::blend_bmp_resize(full_filename,
                                     offset_x,
                                     offset_y,
                                     base_buff,
                                     base_len,
                                     base_width,
                                     base_height,
                                     base_depth,
                                     base_offset_x,
                                     base_offset_y);
              } else {
                base_offset_x = offset_x;
                base_offset_y = offset_y;

                as::read_bmp(full_filename,
                             base_buff,
                             base_len,
                             base_width,
                             base_height,
                             base_depth,
                             base_stride,
                             as::READ_BMP_ONLY32BIT);
              }
            }

            out_filename += ".bmp";

            if (base_buff) {
              as::make_path(out_filename);
              as::write_bmp(out_filename, 
                            base_buff,
                            base_len,
                            base_width,
                            base_height,
                            base_depth);

              delete [] base_buff;
            }
          }
        }
      }
    }

    delete [] out_buff;
    delete [] buff;

    close(fd);
  }

  return 0;
}

