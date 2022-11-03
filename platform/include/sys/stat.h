// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include_next <sys/stat.h>

int lstat(const char *__restrict __path, struct stat *__restrict __buf);
