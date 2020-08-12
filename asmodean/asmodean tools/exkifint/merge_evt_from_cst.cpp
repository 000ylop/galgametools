// merge_evt_from_cst.cpp, v1.0 2013/09/15
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges event images from ガールズbeアンビシャス.

#include "as-util.h"
#include "zlib.h"

struct CSTHDR1 {
  uint8_t  signature[8]; // "CatScene"
  uint32_t length;
  uint32_t original_length;
};

struct CSTHDR2 {
  uint32_t length;
  uint32_t entry_count;
  uint32_t table2_offset; // from end of header
  uint32_t data_offset;   // from end of header
};

struct CSTENTRY1 {
  uint32_t entry_count;
  uint32_t start_index;
};

struct CSTENTRY2 {
  uint32_t offset;
};

struct combination_t {
  typedef std::list<string> parts_t;
  parts_t parts;
  string out_filename;
};

typedef std::list<combination_t> combinations_t;

void parse_combinations(const as::split_strings_t& fields, 
                        uint32_t                   index, 
                        combinations_t&            combinations)
{
  for (uint32_t i = index; i < fields.size(); i++) {
    if (fields[i][0] == '$') {
      as::split_strings_t temp_fields = fields;

      char field_values[] = { "123456789abcdefghijklmnopqrstuvwxyz" };
                              

      for (uint32_t j = 0; j < sizeof(field_values) - 1; j++) {        
        temp_fields[i] = field_values[j];

        parse_combinations(temp_fields, i + 1, combinations);
      }
    } else {
      parse_combinations(fields, i + 1, combinations);
    }
  }

  if (index == fields.size()) {
    combination_t combination;
    combination.out_filename = "merged/" + fields[0] + "+";

    for (uint32_t i = 1; i < fields.size(); i++) {
      string filename = fields[0] + as::stringf("_%0*s.png", i, fields[i].c_str()); 

      if (as::image_t::can_open(filename)) {
        combination.parts.push_back(filename);        
      } else {
        if (fields[i] != "0") {
          return;
        }
      }

      combination.out_filename += fields[i];
    }

    combination.out_filename += ".bmp";

    if (combination.parts.size()) {
      combinations.push_back(combination);
    }
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "merge_evt_from_cst v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input1.cst> [input2.cst ...]\n", argv[0]);
    return -1;
  }

  typedef std::set<string> done_t;
  done_t done;

  as::used_files_t used;

  as::make_path("merged/");

  for (int i = 1; i < argc; i++) {
    string in_filename  = argv[i];
    int    fd           = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

    CSTHDR1 hdr1;
    read(fd, &hdr1, sizeof(hdr1));

    uint32_t len  = hdr1.length;
    uint8_t* buff = new uint8_t[len];
    read(fd, buff, len);

    uLongf   out_len  = hdr1.original_length;
    uint8_t* out_buff = new uint8_t[out_len];
    uncompress(out_buff, &out_len, buff, len);

    CSTHDR2*   hdr2     = (CSTHDR2*) out_buff;
    CSTENTRY1* sections = (CSTENTRY1*) (hdr2 + 1);
    CSTENTRY2* entries  = (CSTENTRY2*) (out_buff + sizeof(*hdr2) + hdr2->table2_offset);
    char*      data     = (char*) (out_buff + sizeof(*hdr2) + hdr2->data_offset);

    for (uint32_t j = 0; j < hdr2->entry_count; j++) {
      for (uint32_t k = 0; k < sections[j].entry_count; k++) {
        string s = data + entries[sections[j].start_index + k].offset;

        string::size_type pos = s.find("bg ");
        if (pos == string::npos) {
          pos = s.find("cg ");
        }
        
        if (pos != string::npos) {
          s = s.substr(pos);

          if (done.find(s) != done.end()) {
            continue;
          }

          done.insert(s);

          as::split_strings_t fields1 = as::splitstr(s, " ");

          if (fields1.size() >= 3) {
            as::split_strings_t fields2 = as::splitstr(fields1[2], ",");

            combinations_t combinations;
            parse_combinations(fields2, 1, combinations);

            // Hack to get some missing large variations done...
            if (fields1[0] == "cg") {
              as::split_strings_t temp = fields2;

              temp[0] = fields2[0] + "l";
              parse_combinations(temp, 1, combinations);

              temp[0] = fields2[0] + "ls";
              parse_combinations(temp, 1, combinations);
            }

            for (combinations_t::iterator combination = combinations.begin();
                 combination != combinations.end();
                 ++combination)
            {
              as::image_t image(combination->parts.front());              
              combination->parts.pop_front();

              used.add(image.source());

              for (combination_t::parts_t::iterator part = combination->parts.begin();
                   part != combination->parts.end();
                   ++part)
              {
                as::image_t layer(*part);
                image.blend(layer);

                used.add(layer.source());
              }

              image.write(combination->out_filename);
            }
          }
        }
      }
    }

    delete [] out_buff;
    delete [] buff;

    close(fd);
  }

  used.done();

  return 0;
}

