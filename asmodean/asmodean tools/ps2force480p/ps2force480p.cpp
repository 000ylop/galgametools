// ps2force480p.cpp, v1.14 2008/05/18
// coded by asmodean

// contact: 
//   web:   http://plaza.rakuten.co.jp/asmodean
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This tool attempts to patch PlayStation2 games to force display at 480p.
// There are similar tools which can do so at runtime (HDTV Xploder), but
// this interferes with some games and hdloader I hear.

// Be aware that the tool may not always find a patch location.  Even if it
// does, the patch may not work well with all games.  Do not use this if
// you are a whining annoying idiot.

#include "as-util.h"
#include <list>

// Special patch to fix Persona 3's frame rate.  Need to implement a more
// general solution though...
#define PERSONA3_SPEED

struct elfhdr_t {
  unsigned char  e_ident[16];
  unsigned short e_type;
  unsigned short e_machine;
  unsigned long  e_version;
  unsigned long  e_entry;
  unsigned long  e_phoff;
  unsigned long  e_shoff;
  unsigned long  e_flags;
  unsigned short e_ehsize;
  unsigned short e_phentsize;
  unsigned short e_phnum;
  unsigned short e_shentsize;
  unsigned short e_shnum;
  unsigned short e_shstrndx;
};

struct elf_phdr_t {
  unsigned long p_type;
  unsigned long p_offset;
  unsigned long p_vaddr;
  unsigned long p_paddr;
  unsigned long p_filesz;
  unsigned long p_memsz;
  unsigned long p_flags;
  unsigned long p_align;
};

