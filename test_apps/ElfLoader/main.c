#include <stdio.h>
#include <elf_file_loader.h>
#include <stdlib.h>
#include <syscalls.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>

// function names should be sorted in alphabetical order
const function_def function_map[] = {
  {"osDelay", osDelay},
  {"osExit", osExit},
  {"osLeds", osLeds},
  {"osTaskSwitch", osTaskSwitch},
  {"printf", printf}
};

static app_image image;

static void sleep_ms(long milliseconds)
{
  struct timespec req;
  req.tv_sec = milliseconds / 1000;          // Get whole seconds
  req.tv_nsec = (milliseconds % 1000) * 1000000; // Get remaining nanoseconds

  nanosleep(&req, nullptr);
}

void osTaskSwitch(void)
{
  puts("osTaskSwitch should not be called");
  rwx_free(image.address, image.size);
  exit(1);
}

void osExit(const int code)
{
  printf("osExit %d\n", code);
  rwx_free(image.address, image.size);
  exit(code);
}

void osDelay(const int ms)
{
  printf("osDelay %dms\n", ms);
  sleep_ms(ms);
}

void osLeds(const bool on, const unsigned int leds)
{
  printf("osLeds %d %d\n", on, leds);
}

void *rwx_alloc(unsigned int size)
{
  return mmap(nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                   MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}

void rwx_free(void *p, unsigned int size)
{
  munmap(p, size);
}

int main(int argc, const char **argv)
{
  FILE *pfile;
  void *buffer;
  struct stat st;

  if (argc < 2)
  {
    puts("Usage: ElfLoader elf_file_name [parameters]");
    return 1;
  }

  pfile = fopen(argv[1], "rb");
  if (!pfile)
  {
    puts("fopen error");
    return 2;
  }

  if (fstat(fileno(pfile), &st))
  {
    perror("fstat");
    fclose(pfile);
    return 3;
  }

  buffer = malloc(st.st_size);
  if (!buffer)
  {
    fclose(pfile);
    puts("out of memory");
    return 4;
  }

  if (fread(buffer, 1, st.st_size, pfile) != st.st_size)
  {
    fclose(pfile);
    free(buffer);
    puts("fread error");
    return 5;
  }

  fclose(pfile);

  int rc = elf_file_load(buffer, function_map, sizeof(function_map)/sizeof(function_def), 2048, argc - 1, &argv[1], &image);
  printf("elf_file_load returned %d\n", rc);
  if (rc != 0)
  {
    free(buffer);
    return rc;
  }

  free(buffer);

  image.main(argc - 1, &argv[1]);

  rwx_free(image.address, image.size);

  puts("should not happen");

  return 1;
}
