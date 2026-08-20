#ifndef LOGGER_H
#define LOGGER_H

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARNING 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3

typedef void (*log_func)(int level, const char *format, ...);

#endif