static const unsigned char ELF_SIG[]
  = { 0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 0x00, 
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const unsigned char SCEGSRESETGRAPH_SIG[] 
  = { 0xB0, 0xFF, 0xBD, 0x27, 0x00, 0x24, 0x04, 0x00, 
      0x30, 0x00, 0xB3, 0xFF, 0x00, 0x2C, 0x05, 0x00, 
      0x20, 0x00, 0xB2, 0xFF, 0x00, 0x34, 0x06, 0x00, 
      0x10, 0x00, 0xB1, 0xFF, 0x00, 0x3C, 0x07, 0x00, 
      0x40, 0x00, 0xBF, 0xFF, 0x03, 0x24, 0x04, 0x00,
      0x00, 0x00, 0xB0, 0xFF, 0x03, 0x8C, 0x05, 0x00, 
      0x03, 0x94, 0x06, 0x00 };

// Matches four instructions after start
static const unsigned char SCEGSPUTDISPENV_SIG[]
  = { 0x2D, 0x80, 0x80, 0x00, 0x06, 0x00, 0x43, 0x84,
      0x01, 0x00, 0x02, 0x24, 0x11, 0x00, 0x62, 0x14, 
      0x00, 0x00, 0x04, 0xDE, 0x00, 0x12, 0x02, 0x3C, 
      0x00, 0x12, 0x03, 0x3C, 0x00, 0x12, 0x06, 0x3C };

// We will use an unexpected error string to store some patch bytes. If this
// string doesn't exist in some target, well ... pick something else (need
// at least 44 bytes).
static const char CLOBBER_STR1[]
  = "sceGsExecStoreImage: Enough data does not reach VIF1";

// Going to clobber multiple strings starting with this one!
static const char CLOBBER_STR2[]
  = "sceGsExecStoreImage: DMA Ch.1 does not terminate";

unsigned long find_vaddr(elf_phdr_t*   phdrs, 
                         unsigned long phdr_count, 
                         unsigned long offset)
{
  for (unsigned long i = 0; i < phdr_count; i++) {
    if (offset >= phdrs[i].p_offset && offset < (phdrs[i].p_offset + phdrs[i].p_filesz)) {
      return (offset - phdrs[i].p_offset) + phdrs[i].p_vaddr;
    }
  }

  return -1;
}

// May need to implement wildcarding if the library function signatures are volatile
unsigned char* find_pattern(unsigned char*       start,
                            unsigned char*       end,
                            const unsigned char* pattern,
                            unsigned long        pattern_len)
{
  for (unsigned char* p = start; p + pattern_len < end; p += 4) {
    if (!memcmp(p, pattern, pattern_len)) {
      return p;
    }
  }

  return NULL;
}

inline unsigned long mips_get_opr(unsigned long code) {return (code & 0xFC000000) >> 26;}
inline unsigned long mips_get_src(unsigned long code) {return (code & 0x03E00000) >> 21;}
inline unsigned long mips_get_tgt(unsigned long code) {return (code & 0x001F0000) >> 16;}
inline unsigned long mips_get_val(unsigned long code) {return code & 0x0000FFFF;}

inline unsigned long mips_simple(unsigned long opr, 
                                 unsigned long src, 
                                 unsigned long tgt, 
                                 unsigned long val) 
{
  return (opr << 26) | (src << 21) | (tgt << 16) | val;
}

typedef std::list<unsigned char*> addr_list_t;

// This will only accurately find extremely localized sets of operations
addr_list_t find_sd(unsigned char* start,
                    unsigned char* end,
                    unsigned long  target_addr)
{
  addr_list_t addr_list;

  unsigned long regs[32] = { 0 };

  for (unsigned char* p = start; p + 4 < end; p += 4) {
    unsigned long* code = (unsigned long*) p;

    unsigned long o = mips_get_opr(*code);
    unsigned long s = mips_get_src(*code);
    unsigned long t = mips_get_tgt(*code);
    unsigned long v = mips_get_val(*code);

    switch (o) {
    case 0x0F: // lui    
      regs[t] = v << 16;
      break;    

    case 0x09: // addiu
      if (v & 0x8000) v &= 0xFFFF0000;

      regs[t] = regs[s] + v;
      break;

    case 0x0D: // ori
      regs[t] = regs[s] | v;
      break;

    case 0x3F: // sd
      if (v & 0x8000) v &= 0xFFFF0000;

      if (regs[s] + v == target_addr) {
        addr_list.push_back(p);
      }
      break;
    }
  }

  return addr_list;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "ps2force480p v1.14 by asmodean (asmodean [at] hush.com)\n\n"
                    "usage: %s <input> [DY]\n\n"
                    "\t input - Executable or an ISO (will scan for executables).\n"
                    "\t         This input file will be modified!\n\n"
                    "\t DY    - If specified, overrides the display vertical displacement.\n"
                    "\t         Specify \"aggressive\" to perform an aggressive patch of\n"
                    "\t         all display mode modifications.  This is more dangerous!\n\n"
                    "\t NOTE: Ar Tonelico 2 (SLPS_258.19) uses DY=35 by default and the\n"
                    "\t       screen is cut off at the top.  DY=51 centers it, but I am\n"
                    "\t       not sure how general these numbers will be for other games.\n",
                    argv[0]);
    return -1;
  }
  
  string        in_filename(argv[1]);
  unsigned long  new_dy          = -1;
  bool           simple_dy       = false;
  bool           aggressive_dy   = false;
  unsigned char* clobber_pat     = NULL;
  unsigned long  clobber_pat_len = 0;

  if (argc > 2) {
    string dy_param = argv[2];

    if (dy_param == "aggressive") {
      aggressive_dy   = true;
      clobber_pat     = (unsigned char*) CLOBBER_STR2;
      clobber_pat_len = sizeof(CLOBBER_STR2) - 1;
    } else {
      simple_dy       = true;
      new_dy          = atol(dy_param.c_str());
      clobber_pat     = (unsigned char*) CLOBBER_STR1;
      clobber_pat_len = sizeof(CLOBBER_STR1) - 1;
    }
  }

  if (simple_dy) {
    printf("Will attempt simple DY patching (to %d)\n", new_dy);
  }

  if (aggressive_dy) {
    printf("Will attempt aggressive DY patching (+16) -- dangerous!\n");
  }

  int fd = as::open_or_die(in_filename, O_RDWR | O_BINARY); 

  for (unsigned long sector_num = 0; true; sector_num++) {
    unsigned char sector[2048];
    if (read(fd, sector, sizeof(sector)) != sizeof(sector)) {
      break;
    }

    if (!memcmp(sector, ELF_SIG, sizeof(ELF_SIG))) {
      printf("Scanning ELF at sector %d\n", sector_num);

      elfhdr_t* hdr = (elfhdr_t*) sector;

      unsigned long  len  = (hdr->e_shoff + 2047) & ~2047;
      unsigned char* buff = new unsigned char[len];
      _lseeki64(fd, -(int)sizeof(sector), SEEK_CUR);
      len = read(fd, buff, len);

      elf_phdr_t*    phdrs = (elf_phdr_t*) (buff + hdr->e_phoff);
      unsigned char* start = buff + phdrs[0].p_offset;
      unsigned char* end   = buff + len;

      unsigned long want_patches = 1;

      // Easy patch to sceGsResetGraph forcing mode to 480p
      unsigned char* p = find_pattern(start,
                                      end,
                                      SCEGSRESETGRAPH_SIG,
                                      sizeof(SCEGSRESETGRAPH_SIG));
      if (p) {      
        printf("  - patching sceGsResetGraph() at 0x%X\n", p - buff);

        unsigned long* opcodes = (unsigned long*) p;
        
        opcodes[11] = 0x24110000; // addiu s1, zero, $0000
        opcodes[12] = 0x24120050; // addiu s2, zero, $0050
        opcodes[15] = 0x24130001; // addiu s3, zero, $0001

        want_patches--;
      }

      unsigned long clobber_off = 0;

      if (clobber_pat_len) {
        want_patches++;

        // Find somewhere to put our patch instructions
        p = find_pattern(start, end, clobber_pat, clobber_pat_len);
        if (p) {
          want_patches--;

          clobber_off = (unsigned long) (p - buff);
          printf("  - found clobber location at 0x%X\n", clobber_off);
        }
      }

      // Patch all stores to some GS registers
      if (aggressive_dy) {
        unsigned long  patch_loc     = find_vaddr(phdrs, hdr->e_phnum, clobber_off);
        unsigned long* patch_opcodes = (unsigned long*) (buff + clobber_off);
        unsigned long  patch_idx     = 0;

        addr_list_t addr_list = find_sd(start, end, 0x12000080);

        addr_list.splice(addr_list.end(), find_sd(start, end, 0x120000A0));

        for (addr_list_t::iterator i = addr_list.begin();
             i != addr_list.end();
             i++) {
          want_patches++;

          // One instruction prior to the match
          unsigned long  from_off     = (unsigned long) (*i - buff - 4);
          unsigned long  from_loc     = find_vaddr(phdrs, hdr->e_phnum, from_off);
          unsigned long* from_opcodes = (unsigned long*) (buff + from_off);

          printf("  - patching raw display modification at 0x%X\n", from_off);
          
          unsigned long ret_loc     = from_loc + (2 * 4); 
          unsigned long patch_start = patch_loc / 4 + patch_idx;

          unsigned long delay_opcode = 0;
                       
          switch (mips_get_opr(from_opcodes[0])) {
            case 4: // beq
              if (mips_get_src(from_opcodes[0]) == 0 &&
                  mips_get_tgt(from_opcodes[0]) == 0)
              {
                unsigned long branch_tgt = from_opcodes[0] & 0x3FFFFFF;
                if (branch_tgt & 0x2000000) branch_tgt |= 0xFC000000;

                ret_loc = from_loc + (branch_tgt + 1) * 4;             
                break;
              }
            // else deliberate fall through
            case 1: // bgez, bgezal, bltz, bltzal
            case 7: // bgtz
            case 6: // blez
            case 5: // bne
              printf("  -- in delay slot for unsupported branch instruction (%08X)\n", from_opcodes[0]);
              continue;

            default:
              delay_opcode = from_opcodes[0];
          }

          want_patches--;

          unsigned long fix_reg  = mips_get_tgt(from_opcodes[1]);
          unsigned long temp_reg = fix_reg == 5 ? 6 : 5;

          patch_opcodes[patch_idx++] = mips_simple(0x3F, 0x1D, temp_reg, 0xFFF8);                         // sd temp_reg, $FFF8(sp)
          if (!delay_opcode) delay_opcode = patch_opcodes[--patch_idx];
          patch_opcodes[patch_idx++] = mips_simple(0x09, 0x00, temp_reg, 0x0010);                         // addiu temp_reg, 0, $0010
          patch_opcodes[patch_idx++] = 0x00000000 | (12 << 6) | (temp_reg << 11) | (temp_reg << 16);      // sll temp_reg, temp_reg, $000C
          patch_opcodes[patch_idx++] = 0x0000002D | (fix_reg << 11) | (fix_reg << 16) | (temp_reg << 21); // addu fix_reg, fix_reg, temp_reg
          patch_opcodes[patch_idx++] = mips_simple(0x37, 0x1D, temp_reg, 0xFFF8);                         // ld temp_reg, $FFF8(sp)
          patch_opcodes[patch_idx++] = 0x08000000 | (ret_loc / 4);                                        // j ret_loc
          patch_opcodes[patch_idx++] = from_opcodes[1];                                                   // sd, ??, 0x12000080

          from_opcodes[0] = 0x08000000 | patch_start; // j patch_start
          from_opcodes[1] = delay_opcode;
        }
      }

      if (simple_dy) {
        want_patches++;

        // Insert our patch into sceGsPutDispEnv() to fix the Y position.
        p = find_pattern(start, end, SCEGSPUTDISPENV_SIG, sizeof(SCEGSPUTDISPENV_SIG));
        if (p) {
          unsigned long from_off = (unsigned long) (p - buff - 16);
          printf("  - patching sceGsPutDispEnv() at 0x%X\n", from_off);

          unsigned long patch_loc = find_vaddr(phdrs, hdr->e_phnum, clobber_off);
          unsigned long from_loc  = find_vaddr(phdrs, hdr->e_phnum, from_off);
          unsigned long ret_loc   = from_loc + (3 * 4); 
    
          unsigned long* from_opcodes  = (unsigned long*) (buff + from_off);
          unsigned long* patch_opcodes = (unsigned long*) (buff + clobber_off);
 
          unsigned long i = 0;
          patch_opcodes[i++] = from_opcodes[1];
          patch_opcodes[i++] = 0x8C900018;                 // lw s0, $0018(a0)
          patch_opcodes[i++] = 0x3C02FF80;                 // lui v0, $FF80
          patch_opcodes[i++] = 0x24420FFF;                 // addiu v0, v0, $0FFF
          patch_opcodes[i++] = 0x00501024;                 // and v0, v0, s0
          patch_opcodes[i++] = 0x24100000 | new_dy;        // addiu $s0, 0, new_dy
          patch_opcodes[i++] = 0x00108300;                 // sll s0, s0, $000C
          patch_opcodes[i++] = 0x02028025;                 // or s0, s0, v0
          patch_opcodes[i++] = 0x08000000 | (ret_loc / 4); // j ret_loc
          patch_opcodes[i++] = 0xAC900018;                 // sw s0, $0018(a0)          

          from_opcodes[1] = 0x08000000 | (patch_loc / 4); // j patch_loc

          want_patches--;
        }
      }

#ifdef PERSONA3_SPEED
      static const unsigned char VSYNC_HANDLER_SIG[]
        = { 0x00, 0x12, 0x02, 0x3C, 0x00, 0x10, 0x42, 0xDC,
            0x7A, 0x13, 0x02, 0x00, 0x01, 0x00, 0x42, 0x30,
            0x06, 0x00, 0x40, 0x14, 0x00, 0x00, 0x00, 0x00,
            0x0F, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x42 };

      p = find_pattern(start, end, VSYNC_HANDLER_SIG, sizeof(VSYNC_HANDLER_SIG));
      if (p) {
        unsigned long  patch_off      = (unsigned long) (p - buff);
        unsigned long  patch_loc      = find_vaddr(phdrs, hdr->e_phnum, patch_off);
        unsigned long* patch_opcodes  = (unsigned long*) (buff + patch_off);

        printf("  - patching Persona 3 INTC_VBLANK_S handler at 0x%X\n", patch_off);

        unsigned long i = 0;
	      patch_opcodes[i++] = 0x00000000; // see below
	      patch_opcodes[i++] = 0x00000000; // see below
	      patch_opcodes[i++] = 0x8c820000; // lw v0, $0000(a0)
	      patch_opcodes[i++] = 0x38420001; // xori v0, v0, $0001
	      patch_opcodes[i++] = 0xac820000; // sw v0, $0000(a0)
        patch_opcodes[i++] = 0x000217fc; // dsll32 v0, v0, 31
        patch_opcodes[i++] = 0x000217ff; // dsra32 v0, v0, 31
	      patch_opcodes[i++] = 0x0000000f; // sync
	      patch_opcodes[i++] = 0x42000038; // ei
	      patch_opcodes[i++] = 0x03e00008; // jr ra
	      patch_opcodes[i++] = 0x00000000; // nop

        unsigned long data_loc = patch_loc + (i * 4);

        patch_opcodes[0]   = 0x3c040000 | (data_loc >> 16);    // lui a0, high
	      patch_opcodes[1]   = 0x34840000 | (data_loc & 0xFFFF); // ori a0, a0, low

        patch_opcodes[i++] = 0x00000001; // data
      }
#endif

      if (want_patches) {
        printf("  - didn't perform all patches (discarding changes)\n");
      } else {
        printf("  - changes written\n", p - buff);
        _lseeki64(fd, -(int)len, SEEK_CUR);
        write(fd, buff, len);
      }

      sector_num += len / 2048;

      delete [] buff;
    }
  }

  close(fd);

  return 0;
}
