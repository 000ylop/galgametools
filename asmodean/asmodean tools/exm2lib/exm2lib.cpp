// exm2lib.cpp, v1.02 2011/09/29
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts mxb (*_info.psb.m + *_body.bin) archives.

#include "windows.h"
#include "Xcompress.h"

#include "as-util.h"
#include "mt.h"

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "md5.h"

struct game_info_t {
  string        name;
  string        seed;
  unsigned long key_length;
};

static const game_info_t   GAME_INFO[] = {
  { "Dunamis 15 (XBOX360)", "4nDSd4sa2v", 0x54 },
};

static const unsigned long GAME_INFO_COUNT = sizeof(GAME_INFO) / sizeof(GAME_INFO[0]);

struct MHDR {
  unsigned char signature[4]; // "mfl", "mxb", "mdf"
  unsigned long original_length;
};

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

void unobfuscate(unsigned char*     buff,
                 unsigned long      len,
                 const game_info_t& game_info,
                 const string&      filename)
{
  string seed = game_info.seed + as::stringtol(as::get_filename(filename));

  unsigned char md5hash[CryptoPP::Weak::MD5::DIGESTSIZE] = { 0 };

  CryptoPP::Weak::MD5().CalculateDigest(md5hash, (unsigned char*)seed.c_str(), seed.length());

  init_by_array((unsigned long*)md5hash, 4);

  unsigned long  key_len  = game_info.key_length;
  unsigned char* key_buff = new unsigned char[(key_len + 3) & ~3];
  unsigned long* key_p    = (unsigned long*) key_buff;

  for (unsigned long i = 0; i < key_len; i += 4) {
    *key_p++ = genrand_int32();
  }

  for (unsigned long i = 0; i < len; i++) {
    buff[i] ^= key_buff[i % key_len];
  }

  delete [] key_buff;
}

void read_decrypt_decompress(int                fd,
                             const game_info_t& game_info,
                             string&            filename,
                             unsigned long      len,
                             unsigned char*&    out_buff,
                             unsigned long&     out_len)
{
  unsigned long  temp_len  = len;
  unsigned char* temp_buff = new unsigned char[temp_len];
  read(fd, temp_buff, temp_len);

  MHDR*         hdr        = (MHDR*) temp_buff;
  unsigned char* data_buff = (unsigned char*) (hdr + 1);
  unsigned long  data_len  = temp_len  - sizeof(*hdr);

  bool is_mfl = !memcmp(hdr->signature, "mfl", 3);
  bool is_mxb = !memcmp(hdr->signature, "mxb", 3);
  bool is_mdf = !memcmp(hdr->signature, "mdf", 3);

  if (is_mfl || is_mxb || is_mdf) {
    unobfuscate(data_buff, data_len, game_info, filename);

    if (is_mxb) {
      XCOMPRESS_FILE_HEADER_LZXNATIVE* xhdr = (XCOMPRESS_FILE_HEADER_LZXNATIVE*) data_buff;
      unsigned char*                   p    = (unsigned char*) (xhdr + 1);

      xhdr->ContextFlags                         = as::flip_endian(xhdr->ContextFlags);
      xhdr->CodecParams.Flags                    = as::flip_endian(xhdr->CodecParams.Flags);
      xhdr->CodecParams.WindowSize               = as::flip_endian(xhdr->CodecParams.WindowSize);
      xhdr->CodecParams.CompressionPartitionSize = as::flip_endian(xhdr->CodecParams.CompressionPartitionSize);

      xhdr->UncompressedSizeLow    = as::flip_endian(xhdr->UncompressedSizeLow);
      xhdr->CompressedSizeLow      = as::flip_endian(xhdr->CompressedSizeLow);
      xhdr->UncompressedBlockSize  = as::flip_endian(xhdr->UncompressedBlockSize);
      xhdr->CompressedBlockSizeMax = as::flip_endian(xhdr->CompressedBlockSizeMax);

      out_len  = 0;
      out_buff = new unsigned char[xhdr->UncompressedSizeLow];

      XMEMDECOMPRESSION_CONTEXT context = NULL;
      XMemCreateDecompressionContext(XMEMCODEC_LZX, &xhdr->CodecParams, xhdr->ContextFlags, &context);

      while (out_len != xhdr->UncompressedSizeLow) {
        unsigned long block_len = as::flip_endian(*(unsigned long*) p);
        p += 4;

        SIZE_T decomp_len = xhdr->UncompressedSizeLow;   
        XMemDecompress(context, out_buff + out_len, &decomp_len, p, block_len);
        p += block_len;

        out_len += decomp_len;
      }

      XMemDestroyDecompressionContext(context);

      delete [] temp_buff;

      if (filename.length() >= 2 && filename[filename.length() - 2] == '.' && filename[filename.length() - 1] == 'm') {
        filename = filename.substr(0, filename.length() - 2);
      }
    } else {
      out_buff = temp_buff;
      out_len  = temp_len;
    }
  } else {
    out_buff = temp_buff;
    out_len  = temp_len;
  }
}

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

