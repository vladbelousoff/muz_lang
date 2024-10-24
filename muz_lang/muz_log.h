#pragma once

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define MUZ_SEPARATOR '\\'
#else
#define MUZ_SEPARATOR '/'
#endif

#define MUZ_FILENAME                               ((char*)(strrchr(__FILE__, MUZ_SEPARATOR) + 1))

#define LogPrint_(lvl, file, line, func, fmt, ...) printf("[%s] %s:%u (%s) " fmt "\n", lvl, file, line, func, ##__VA_ARGS__)

#if MUZ_DEBUG_LEVEL >= 4
#define MuzLogI printf
#else
#define MuzLogI(_fmt, ...)                                                                                                                                                         \
   do {                                                                                                                                                                            \
   } while (0)
#endif

#if MUZ_DEBUG_LEVEL >= 3
#define MuzLogD(_fmt, ...) LogPrint_("D", MUZ_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define MuzLogD(_fmt, ...)                                                                                                                                                         \
   do {                                                                                                                                                                            \
   } while (0)
#endif

#if MUZ_DEBUG_LEVEL >= 2
#define MuzLogW(_fmt, ...) LogPrint_("W", MUZ_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define MuzLogW(_fmt, ...)                                                                                                                                                         \
   do {                                                                                                                                                                            \
   } while (0)
#endif

#if MUZ_DEBUG_LEVEL >= 1
#define MuzLogE(_fmt, ...) LogPrint_("E", MUZ_FILENAME, __LINE__, __FUNCTION__, _fmt, ##__VA_ARGS__)
#else
#define MuzLogE(_fmt, ...)                                                                                                                                                         \
   do {                                                                                                                                                                            \
   } while (0)
#endif
