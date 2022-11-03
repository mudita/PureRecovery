#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)


void debug_log_impl(const char *file, const char *func, int line, const char *msg, ...);

#define debug_log(...)  debug_log_impl(__FILENAME__,__func__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif