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

bool recursive_unlink(const char *what);

bool recursive_cp(const char *what, const char *where);

#ifdef __cplusplus
}
#endif
