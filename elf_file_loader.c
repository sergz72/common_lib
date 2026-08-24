#include "board.h"
#include <elf_file_loader.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  symbol_table_entry *symbol_table;
  unsigned int symbol_table_size;
  char *symbol_strings;
} symbol_table_info;

static unsigned int calc_args_size(int argc, const char **argv)
{
  unsigned int size = argc * sizeof(char*);
  while (argc--)
    size += strlen(*argv++) + 1;
  return size;
}

static void argcpy(void *p, int argc, const char **argv)
{
  char **argvp = p;
  char *argp = (char*)p + argc * sizeof(char*);
  for (int i = 0; i < argc; i++)
  {
    const char *arg = *argv++;
    size_t l = strlen(arg);
    strcpy(argp, arg);
    *argvp++ = argp;
    argp += l + 1;
  }
}

static int compare_function_def(const void *a, const void *b)
{
  function_def *fa = (function_def*)a;
  function_def *fb = (function_def*)b;
  return strcmp(fa->name, fb->name);
}

static const void* find_function_address(const char* name, const function_def* function_map, unsigned int function_map_size)
{
  function_def f;
  f.name = (char*)name;
  const function_def *ff = bsearch(&f, function_map, function_map_size, sizeof(function_def), compare_function_def);
  return ff ? ff->pointer : nullptr;
}

static unsigned long long int get_symbol_value_by_name(const char *name, const symbol_table_info *symbols)
{
  unsigned int size = symbols->symbol_table_size;
  const symbol_table_entry *p = symbols->symbol_table;

  while (size--)
  {
    unsigned char bind = p->info >> 4;
    const char *symbol_name = symbols->symbol_strings + p->name_offset;
    if (bind == 1 && !strcmp(name, symbol_name))
      return p->value;
    p++;
  }

  return ULLONG_MAX;
}

int elf_file_load(const void *data, const function_def *function_map, unsigned int function_map_size,
                  unsigned int stack_size, int argc, const char **argv, app_image *image)
{
  const elf_header *h = data;

  int rc = elf_header_check(h);
  if (rc != 0)
    return rc;
  if (h->type != 3 && h->type != 2)
    return 4;

  const void *textp = nullptr;
  const got_item *gotp = nullptr;
  const void *datap = nullptr;
  const symbol_table_entry *dynsymp = nullptr;
  const char *dynstrp = nullptr;
  const relocation_table_entry *relap = nullptr;
  unsigned int got_size = 0;
  unsigned int data_size = 0;
  unsigned int data_copy_size = 0;
  symbol_table_info symbols = {0};
  int rela_size = 0;
  const char *section_header_table = (const char *)data + h->section_header_table_offset;
  elf_section_header *strings_section = (elf_section_header *) (section_header_table +
                                                                h->section_header_table_entry_size *
                                                                h->section_header_string_table_section);
  char *strings = (char *)data + strings_section->offset;
  for (unsigned short i = 0; i < h->section_header_table_entries_count; i++)
  {
    elf_section_header *sh = (elf_section_header *)section_header_table;
    const char *name = strings + sh->name_offset;
    if (i == 1)
    {
      if (strcmp(name, ".text"))
        return 5;
      textp = (char*)data + sh->offset;
    } else if (!strcmp(name, ".got"))
    {
      gotp = (const got_item)((char*)data + sh->offset);
      got_size = sh->size;
    } else if (!strcmp(name, ".sdata") || !strcmp(name, ".data"))
    {
      if (!datap)
        datap = (char*)data + sh->offset;
      data_size += sh->size;
      data_copy_size += sh->size;
    } else if (!strcmp(name, ".bss"))
    {
      if (!datap)
        datap = (char*)data + sh->offset;
      data_size += sh->size;
    } else if (!strcmp(name, ".rela.plt"))
    {
      relap = (const relocation_table_entry*)((char*)data + sh->offset);
      rela_size = (int)sh->size;
    } else if (!strcmp(name, ".dynsym"))
      dynsymp = (const symbol_table_entry*)((char*)data + sh->offset);
    else if (!strcmp(name, ".dynstr"))
      dynstrp = (char*)data + sh->offset;
    else if (sh->type == 3 && !strcmp(name, ".strtab"))
      symbols.symbol_strings = (char*)data + sh->offset;
    else if (sh->type == 2)
    {
      symbols.symbol_table = (symbol_table_entry *)((char *) data + sh->offset);
      symbols.symbol_table_size = sh->size / sizeof(symbol_table_entry);
    }
    section_header_table += h->section_header_table_entry_size;
  }
  if (!textp || !gotp || !symbols.symbol_table || !symbols.symbol_strings)
    return 6;
  unsigned long long int flash_size = get_symbol_value_by_name("FLASH_SIZE", &symbols);
  if (flash_size == ULLONG_MAX)
    return 7;
  unsigned long long int ram_size = get_symbol_value_by_name("RAM_SIZE", &symbols);
  if (ram_size == ULLONG_MAX)
    return 8;
  const unsigned int text_copy_size = (void*)gotp - textp;
  image->text_size = datap == nullptr ? (unsigned int)flash_size : datap - textp;
  unsigned int data_alloc_size = (unsigned int)ram_size;
  if (data_alloc_size < stack_size)
    data_alloc_size = stack_size * 2;
#ifdef ELF_LOADER_PRINTF
  const unsigned int args_size = calc_args_size(argc, argv);
  unsigned int text_size = text_copy_size + got_size + args_size;
  ELF_LOADER_PRINTF("Text size %d alloc size %d\n", text_size, image->text_size);
  ELF_LOADER_PRINTF("Data size %d alloc size %d copy size %d\n", data_size, data_alloc_size, data_copy_size);
#endif
  image->size = image->text_size + data_alloc_size;
  image->address = elf_file_alloc(image->size, image->text_size);
  if (!image->address)
    return 9;
  image->argvp = (const char**)((char*)image->address + text_copy_size + got_size);
  memcpy(image->address, textp, text_copy_size);
  argcpy(image->argvp, argc, argv);
  memcpy(image->address + image->text_size, datap, data_copy_size);
  if (relap)
  {
    if (!dynsymp || !dynstrp)
      return 10;
    printf("GOT size %d\n", got_size);
    unsigned int got_offset = (void*)gotp - textp;
    while (rela_size > 0)
    {
      if (relap->offset >= got_offset && relap->offset < got_offset + got_size)
      {
#ifdef ELF_LOADER_PRINTF
        ELF_LOADER_PRINTF("Got offset %016llX addend %016llX info %016llX\n", relap->offset, relap->addend, relap->info);
#endif
#ifdef Target64Bit
        unsigned int dynsym_idx = relap->info >> 32;
#else
        unsigned int dynsym_idx = relap->info >> 8;
#endif
        const char *name = dynstrp + dynsymp[dynsym_idx].name_offset;
#ifdef ELF_LOADER_PRINTF
        ELF_LOADER_PRINTF("Relocation name %s\n", name);
#endif
        const void *address = find_function_address(name, function_map, function_map_size);
        if (!address)
          return 11;
        got_item *got_itemp = (got_item*)((char*)image->address + relap->offset);
        *got_itemp = (got_item)address;
      }
      rela_size -= sizeof(relocation_table_entry);
      relap++;
    }
  }
  image->main = image->address + h->entry_offset;
  return 0;
}
