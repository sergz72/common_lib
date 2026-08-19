#include "board.h"
#include <common_printf.h>
#include <stdio.h>
#include <stdarg.h>
#include <myprintf.h>

int common_printf(const char *format, ...)
{
  char buffer[PRINTF_BUFFER_LENGTH];
  va_list vArgs;
  int rc;

  va_start(vArgs, format);
#ifdef USE_MYVSPRINTF
  rc = myvsprintf(buffer, format, vArgs);
#else
  rc = vsnprintf(buffer, sizeof(buffer), format, vArgs);
#endif
  va_end(vArgs);
  puts_(buffer);
  return rc;
}
