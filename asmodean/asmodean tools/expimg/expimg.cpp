// expimg.cpp, v1.0 2012/03/30
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts PSB (*.pimg) image composites used by DRACU-RIOT.

#include "as-util.h"

struct PSBHDR {
  unsigned char signature[4];
  unsigned long type;
  unsigned long unknown1;
  unsigned long offset_names;
  unsigned long offset_strings;
  unsigned long offset_strings_data;
  unsigned long offset_chunk_offsets;
  unsigned long offset_chunk_lengths;
  unsigned long offset_chunk_data;
  unsigned long offset_entries;
};

/***************************************************************************
 * A whole bunch of packed data junk...
 ***************************************************************************/

class psb_t;

/***************************************************************************
 * psb_value_t
 */
class psb_value_t {
public:
  // Probably this should actually be kind as in get_number below. Don't care.
  enum type_t {
    TYPE_ARRAY   = 0x31337, // fake
    TYPE_OFFSETS = 0x20,
    TYPE_OBJECTS = 0x21,
  };

  psb_value_t(const psb_t&    psb, 
              type_t          type,
              unsigned char*& p);

  virtual ~psb_value_t(void);

  type_t get_type(void) const;

protected:
  const psb_t& psb;

private:
  type_t type;
};

/***************************************************************************
 * psb_array_t
 */
class psb_array_t : public psb_value_t {
public:
  psb_array_t(const psb_t&    psb, 
              unsigned char*& p);

  unsigned long size(void) const;

  unsigned long get(unsigned long index) const;

  unsigned long  entry_count;
  unsigned long  entry_length;
  unsigned char* buff;
};

/***************************************************************************
 * psb_objects_t
 */
class psb_objects_t : public psb_value_t {
public:
  psb_objects_t(const psb_t&    psb, 
                unsigned char*& p);

  ~psb_objects_t(void);

  unsigned long size(void) const;

  string get_name(unsigned long index) const;

  unsigned char* get_data(unsigned long index) const;

  unsigned char* get_data(const string& name) const;

  template<class T> void unpack(T*& out, const string& name) const;

private:
  psb_array_t*   names;
  psb_array_t*   offsets;
  unsigned char* buff;
};

/***************************************************************************
 * psb_offsets_t
 */
class psb_offsets_t : public psb_value_t {
public:
  psb_offsets_t(const psb_t&    psb, 
                unsigned char*& p);

  ~psb_offsets_t(void);

  unsigned long size(void) const;

  unsigned char* get(unsigned long index) const;

  template<class T> void unpack(T*& out, unsigned long index) const;

private:
  psb_array_t*   offsets;
  unsigned char* buff;
};

/***************************************************************************
 * psb_t
 */
class psb_t {
public:
  psb_t(unsigned char* buff);

  ~psb_t(void);

  string get_name(unsigned long index) const;

  unsigned long get_number(unsigned char* p) const;

  string get_string(unsigned char* p) const;

  const psb_objects_t* get_objects(void) const;

  unsigned char* get_chunk(unsigned char* p) const;

  unsigned long get_chunk_length(unsigned char* p) const;

  string make_filename(const string& name) const;

  psb_value_t* unpack(unsigned char*& p) const;

  template<class T> void unpack(T*& out, unsigned char*& p) const;

private:
  unsigned long get_chunk_index(unsigned char* p) const;

  unsigned char* buff;
  PSBHDR*        hdr;
  psb_array_t*   str1;
  psb_array_t*   str2;
  psb_array_t*   str3;
  psb_array_t*   strings;
  char*          strings_data;
  psb_array_t*   chunk_offsets;
  psb_array_t*   chunk_lengths;
  unsigned char* chunk_data;

  psb_objects_t* objects;
  psb_offsets_t* expire_suffix_list;

  string         extension;
};

/***************************************************************************
 * Implementation of lots of junk!
 ***************************************************************************/

/***************************************************************************
 * psb_value_t
 */
psb_value_t::
psb_value_t(const psb_t&    psb,
            type_t          type,
            unsigned char*& p)
  : psb(psb),
    type(type)
{}

psb_value_t::
~psb_value_t(void) {
}

psb_value_t::type_t
psb_value_t::
get_type(void) const {
  return type;
}

/***************************************************************************
 * psb_array_t
 */
psb_array_t::
psb_array_t(const psb_t&    psb, 
            unsigned char*& p) 
  : psb_value_t(psb, TYPE_ARRAY, p)
{
  unsigned long n = *p++ - 0xC;
  unsigned long v = 0;

  for (unsigned long i = 0; i < n; i++) {
    v |= *p++ << (i * 8);
  }

  entry_count  = v;
  entry_length = *p++ - 0xC;
  buff         = p;

  p += entry_count * entry_length;
}

unsigned long 
psb_array_t::
size(void) const {
  return entry_count;
}

unsigned long 
psb_array_t::
get(unsigned long index) const {
  unsigned long v = 0;

  unsigned char* p = buff + index * entry_length;

  for (unsigned long i = 0; i < entry_length; i++) {
    v |= *p++ << (i * 8);
  }

  return v;
}

