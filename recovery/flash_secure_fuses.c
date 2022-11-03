// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "flash_secure_fuses.h"

#include "security/pgmkeys.h"

#include <common/log.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

void flash_secure_fuses(const char *srk_path, const char *chksum_file) {
    struct program_keys_handle khandle;
    khandle.srk_file = malloc(strlen(srk_path));
    khandle.chksum_srk_file = malloc(strlen(chksum_file));
    strcpy(khandle.srk_file, srk_path);
    strcpy(khandle.chksum_srk_file, chksum_file);
    if (program_keys_is_needed(&khandle)) {
        debug_log("Update: keys programming is required. Key file: %s checksum: %s", khandle.srk_file,
                  khandle.chksum_srk_file);
        if (program_keys(&khandle)) {
            debug_log("Update: failed to program keys");
        }
        unlink(khandle.srk_file);
        unlink(khandle.chksum_srk_file);
        free(khandle.srk_file);
        free(khandle.chksum_srk_file);
    } else {
        debug_log("Update: programming the keys is not needed");
    }
}
