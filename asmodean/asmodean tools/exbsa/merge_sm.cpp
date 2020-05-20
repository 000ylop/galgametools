// merge_sm.cpp, v1.02 2011/08/06
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool merges delta images from éOéÀñ ík Å`òAçΩÇ∑ÇÈípêJÅEí≤ã≥ÇÃäwâÄÅ`.

#include <windows.h>
#include "as-util.h"
#include <set>
#include <vector>

string find_file(const string& path, const string& s) {
  WIN32_FIND_DATA found;
  string          name = path + "/" + s;
  string          glob = name + ".bmp";
  HANDLE          h    = FindFirstFile(glob.c_str(), &found);

  if (h == INVALID_HANDLE_VALUE) {
    glob = name + "+*.bmp";
    h    = FindFirstFile(glob.c_str(), &found);

    if (h == INVALID_HANDLE_VALUE) {
      return "";
    }
  }

  FindClose(h);

  return found.cFileName;
}

string complete_filename(const string&  s, 
                         unsigned long* offset_x = NULL,
                         unsigned long* offset_y = NULL)
{

  string prefix = "bg";
  string file   = find_file(prefix, s);

  if (file.empty()) {
    prefix = ".";
    file   = find_file(prefix, s);
  }

  if (file.empty()) {
    as::split_strings_t segments = as::splitstr(s, "_");

    if (segments.size() >= 2) {
      prefix = segments[0] + "/" + segments[1];
      file   = find_file(prefix, s);

      if (file.empty()) {
        prefix = segments[1];
        file   = find_file(prefix, s);
      }
    }
  }

  if (file.empty()) {
    return "";
  }

  string pattern = s + "+000+x%dy%d";

  if (offset_x && offset_y) {
    if (sscanf(file.c_str(), pattern.c_str(), offset_x, offset_y) != 2) {
      *offset_x = 0;
      *offset_y = 0;
    }   
  }

  return prefix + "/" + file;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "merge_sm v1.02 by asmodean\n\n");
    fprintf(stderr, "usage: %s <cg.dat>\n", argv[0]);    
    return -1;
  }

  string in_filename(argv[1]);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  unsigned long  len  = as::get_file_size(fd);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);  

  struct composite_t {
    unsigned long* layers;
    unsigned long  layer_count;
  };

  typedef std::vector<composite_t> composites_t;
  composites_t composites;

  unsigned char* p = buff;

  unsigned long group_group_count = *p++;

  for (unsigned long i = 0; i < group_group_count; i++) {
    unsigned char group_count = *p++;

    for (unsigned long j = 0; j < group_count; j++) {
      unsigned short entry_count = *(unsigned short*) p;
      p += 2;

      for (unsigned long k = 0; k < entry_count; k++) {
        unsigned char layer_count = *p++;

        unsigned long* layers = (unsigned long*) p;
        p += 4 * layer_count;

        composite_t composite = { layers, layer_count };
        composites.push_back(composite);

        // Hmm...
        if (k + 1 == entry_count) {
          p += 1;
        } else {
          p += 2;
        }
      }
    }
  }

  unsigned short file_count = *(unsigned short*) p;
  p += 2;

  typedef std::vector<string> files_t;
  files_t files;

  wchar_t* wp = (wchar_t*) p;

  for (unsigned long i = 0; i < file_count; i++) {
    string s = as::convert_wchar(wp);
    wp += wcslen(wp) + 1;
    
    files.push_back(s);
  }

  as::make_path("used/");
  as::make_path("merged/");

  typedef std::set<string> used_t;
  used_t used;

  for (composites_t::iterator i = composites.begin();
       i != composites.end();
       ++i)
  {
    bool           skipped       = false;
    string         base_name     = files[i->layers[0]];
    string         base_filename = complete_filename(base_name);

    if (base_filename.empty()) {
      printf("%s: base not found (skipped)\n", base_name.c_str());
      continue;
    }

    unsigned long  base_len      = 0;
    unsigned char* base_buff     = NULL;
    unsigned long  base_width    = 0;
    unsigned long  base_height   = 0;
    unsigned long  base_depth    = 0;
    unsigned long  base_stride   = 0;    

    as::read_bmp(base_filename,
                 base_buff,
                 base_len,
                 base_width,
                 base_height,
                 base_depth,
                 base_stride);

    string out_filename = "merged/" + base_name;

    for (unsigned long j = 1; j < i->layer_count; j++) {
      string        delta_name     = files[i->layers[j]];
      unsigned long delta_offset_x = 0;
      unsigned long delta_offset_y = 0;

      out_filename += "+" + delta_name;

      string delta_filename = complete_filename(delta_name, &delta_offset_x, &delta_offset_y);

      if (delta_filename.empty()) {
        printf("%s+%s: delta not found (skipped)\n", base_name.c_str(), delta_name.c_str());

        skipped = true;
        break;
      }

      as::blend_bmp(delta_filename,
                    delta_offset_x,
                    delta_offset_y,
                    base_buff,
                    base_len,
                    base_width,
                    base_height,
                    base_depth);

      used.insert(delta_filename);
    }

    if (!skipped) {
      as::write_bmp(out_filename + ".bmp",
                    base_buff,
                    base_len,
                    base_width,
                    base_height,
                    base_depth);

      if (i->layer_count == 1) {
        used.insert(base_filename);
      }
    }

    delete [] base_buff;    
  }

  for (used_t::iterator i = used.begin();
       i != used.end();
       ++i) {
    string target = "used/" + *i;

    as::make_path(target);
    rename(i->c_str(), target.c_str());
  }

  delete [] buff;

  close(fd);
  
  return 0;
}