/***************************************************************************
 * psb_objects_t
 */
psb_objects_t::
psb_objects_t(const psb_t&    psb, 
              unsigned char*& p)
  : psb_value_t(psb, TYPE_OBJECTS, p),
    buff(p)
{
  names   = new psb_array_t(psb, buff);
  offsets = new psb_array_t(psb, buff);
}

psb_objects_t::
~psb_objects_t(void) {
  delete offsets;
  delete names;
}

unsigned long 
psb_objects_t::
size(void) const {
  return names->size();
}

string 
psb_objects_t::
get_name(unsigned long index) const {
  return psb.get_name(names->get(index));
}

unsigned char* 
psb_objects_t::
get_data(unsigned long index) const {
  return buff + offsets->get(index);
}

unsigned char* 
psb_objects_t::
get_data(const string& name) const {
  for (unsigned long i = 0; i < names->size(); i++) {
    if (get_name(i) == name) {
      return get_data(i);
    }
  }

  return NULL;
}

template<class T> 
void
psb_objects_t::
unpack(T*& out, const string& name) const {
  out = NULL;  

  unsigned char* temp = get_data(name);

  if (temp) {
    psb.unpack(out, temp);
  }
}

/***************************************************************************
 * psb_offsets_t
 */
psb_offsets_t::
psb_offsets_t(const psb_t&    psb, 
              unsigned char*& p)
  : psb_value_t(psb, TYPE_OFFSETS, p)
{
  offsets = new psb_array_t(psb, p);
  buff    = p;
}

psb_offsets_t::
~psb_offsets_t(void) {
  delete offsets;
}

unsigned long 
psb_offsets_t::
size(void) const {
  return offsets->size();
}

unsigned char*
psb_offsets_t::
get(unsigned long index) const {
  return buff + offsets->get(index);
}

template<class T> 
void 
psb_offsets_t::
unpack(T*& out, unsigned long index) const {
  unsigned char* temp = get(index);

  psb.unpack(out, temp);
}

/***************************************************************************
 * psb_t
 */
psb_t::
psb_t(unsigned char* buff) {
  this->buff = buff;
  hdr        = (PSBHDR*) buff;

  unsigned char* p = buff + hdr->offset_names;
  str1 = new psb_array_t(*this, p);
  str2 = new psb_array_t(*this, p);
  str3 = new psb_array_t(*this, p);

  p             = buff + hdr->offset_strings;
  strings       = new psb_array_t(*this, p);
  
  strings_data  = (char*) (buff + hdr->offset_strings_data);

  p             = buff + hdr->offset_chunk_offsets;
  chunk_offsets = new psb_array_t(*this, p);

  p             = buff + hdr->offset_chunk_lengths;
  chunk_lengths = new psb_array_t(*this, p);

  chunk_data    = buff + hdr->offset_chunk_data;

  p = buff + hdr->offset_entries;
  unpack(objects, p);

  expire_suffix_list = NULL;

  if (objects) {
    objects->unpack(expire_suffix_list, "expire_suffix_list");

    if (expire_suffix_list) {
      // Hrm ... ever more than one?
      extension = get_string(expire_suffix_list->get(0));
    }
  }
}

psb_t::
~psb_t(void) {
  delete expire_suffix_list;
  delete objects;
  delete chunk_lengths;
  delete chunk_offsets;  
  delete strings;
  delete str3;
  delete str2;
  delete str1;
}

string 
psb_t::
get_name(unsigned long index) const {
  string accum;

  unsigned long a = str3->get(index);
  unsigned long b = str2->get(a);

  while (true) {
    unsigned long c = str2->get(b);
    unsigned long d = str1->get(c);
    unsigned long e = b - d;

    b = c;

    accum = (char)e + accum;

    if (!b) {
      break;
    }
  }

  return accum;
}

unsigned long
psb_t::
get_number(unsigned char* p) const {
  static const unsigned long TYPE_TO_KIND[] = { 
     0, 1, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 0xA, 0xB, 0xC
  };

  unsigned char  type = *p++;
  unsigned long  kind = TYPE_TO_KIND[type];
  unsigned long  v    = 0;

  switch (kind) {
    case 1:
      v = 0;
      break;

    case 2:
      v = 1;
      break;

    case 3:
    {
      unsigned long n = type - 4;

      for (unsigned long i = 0; i < n; i++) {
        v |= *p++ << (i * 8);
      }
    }
    break;

    case 9:
      if (type == 0x1E) {
        v = (unsigned long) *(float*)p;
      }
      break;

    case 10:
      if (type == 0x1F) {
        v = (unsigned long) *(double*)p;
        p += 8;
      }
      break;

    default:
      printf("warning: unsupported packed number type (%d)\n", kind);
      break;
  }

  return v;  
}

string 
psb_t::
get_string(unsigned char* p) const {
  unsigned long n = *p++ - 0x14;
  unsigned long v = 0;

  for (unsigned long i = 0; i < n; i++) {
    v |= *p++ << (i * 8);
  }

  return strings_data + strings->get(v);
}

