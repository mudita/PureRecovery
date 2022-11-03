// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <stdint.h>
#include "fsl_snvs_lp.h"

uint32_t boot_reason_get_raw() {
    return SNVS->LPGPR[0];
}

void boot_reason_set_raw(uint32_t raw) {
    SNVS->LPGPR[0] = raw;
}