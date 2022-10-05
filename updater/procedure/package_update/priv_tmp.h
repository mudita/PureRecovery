#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#include "update.h"
#include "common/log.h"

/// create temporary catalog for update - removes old catalog if exists in that place
bool tmp_create_catalog(struct update_handle_s *handle);
/// Move files:
/// 1. recursive: from {catalog}/{handle_os} move to {handle os}
/// 2. recursive: from {catalog}/{handle_user} move to {handle user}
bool tmp_files_move(struct update_handle_s *handle);

bool user_files_move_test(const char* const from, const char* const to);

bool recursive_unlink(const char *what, bool factory_reset);

#ifdef __cplusplus
}
#endif
