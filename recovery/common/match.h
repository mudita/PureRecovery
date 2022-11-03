#pragma once

#include <stdbool.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// match the end of stirng with string
bool string_match_end(const char *in, const char *to_match);

bool string_match_partial(const char *in, const char *to_match);

bool string_match_any_of(const char *str, const char **file_table, size_t cnt);

bool string_match_any_of_partial(const char *str, const char **file_table, size_t cnt);

#ifdef __cplusplus
extern "C"
}
#endif


