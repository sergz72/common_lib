#ifndef OBJECT_FILE_LOADER_H
#define OBJECT_FILE_LOADER_H

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || (defined(__riscv) && (__riscv_xlen == 64))
#define Target64Bit
#endif

typedef struct
{
  unsigned char magic;
  char elf[3];
  unsigned char bits;
  unsigned char endianess;
  unsigned char header_version;
  unsigned char os_abi;
  unsigned char padding[8];
  unsigned short type;
  unsigned short instruction_set;
  unsigned int version;
#ifdef Target64Bit
  unsigned long long int entry_offset;
  unsigned long long int program_header_table_offset;
  unsigned long long int section_header_table_offset;
#else
  unsigned int entry_offset;
  unsigned int program_header_table_offset;
  unsigned int section_header_table_offset;
#endif
  unsigned int flags;
  unsigned short header_size;
  unsigned short program_header_table_entry_size;
  unsigned short program_header_table_entries_count;
  unsigned short section_header_table_entry_size;
  unsigned short section_header_table_entries_count;
  unsigned short section_header_string_table_section;
} elf_header;

typedef struct
{
  unsigned int name_offset;
  unsigned int type;
#ifdef Target64Bit
  unsigned long long int flags;
  unsigned long long int address;
  unsigned long long int offset;
  unsigned long long int size;
#else
  unsigned int flags;
  unsigned int address;
  unsigned int offset;
  unsigned int size;
#endif
  unsigned int link;
  unsigned int info;
} elf_section_header;

typedef struct
{
#ifdef Target64Bit
  unsigned int name_offset;
  unsigned char info;
  unsigned char other;
  unsigned short section_index;
  unsigned long long int value;
  unsigned long long int size;
#else
  unsigned int name_offset;
  unsigned int value;
  unsigned int size;
  unsigned char info;
  unsigned char other;
  unsigned short section_index;
#endif
} symbol_table_entry;

typedef struct
{
#ifdef Target64Bit
  unsigned long long int offset;
  unsigned long long int info;
  long long int addend;
#else
  unsigned int offset;
  unsigned int info;
  int addend;
#endif
} relocation_table_entry;

typedef struct
{
#ifdef Target64Bit
  unsigned long long int offset;
  unsigned long long int info;
#else
  unsigned int offset;
  unsigned int info;
#endif
} relocation_table_entry_wo_addens;

typedef struct {
  unsigned int type;   // Segment type (e.g., PT_LOAD, PT_DYNAMIC)
  unsigned int flags;  // Segment permissions (R, W, X)
#ifdef Target64Bit
  unsigned long long int offset; // File offset where the segment starts
  unsigned long long int vaddr;  // Virtual address where segment should be loaded
  unsigned long long int paddr;  // Physical address (rarely used, usually matches vaddr)
  unsigned long long int filesz; // Number of bytes of this segment stored in the file
  unsigned long long int memsz;  // Total number of bytes this segment takes up in memory
  unsigned long long int align;  // Alignment required in memory (usually a power of 2)
#else
  unsigned int offset; // File offset where the segment starts
  unsigned int vaddr;  // Virtual address where segment should be loaded
  unsigned int paddr;  // Physical address (rarely used, usually matches vaddr)
  unsigned int filesz; // Number of bytes of this segment stored in the file
  unsigned int memsz;  // Total number of bytes this segment takes up in memory
  unsigned int align;  // Alignment required in memory (usually a power of 2)
#endif
} elf_program_header;

typedef void *got_item;

#ifdef Target64Bit
#define R_SYM(i)    ((i)>>32)
#define R_TYPE(i)   ((i)&0xffffffffL)
#else
#define R_SYM(i)	((i)>>8)
#define R_TYPE(i)   ((unsigned char)(i))
#endif

typedef struct
{
    char *name;
    void *pointer;
} function_def;

static inline int elf_header_check(const elf_header *h)
{
  if (h->magic != 0x7F || h->elf[0] != 'E' || h->elf[1] != 'L' || h->elf[2] != 'F')
    return 1;
  if (sizeof(void *) == 4 && h->bits != 1)
    return 2;
  if (sizeof(void *) == 8 && h->bits != 2)
    return 2;
  if (h->endianess != 1)
    return 3;
  return 0;
}

int object_file_load(void *data, const function_def *function_map, void *bss, unsigned int bss_size);
int object_file_call(const char* function);

#endif
