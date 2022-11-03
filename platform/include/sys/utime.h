// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct utimbuf
{
    time_t actime;
    time_t modtime;
};

int utime(const char *filename, const struct utimbuf *times);

#ifdef __cplusplus
};
#endif