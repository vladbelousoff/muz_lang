#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define MUZ_SEPARATOR '\\'
#else
#define MUZ_SEPARATOR '/'
#endif

struct muz_logger_context
{
  int (*format_print)(const char* const Format, ...);
};

static struct muz_logger_context muz_logger_context = { printf };

#define MUZ_FILENAME                                    ((char*)(strrchr(__FILE__, MUZ_SEPARATOR) + 1))
#define MUZ_LOG_PRINTF(lvl, file, line, func, fmt, ...) muz_logger_context.format_print("[%s] %s:%u (%s) " fmt "\n", lvl, file, line, func, ##__VA_ARGS__)

#if MUZ_DEBUG_LEVEL >= 4
#define muz_log_info printf
#else
#define muz_log_info(_fmt, ...)                                                                                                                                                    \
  do {                                                                                                                                                                             \
  } while (0)
#endif

#if MUZ_DEBUG_LEVEL >= 3
#define muz_log_debug(_fmt, ...) MUZ_LOG_PRINTF("D", MUZ_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define muz_log_debug(_fmt, ...)                                                                                                                                                   \
  do {                                                                                                                                                                             \
  } while (0)
#endif

#if MUZ_DEBUG_LEVEL >= 2
#define muz_log_warn(_fmt, ...) MUZ_LOG_PRINTF("W", MUZ_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define muz_log_warn(_fmt, ...)                                                                                                                                                    \
  do {                                                                                                                                                                             \
  } while (0)
#endif

#if MUZ_DEBUG_LEVEL >= 1
#define muz_log_error(_fmt, ...) MUZ_LOG_PRINTF("E", MUZ_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define muz_log_error(_fmt, ...)                                                                                                                                                   \
  do {                                                                                                                                                                             \
  } while (0)
#endif