string 
psb_t::
make_filename(const string& name) const {
  string filename = name;
  string suffix   = ".m";

  // Lame hack.  I don't think there's enough information stored to tell me which names
  // have an "expired" suffix ...
  if (std::mismatch(suffix.rbegin(), suffix.rend(), filename.rbegin()).first != suffix.rend()) {
    filename += extension;
  }

  return filename;
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

bool proc_psb_image(const string& filename,
                        const psb_t&  psb)
{
  const psb_objects_t* psb_objects = psb.get_objects();

  if (!psb_objects || !psb_objects->get_data("imageList")) {
    return false;
  }

  psb_offsets_t* imagelist_offsets = NULL;
  psb_objects->unpack(imagelist_offsets, "imageList");

  for (unsigned long i = 0; i < imagelist_offsets->size(); i++) {
    psb_objects_t* image_objects = NULL;
    imagelist_offsets->unpack(image_objects, i);

    unsigned long width  = psb.get_number(image_objects->get_data("width"));
    unsigned long height = psb.get_number(image_objects->get_data("height"));

    psb_offsets_t* textures_offsets = NULL;
    image_objects->unpack(textures_offsets, "texture");

    unsigned long  out_len  = width * height * 4;
    unsigned char* out_buff = new unsigned char[out_len];
    memset(out_buff, 0, out_len);

    for (unsigned long j = 0; j < textures_offsets->size(); j++) {
      psb_objects_t* tex_objects = NULL;
      textures_offsets->unpack(tex_objects, j);

      unsigned long  tex_width  = psb.get_number(tex_objects->get_data("width"));
      unsigned long  tex_height = psb.get_number(tex_objects->get_data("height"));
      unsigned long  tex_top    = psb.get_number(tex_objects->get_data("top"));
      unsigned long  tex_left   = psb.get_number(tex_objects->get_data("left"));

      psb_objects_t* chunk_objects = NULL;
      tex_objects->unpack(chunk_objects, "image");

      string         type         = psb.get_string(chunk_objects->get_data("type"));
      unsigned char* chunk_buff   = psb.get_chunk(chunk_objects->get_data("pixel"));
      unsigned long  chunk_len    = psb.get_chunk_length(chunk_objects->get_data("pixel"));
      unsigned long  chunk_width  = psb.get_number(chunk_objects->get_data("width"));
      unsigned long  chunk_height = psb.get_number(chunk_objects->get_data("height"));

      enum pixel_type_t {
        PIXEL_TYPE_ALPHA,
        PIXEL_TYPE_LUMA,
        PIXEL_TYPE_ALPHALUMA,
        PIXEL_TYPE_RGBA
      };

      pixel_type_t  pixel_type  = PIXEL_TYPE_RGBA;
      unsigned long pixel_depth = 4;

      if (type == "A8") {
        pixel_type  = PIXEL_TYPE_ALPHA;
        pixel_depth = 1;
      } else if (type == "L8") {
        pixel_type  = PIXEL_TYPE_LUMA;
        pixel_depth = 1;
      } else if (type == "A8L8") {
        pixel_type  = PIXEL_TYPE_ALPHALUMA;
        pixel_depth = 2;
      }

      for (unsigned long y = 0; y < tex_height && y + tex_top < height; y++) {
        unsigned char* src = chunk_buff + y * tex_width * pixel_depth;
        unsigned char* dst = out_buff + (tex_top + y) * width * 4 + tex_left * 4;

        for (unsigned long x = 0; x < tex_width && x + tex_left < width; x++) {
          switch (pixel_type) {
            case PIXEL_TYPE_ALPHA:
            case PIXEL_TYPE_RGBA:
              for (unsigned long p = 0; p < pixel_depth; p++) {
                dst[p] = src[p];
              }
              break;

            case PIXEL_TYPE_LUMA:
              dst[0] = 0xFF;
              dst[1] = src[0];
              dst[2] = src[0];
              dst[3] = src[0];
              break;

            case PIXEL_TYPE_ALPHALUMA:
              dst[0] = src[0];
              dst[1] = src[1];
              dst[2] = src[1];
              dst[3] = src[1];
              break;
          }

          src += pixel_depth;
          dst += 4;
        }
      }

      delete chunk_objects;
      delete tex_objects;
    }

    as::write_bmp(as::get_file_prefix(filename) + as::stringf("+%03d.bmp", i),
                  out_buff,
                  out_len,
                  width,
                  height,
                  4,
                  as::WRITE_BMP_BIGENDIAN | as::WRITE_BMP_FLIP);

    delete [] out_buff;
    delete textures_offsets;
    delete image_objects;
  }

  delete imagelist_offsets;

  return true;
}

bool proc_psb_imageinfo(const string& filename,
                        const psb_t&  psb)
{
  const psb_objects_t* psb_objects = psb.get_objects();

  if (!psb_objects || !psb_objects->get_data("diff")) {
    return false;
  }

  unsigned long  w         = psb.get_number(psb_objects->get_data("w"));
  unsigned long  h         = psb.get_number(psb_objects->get_data("h"));
  unsigned long  diffbase  = psb.get_number(psb_objects->get_data("diffbase"));

  psb_objects_t* diff_objects = NULL;
  psb_objects->unpack(diff_objects, "diff");

  psb_objects_t* crop_objects = NULL;
  psb_objects->unpack(crop_objects, "crop");

  FILE* fh = as::open_or_die_file(as::get_file_prefix(filename) + ".txt", "w");

  fprintf(fh, "w: %d\n", w);
  fprintf(fh, "h: %d\n", h);
  fprintf(fh, "diffbase: %d\n", diffbase);    

  if (diff_objects) {
    for (unsigned long i = 0; i < diff_objects->size(); i++) {
      string         name  = diff_objects->get_name(i);
      unsigned long  value = psb.get_number(diff_objects->get_data(i));

      fprintf(fh, "diff_%s: %d\n", name.c_str(), value);
    }
  }

  for (unsigned long i = 0; i < crop_objects->size(); i++) {
    string         name = crop_objects->get_name(i);
    unsigned long  value = psb.get_number(crop_objects->get_data(i));

    fprintf(fh, "crop_%s: %d\n", name.c_str(), value);
  }

  fclose(fh);

  delete crop_objects;
  delete diff_objects;

  return true;
}

bool proc_psb_file(const string&  filename,
                   unsigned char* buff,
                   unsigned long  len)
{
  if (len < 3 || memcmp(buff, "PSB", 3)) {
    return false;
  }

  psb_t psb(buff);
  
  return proc_psb_image(filename, psb) || proc_psb_imageinfo(filename, psb);
}

int main(int argc, char** argv) {
  if (argc < 4) {
    fprintf(stderr, "exm2lib v1.02 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.psb.m> <input.bin> [game index]\n\n", argv[0]);

    for (unsigned long i = 0; i < GAME_INFO_COUNT; i++) {
      fprintf(stderr, "\tgame_index -> %d = %s\n", i, GAME_INFO[i].name.c_str());
    }

    return -1;
  }

  string        in_psb_filename(argv[1]);
  string        in_bin_filename(argv[2]);
  unsigned long game_index = atol(argv[3]);

  if (game_index >= GAME_INFO_COUNT) {
    fprintf(stderr, "Invalid game index: %d\n", game_index);
    return -1;
  }

  const game_info_t& game_info = GAME_INFO[game_index];

  int fd = as::open_or_die(in_psb_filename, O_RDONLY | O_BINARY);

  unsigned char* toc_buff = NULL;
  unsigned long  toc_len  = NULL;
  read_decrypt_decompress(fd, game_info, in_psb_filename, as::get_file_size(fd), toc_buff, toc_len);
  close(fd);

  fd = as::open_or_die(in_bin_filename, O_RDONLY | O_BINARY);

  psb_t                psb(toc_buff);
  const psb_objects_t* psb_objects = psb.get_objects();

  psb_objects_t* info_objects = NULL;
  psb_objects->unpack(info_objects, "file_info");

  for (unsigned long i = 0; i < info_objects->size(); i++) {
    string         entry_name = info_objects->get_name(i);
    unsigned char* entry_buff = info_objects->get_data(i);
    psb_offsets_t* entry_loc  = NULL;
    psb.unpack(entry_loc, entry_buff);

    string         filename = psb.make_filename(entry_name);
    unsigned long  offset   = psb.get_number(entry_loc->get(0));
    unsigned long  comp_len = psb.get_number(entry_loc->get(1));
    unsigned long  len      = 0;
    unsigned char* buff     = NULL;
    lseek(fd, offset, SEEK_SET);
    read_decrypt_decompress(fd, game_info, filename, comp_len, buff, len);

    if (!proc_psb_file(filename, buff, len)) {
      as::write_file(filename, buff, len);
    }

    delete [] buff;
    delete entry_loc;
  }

  delete info_objects;
  delete [] toc_buff;

  close(fd);

  return 0;
}
