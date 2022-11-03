#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/stat.h>
#include <stdbool.h>

/// remove duplicated slashes from path
void path_remove_dup_slash(char *from);

/// remove ./
void path_remove_cwd(char *from);

/// remove dup from path and entry /
char *path_sanitize(char *from, char *path);

void path_remove_trailing_slash(char *out);

const char *path_basename_const(const char *path);

bool path_check_if_exists(const char *path);

#ifdef __cplusplus
}
#endif
