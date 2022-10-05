// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "database.h"

#include "sqlite3_vfs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const size_t queryBufferSize = 1024 * 4;

int sqlite3_os_init(void) {
    sqlite3_vfs_register(sqlite3_vfs_wrap(), 1);
    return SQLITE_OK;
}

int sqlite3_os_end(void) {
    return SQLITE_OK;
}

void database_initialize() {
    sqlite3_initialize();
}

void database_deinitialize() {
    sqlite3_shutdown();
}

database_t *database_open(const char *filename) {
    database_t *instance = malloc(sizeof(database_t));
    if (instance == NULL) {
        goto err;
    }
    instance->queryBuffer = malloc(queryBufferSize);
    if (instance->queryBuffer == NULL) {
        goto err;
    }
    if (sqlite3_open(filename, &instance->db)) {
        printf("Opening database: %s failed, err: %d, errmg: %s\n",
               filename,
               sqlite3_errcode(instance->db),
               sqlite3_errmsg(instance->db));

        goto err;
    }

    return instance;

    err:
    if (instance->queryBuffer != NULL) {
        free(instance->queryBuffer);
    }
    if (instance != NULL) {
        free(instance);
    }
    return NULL;
}

void database_close(database_t *const instance) {
    if (instance->db != NULL) {
        sqlite3_close(instance->db);
    }
    if (instance->queryBuffer != NULL) {
        free(instance->queryBuffer);
    }
    if (instance != NULL) { free(instance); }
}

database_ret_t database_execute(database_t *const instance, const char *format, ...) {
    if (instance == NULL) {
        return Db_InvalidHandle;
    }

    memset(instance->queryBuffer, 0, queryBufferSize);
    va_list ap;
    va_start(ap, format);
    sqlite3_vsnprintf(queryBufferSize, instance->queryBuffer, format, ap);
    va_end(ap);

    const int result = sqlite3_exec(instance->db, instance->queryBuffer, NULL, NULL, NULL);
    if (result) {
        printf("Execution failed with %d, errcode: %d, errmsg: %s\n",
               result,
               sqlite3_errcode(instance->db),
               sqlite3_errmsg(instance->db));
        return Db_Error;
    }
    return Db_Ok;
}
