#include "log.h"

#define BUFFER_SIZE 512

static FILE *log_file_handle = NULL;

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

void redirect_logs_to_file(const char *name) {
    if (log_file_handle != NULL) {
        fclose(log_file_handle);
    }
    log_file_handle = freopen(name, "a+", stdout);
}

void flush_logs() {
    if (log_file_handle != NULL) {
        fclose(log_file_handle);
    }
}
