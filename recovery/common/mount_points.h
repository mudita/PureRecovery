// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <hal/tinyvfs.h>

int get_mount_points(vfs_mount_point_desc_t fstab[3]);

const char* get_log_filename();
const char* get_log_directory();
const char* get_user_mount_point();
const char* get_system_mount_point();

