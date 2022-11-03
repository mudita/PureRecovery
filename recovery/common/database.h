// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <sqlite3.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    sqlite3 *db;
    char *queryBuffer;
} database_t;

/// Return codes
typedef enum {
    Db_Ok,
    Db_Error,
    Db_InvalidHandle
} database_ret_t;

/// Initialize SQLite engine. Ought to be called only once before any database operation are executed
void database_initialize();

/// De-initialize SQLite engine. Ought be called only once after finished using SQLite
void database_deinitialize();

/// Open database, return NULL in case of failure.
database_t *database_open(const char *filename);

/// Close database, it is safe to call it multiple times.
void database_close(database_t *const instance);

/// Execute SQL
database_ret_t database_execute(database_t *const instance, const char *format, ...);

#ifdef __cplusplus
}
#endif




