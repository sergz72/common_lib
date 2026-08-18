#include "board.h"
#include <crc_commands.h>
#include <shell.h>
#include <crc32.h>
#include <string.h>

static int crc32_handler(printf_func pfunc, gets_func gfunc, int argc, char **argv, void *data);
static const ShellCommandItem crc32_command_items[] = {
  {nullptr, param_handler, nullptr},
  {nullptr, nullptr, crc32_handler},
  {nullptr, nullptr, nullptr}
};
static const ShellCommand crc32_command = {
  crc32_command_items,
  "crc32",
  "crc32 text",
  nullptr,
  nullptr
};

static int crc32_handler(printf_func pfunc, gets_func gfunc, int argc, char **argv, void *data)
{
  crc32_start();
  crc32_add(argv[0], strlen(argv[0]));
  unsigned int crc = crc32_end();
  pfunc("CRC32 = %08X\n", crc);
  return 0;
}

void register_crc_commands(void)
{
  shell_register_command(&crc32_command);
}
