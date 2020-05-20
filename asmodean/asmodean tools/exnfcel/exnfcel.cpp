// exnfcel.cpp, v1.0 2010/08/10
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts CP10 (*.cel) image composites.

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <Xcompress.h>
#include <Xgraphics.h>
#include <map>

#include "as-util.h"

struct CP10HDR {
  unsigned char signature[4]; // "CP10"
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long length;
  unsigned long unknown4;
};

struct CP10TAG {
  unsigned char signature[4]; // "ENTR", "IMAG" etc
  unsigned long next_offset;
};

struct CP10TAG_ENTR : public CP10TAG {
  unsigned long  index;
  unsigned short anim_index;
  unsigned short unknown2;
  unsigned short offset_x;
  unsigned short offset_y;
  unsigned long  unknown3;
  unsigned long  unknown4;
  unsigned long  unknown5;
  unsigned long  unknown6;
  unsigned long  unknown7;
};

struct CP10TAG_ANIM : public CP10TAG {
  unsigned long  index;
  unsigned long  unknown1;
  unsigned long  entry_count;
  unsigned long  set_index;
};

struct ANIMENTRY {
  unsigned short duration;
  unsigned short imag_index;
};

struct CP10TAG_IMAG : public CP10TAG {
  unsigned long  index;
  unsigned short width;
  unsigned short height;
  unsigned long  unknown1;
  unsigned long  unknown2;
  unsigned long  compression_type;
  unsigned long  length;
};

static const unsigned long COMPRESSION_TYPE_PTC = 3;
static const unsigned long COMPRESSION_TYPE_LZX = 4;

struct CMPHDR {
  unsigned char signature[4]; // "xPTC", "xCmp"
  unsigned long original_length;
  unsigned long length;
};

typedef std::map<unsigned long, CP10TAG_ENTR> idx_to_entr_t;

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exnfcel v1.0 by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.cel>\n", argv[0]);
    return -1;
  }

  string in_filename(argv[1]);
  string prefix = as::get_file_prefix(in_filename);

  int            fd   = as::open_or_die(in_filename, O_RDONLY | O_BINARY);
  unsigned long  len  = as::get_file_size(fd);
  unsigned char* buff = new unsigned char[len];
  read(fd, buff, len);
  close(fd);

  FILE* anim_out = NULL;

  CP10HDR* hdr = (CP10HDR*) buff;

  idx_to_entr_t anim_to_entr;
  idx_to_entr_t imag_to_entr;
  
  unsigned char* p   = (unsigned char*) (hdr + 1);
  unsigned char* end = buff + len;

  unsigned long entr_idx = 0;
  unsigned long imag_idx = 0;

  for (unsigned long i = 0; p < end; i++) {
    CP10TAG* tag = (CP10TAG*) p;

    tag->next_offset = as::flip_endian(tag->next_offset);

    if (!memcmp(tag->signature, "ENTR", 4)) {
      CP10TAG_ENTR* entr = (CP10TAG_ENTR*) p;

      entr->index      = as::flip_endian(entr->index);
      entr->anim_index = as::flip_endian_short(entr->anim_index);
      entr->offset_x   = as::flip_endian_short(entr->offset_x);
      entr->offset_y   = as::flip_endian_short(entr->offset_y);

      anim_to_entr[entr->anim_index] = *entr;

    } else if (!memcmp(tag->signature, "ANIM", 4)) {
      CP10TAG_ANIM* anim = (CP10TAG_ANIM*) p;
      p += sizeof(*anim);

      anim->index       = as::flip_endian(anim->index);
      anim->entry_count = as::flip_endian(anim->entry_count);
      anim->set_index   = as::flip_endian(anim->set_index);

      CP10TAG_ENTR& entr = anim_to_entr[anim->index];

      ANIMENTRY* animentries = (ANIMENTRY*) p;

      for (unsigned long j = 0; j < anim->entry_count; j++) {
        animentries[j].duration   = as::flip_endian_short(animentries[j].duration);
        animentries[j].imag_index = as::flip_endian_short(animentries[j].imag_index);

        imag_to_entr[animentries[j].imag_index] = entr;

        if (!anim_out) {
          anim_out = as::open_or_die_file(prefix + "+anim.txt", "w");
          fprintf(anim_out, "# SET_ID DURATION X Y IMAGE_INDEX\n");
        }

        fprintf(anim_out, 
                "%d %d %d %d %d\n", 
                anim->set_index, 
                animentries[j].duration, 
                entr.offset_x, 
                entr.offset_y, 
                animentries[j].imag_index);
      }
    } else if (!memcmp(tag->signature, "IMAG", 4)) {
      CP10TAG_IMAG* imag = (CP10TAG_IMAG*) p;
      p += sizeof(*imag);      

      imag->index            = as::flip_endian(imag->index);
      imag->width            = as::flip_endian_short(imag->width);
      imag->height           = as::flip_endian_short(imag->height);
      imag->compression_type = as::flip_endian(imag->compression_type);
      imag->length           = as::flip_endian(imag->length);    

      string out_filename = as::stringf("%s+%05d", prefix.c_str(), imag->index);

      idx_to_entr_t::iterator entr = imag_to_entr.find(imag->index);
      if (entr != imag_to_entr.end()) {
        out_filename += as::stringf("+x%dy%d", entr->second.offset_x, entr->second.offset_y);
      }

      switch (imag->compression_type) {
        case COMPRESSION_TYPE_PTC:
          {
            CMPHDR* cmphdr = (CMPHDR*) p;
            p += sizeof(*cmphdr);

            cmphdr->length = as::flip_endian(cmphdr->length);

            unsigned long  out_stride = imag->width * 4;
            unsigned long  out_len    = imag->height * out_stride;
            unsigned char* out_buff   = new unsigned char[out_len];

            XGPTCDecompressSurface(out_buff,
                                   out_stride,
                                   imag->width,
                                   imag->height,
                                   D3DFMT_LIN_A8R8G8B8,
                                   NULL,
                                   p,
                                   cmphdr->length);

            as::write_bmp(out_filename + ".bmp",
                          out_buff,
                          out_len,
                          imag->width,
                          imag->height,
                          4,
                          as::WRITE_BMP_FLIP | as::WRITE_BMP_BIGENDIAN);

            delete [] out_buff;
          }
          break;

        case COMPRESSION_TYPE_LZX:
          {
            CMPHDR* cmphdr = (CMPHDR*) p;
            p += sizeof(*cmphdr);

            cmphdr->length = as::flip_endian(cmphdr->length);

            XCOMPRESS_FILE_HEADER_LZXNATIVE* xhdr = (XCOMPRESS_FILE_HEADER_LZXNATIVE*) p;
            p += sizeof(*xhdr);

            xhdr->ContextFlags                         = as::flip_endian(xhdr->ContextFlags);
            xhdr->CodecParams.Flags                    = as::flip_endian(xhdr->CodecParams.Flags);
            xhdr->CodecParams.WindowSize               = as::flip_endian(xhdr->CodecParams.WindowSize);
            xhdr->CodecParams.CompressionPartitionSize = as::flip_endian(xhdr->CodecParams.CompressionPartitionSize);

            xhdr->UncompressedSizeLow    = as::flip_endian(xhdr->UncompressedSizeLow);
            xhdr->CompressedSizeLow      = as::flip_endian(xhdr->CompressedSizeLow);
            xhdr->UncompressedBlockSize  = as::flip_endian(xhdr->UncompressedBlockSize);
            xhdr->CompressedBlockSizeMax = as::flip_endian(xhdr->CompressedBlockSizeMax);

            unsigned long  out_len  = 0;
            unsigned char* out_buff = new unsigned char[xhdr->UncompressedSizeLow];

            XMEMDECOMPRESSION_CONTEXT decmpContext = NULL;
            XMemCreateDecompressionContext(XMEMCODEC_LZX, &xhdr->CodecParams, xhdr->ContextFlags, &decmpContext);

            while (out_len != xhdr->UncompressedSizeLow) {
              unsigned long block_len = as::flip_endian(*(unsigned long*) p);
              p += 4;

              SIZE_T decomp_len = xhdr->UncompressedSizeLow;   
              XMemDecompress(decmpContext, out_buff + out_len, &decomp_len, p, block_len);
              p += block_len;

              out_len += decomp_len;
            }

            XMemDestroyDecompressionContext(decmpContext);

            as::write_bmp(out_filename + ".bmp",
                          out_buff,
                          out_len,
                          imag->width,
                          imag->height,
                          4,
                          as::WRITE_BMP_FLIP);

            delete [] out_buff;
          }
          break;

        default:
          as::write_file(out_filename, p, imag->length);
      }
    }

    p = buff + tag->next_offset;
  }

  return 0;
}