const psb_objects_t*
psb_t::
get_objects(void) const {
  return objects;
}

unsigned char* 
psb_t::
get_chunk(unsigned char* p) const {
  return chunk_data + chunk_offsets->get(get_chunk_index(p));
}

unsigned long 
psb_t::
get_chunk_length(unsigned char* p) const {
  return chunk_lengths->get(get_chunk_index(p));
}

psb_value_t* 
psb_t::
unpack(unsigned char*& p) const {
  unsigned char type = *p++;

  switch (type) {
    case psb_value_t::TYPE_ARRAY:
      return new psb_array_t(*this, p);

    case psb_value_t::TYPE_OFFSETS:
      return new psb_offsets_t(*this, p);

    case psb_value_t::TYPE_OBJECTS:
      return new psb_objects_t(*this, p);

    default:
      p--;
      return NULL;
  }

  return NULL;
}

template<class T> 
void 
psb_t::
unpack(T*& out, unsigned char*& p) const {
  out = dynamic_cast<T*>(unpack(p));
}

unsigned long
psb_t::
get_chunk_index(unsigned char* p) const {
  unsigned long n = *p++ - 0x18;
  unsigned long v = 0;

  for (unsigned long i = 0; i < n; i++) {
    v |= *p++ << (i * 8);
  }

  return v;
}

/***************************************************************************
 * End of all that garbage!
 ***************************************************************************/

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "expimg v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.pimg>\n", argv[0]);

    return -1;
  }

  string in_filename(argv[1]);
  string prefix = as::get_file_prefix(in_filename);

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY);

  unsigned long  toc_len  = as::get_file_size(fd);
  unsigned char* toc_buff = new unsigned char[toc_len];
  read(fd, toc_buff, toc_len);

  psb_t                psb(toc_buff);
  const psb_objects_t* psb_objects = psb.get_objects();

  unsigned long image_width  = psb.get_number(psb_objects->get_data("width"));
  unsigned long image_height = psb.get_number(psb_objects->get_data("height"));

  for (unsigned long i = 0; i < psb_objects->size(); i++) {
    string entry_name = psb_objects->get_name(i);
    string name       = as::stringtol(entry_name);

    bool is_tlg = name.find(".tlg") != string::npos;
    bool is_png = name.find(".png") != string::npos;

    if (is_tlg || is_png) {
      unsigned char* entry_buff = psb_objects->get_data(i);
      unsigned char* chunk_buff = psb.get_chunk(entry_buff);
      unsigned long  chunk_len  = psb.get_chunk_length(entry_buff);

      as::write_file(prefix + "+pimg+" + entry_name, chunk_buff, chunk_len);
    } else {
      if (entry_name != "width" && entry_name != "height" && entry_name != "layers") {
        fprintf(stderr, "%s: unknown resource \"%s\" (ignored)\n", entry_name.c_str());
      }
    }
  }

  FILE* fh = as::open_or_die_file(prefix + "+pimg+layers.txt", "w");
  fprintf(fh, "image_width:  %d\n", image_width);
  fprintf(fh, "image_height: %d\n\n", image_height);

  unsigned char* layers_buff    = psb_objects->get_data("layers");
  psb_offsets_t* layers_offsets = NULL;
  psb.unpack(layers_offsets, layers_buff);

  for (unsigned long i = 0; layers_offsets && i < layers_offsets->size(); i++) {
    psb_objects_t* layers_objects = NULL;
    layers_offsets->unpack(layers_objects, i);

    unsigned long height     = psb.get_number(layers_objects->get_data("height"));
    unsigned long layer_id   = psb.get_number(layers_objects->get_data("layer_id"));
    unsigned long layer_type = psb.get_number(layers_objects->get_data("layer_type"));
    unsigned long left       = psb.get_number(layers_objects->get_data("left"));
    string        name       = psb.get_string(layers_objects->get_data("name"));
    unsigned long opacity    = psb.get_number(layers_objects->get_data("opacity"));
    unsigned long top        = psb.get_number(layers_objects->get_data("top"));
    unsigned long type       = psb.get_number(layers_objects->get_data("type"));
    unsigned long visible    = psb.get_number(layers_objects->get_data("visible"));
    unsigned long width      = psb.get_number(layers_objects->get_data("width"));  

    fprintf(fh, "name:         %s\n", name.c_str());
    fprintf(fh, "layer_id:     %d\n", layer_id);
    fprintf(fh, "width:        %d\n", width);
    fprintf(fh, "height:       %d\n", height);
    fprintf(fh, "left:         %d\n", left);   
    fprintf(fh, "top:          %d\n", top);
    fprintf(fh, "opacity:      %d\n", opacity);
    fprintf(fh, "layer_type:   %d\n", layer_type);
    fprintf(fh, "type:         %d\n", type);
    fprintf(fh, "visible:      %d\n", visible);
    fprintf(fh, "\n");
  }

  fclose(fh);

  delete layers_offsets;
  delete [] toc_buff;

  close(fd);

  return 0;
}
