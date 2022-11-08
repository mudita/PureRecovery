// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "mount_points.h"
#include "log.h"

#include <hal/boot_control.h>
#include <hal/blk_dev.h>

#include <string.h>

static const char *system_mount_point = "/system";
static const char *user_mount_point = "/user";
static const char *boot_file = "/user/boot.json";
static const char *log_dir = "/system/log";
static const char *log_file = "/system/log/recovery.log";

int get_mount_points(vfs_mount_point_desc_t fstab[3]) {
    int err = boot_control_init(boot_file);
    if (err) {
        debug_log("Unable to init boot control module: %d", err);
        return err;
    }

    const char *slot = get_prefix(get_current_slot());

    fstab[0].disk = blkdev_emmc_user;
    fstab[0].partition = 1;
    fstab[0].type = vfs_fs_auto;
    fstab[0].mount_point = get_prefix(Slot_A);

    fstab[1].disk = blkdev_emmc_user;
    fstab[1].partition = 2;
    fstab[1].type = vfs_fs_auto;
    fstab[1].mount_point = get_prefix(Slot_B);

    fstab[2].disk = blkdev_emmc_user;
    fstab[2].partition = 3;
    fstab[2].type = vfs_fs_auto;
    fstab[2].mount_point = user_mount_point;

    if (strcmp(slot, get_prefix(Slot_A)) == 0) {
        fstab[0].mount_point = system_mount_point;
    } else {
        fstab[1].mount_point = system_mount_point;
    }

    return 0;
}

const char *get_log_filename() {
    return log_file;
}

const char *get_user_mount_point() {
    return user_mount_point;
}

const char *get_system_mount_point() {
    return system_mount_point;
}

const char *get_log_directory() {
    return log_dir;
}
