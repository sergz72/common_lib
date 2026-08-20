#ifndef _ELF_FILE_LOADER
#define _ELF_FILE_LOADER

#include <object_file_loader.h>

typedef struct
{
  void *address;
  void (*main)(int argc, char **argv);
  unsigned int size;
  unsigned int text_size;
} app_image;

void *rwx_alloc(unsigned int size);
void rwx_free(void *p, unsigned int size);

int elf_file_load(const void *data, const function_def *function_map, unsigned int function_map_size,
                  unsigned int stack_size, app_image *image);

#endif
