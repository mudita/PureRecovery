#pragma once

enum convert_fs_state_e {
    CONVERSION_SUCCESS = 0,
    CONVERSION_NOT_REQUIRED,
    CONVERSION_FAILED
};

enum convert_fs_state_e repartition_fs(void);
