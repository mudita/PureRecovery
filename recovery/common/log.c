#include "log.h"

#define BUFFER_SIZE 512

void debug_log_impl(const char *file, const char *func, int line, const char *msg, ...) {
    char str[BUFFER_SIZE];
    va_list argList;
    va_start(argList, msg);
    int length = vsnprintf(str, sizeof(str), msg, argList);
    va_end(argList);
    if (length > 0) {
        printf("([%s:%d] %s): %s\n", file, line, func, str);
    }
}