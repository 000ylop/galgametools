// exfavbin.cpp, v1.0 2012/09/04
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool extracts *.bin archives used by FAVORITE and others.

#include "as-util.h"
#include "zlib.h"

struct BINHDR {
  uint32_t entry_count;
  uint32_t filenames_length;
};

struct BINENTRY {
  uint32_t filename_offset;
  uint32_t offset;
  uint32_t length;
};

struct HZC1HDR {
  uint8_t  signature[4]; // "hzc1"
  uint32_t original_length;
  uint32_t header_length;
};

struct NVSGHDR {
  uint8_t  signature[4]; // "NVSG"
  uint16_t unknown1;
  uint16_t type;
  uint16_t width;
  uint16_t height;
  uint16_t offset_x;
  uint16_t offset_y;
  uint32_t unknown2;
  uint32_t entry_count;
  uint32_t unknown3;
  uint32_t unknown4;
};

static const uint16_t NVSGHDR_TYPE_SINGLE_24BIT = 0;
static const uint16_t NVSGHDR_TYPE_SINGLE_32BIT = 1;
static const uint16_t NVSGHDR_TYPE_MULTI_32BIT  = 2;
static const uint16_t NVSGHDR_TYPE_SINGLE_8BIT  = 3;
static const uint16_t NVSGHDR_TYPE_SINGLE_1BIT  = 4;

bool proc_nvsg(const string& filename,
               uint8_t*      buff,
               uint32_t      len)
{
  if (len < 4 || memcmp(buff, "hzc1", 4)) {
    return false;
  }

  auto     hzc1hdr   = (HZC1HDR*) buff;
  uint32_t data_len  = len - sizeof(*hzc1hdr);
  auto     data_buff = (uint8_t*) (hzc1hdr + 1);

  if (data_len < 4 || memcmp(data_buff, "NVSG", 4)) {
    return false;
  }

  auto nvsghdr = (NVSGHDR*) data_buff;
  data_buff += hzc1hdr->header_length;
  
  uint32_t depth = 0;

  switch (nvsghdr->type) {
  case NVSGHDR_TYPE_SINGLE_24BIT:
    depth = 3;
    break;

  case NVSGHDR_TYPE_SINGLE_32BIT:
  case NVSGHDR_TYPE_MULTI_32BIT:
    depth = 4;
    break;

  case NVSGHDR_TYPE_SINGLE_8BIT:
    depth = 1;
    break;

  case NVSGHDR_TYPE_SINGLE_1BIT:
    depth = 1;
    break;

  default:
    return false;
  }

  uLongf out_len  = hzc1hdr->original_length;
  auto   out_buff = new uint8_t[out_len];
  uncompress(out_buff, &out_len, data_buff, data_len);

  if (nvsghdr->type == NVSGHDR_TYPE_SINGLE_1BIT) {
    for (uint32_t i = 0; i < out_len; i++) {
      if (out_buff[i] == 1) {
        out_buff[i] = 0xFF;
      }
    }
  }

  if (nvsghdr->entry_count) {
    uint32_t frame_len = nvsghdr->width * nvsghdr->height * depth;

    for (uint32_t j = 0; j < nvsghdr->entry_count; j++) {
      as::write_bmp(filename + as::stringf("+%03d+x%dy%d.bmp", j, nvsghdr->offset_x, nvsghdr->offset_y),
                    out_buff + (j * frame_len),
                    frame_len,
                    nvsghdr->width,
                    nvsghdr->height,
                    depth,
                    as::WRITE_BMP_FLIP);
    }
  } else {
    as::write_bmp(filename + ".bmp",
                  out_buff,
                  out_len,
                  nvsghdr->width,
                  nvsghdr->height,
                  depth,
                  as::WRITE_BMP_FLIP);
  }

  delete [] out_buff;

  return true;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "exfavbin v1.0, coded by asmodean\n\n");
    fprintf(stderr, "usage: %s <input.bin>\n", argv[0]);
    return -1;
  }

  string in_filename = argv[1];

  int fd = as::open_or_die(in_filename, O_RDONLY | O_BINARY); 

  BINHDR hdr;
  read(fd, &hdr, sizeof(hdr));

  auto entries = new BINENTRY[hdr.entry_count];
  read(fd, entries, sizeof(BINENTRY) * hdr.entry_count);

  uint32_t filenames_len  = hdr.filenames_length;
  auto     filenames_buff = new uint8_t[filenames_len];
  read(fd, filenames_buff, filenames_len);

  for (uint32_t i = 0; i < hdr.entry_count; i++) {
    string filename = (char*) (filenames_buff + entries[i].filename_offset);

    uint32_t len  = entries[i].length;
    auto     buff = new uint8_t[len];
    lseek(fd, entries[i].offset, SEEK_SET);
    read(fd, buff, len);

    if (!proc_nvsg(filename, buff, len)) {
      as::write_file(filename + as::guess_file_extension(buff, len), buff, len);
    }

    delete [] buff;
  }

  delete [] filenames_buff;
  delete [] entries;

  close(fd);

  return 0;
}
