#include "convert_fs.h"

#include <common/path_opts.h>
#include <ext4_blockdev.h>
#include <ext4_mkfs.h>
#include <ext4_super.h>
#include <log.h>
#include <hal/tinyvfs.h>
#include <hal/blk_dev.h>
#include <hal/boot_control.h>
#include <hal/delay.h>
#include <prv/tinyvfs/ext4_diskio.h>
#include <prv/tinyvfs/vfs_priv_data.h>
#include <mbr.h>
#include <mount_points.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

struct change_ext4_label_config_s {
    const char *mount_point;
    const char *new_label;
};

struct convert_fs_config_s {
    const char *mount_point;
    const struct vfs_mount_point_desc *fstab;
    size_t fstab_size;
    const char *log_tmp_dir;
    const char *uuid;
    const char *label;
};

static int erase_fat_boot_record(int device) {
    struct blk_dev_info disk_info;
    int err = blk_info(device, &disk_info);
    if (err < 0) {
        return err;
    }

    uint8_t *zeros = calloc(1, disk_info.sector_size);
    if (zeros == NULL) {
        return -ENOMEM;
    }
    err = blk_write(device, 0, 1, zeros);

    free(zeros);
    return err;
}

static int create_ext4_fs(struct ext4_blockdev *blkdev, const char *label, const uint8_t *uuid) {
    struct ext4_mkfs_info mkfs_info;
    struct ext4_fs fs;

    memset(&mkfs_info, 0, sizeof(struct ext4_mkfs_info));
    memset(&fs, 0, sizeof(struct ext4_fs));
    mkfs_info.len = blkdev->part_size;
    mkfs_info.journal = true;
    strncpy(mkfs_info.label, label, sizeof(mkfs_info.label));
    memcpy(mkfs_info.uuid, uuid, UUID_SIZE);

    return ext4_mkfs(&fs, blkdev, &mkfs_info, F_SET_EXT4);
}

static void log_uuid(const uint8_t *uuid) {
    char buffer[2 * UUID_SIZE + 4 + 2];
    char *buffer_ptr = &buffer[0];
    for (size_t i = 0; i < UUID_SIZE; i++) {
        buffer_ptr += sprintf(buffer_ptr, "%02x", uuid[i]);
        if ((i >= 3) && (i <= 9) && (i & 1)) {
            buffer_ptr += sprintf(buffer_ptr, "-");
        }
    }
    debug_log("UUID: %s", buffer);
}

static void log_partition_info(const struct ext4_mkfs_info *info) {
    debug_log("size: %lu",
              (unsigned long) info->len); // this is bad, but %llu doesn't work in updater's printf implementation
    debug_log("block size: %lu", info->block_size);
    debug_log("blocks per group: %lu", info->blocks_per_group);
    debug_log("inodes per group: %lu", info->inodes_per_group);
    debug_log("inode size: %lu", info->inode_size);
    debug_log("inodes: %lu", info->inodes);
    debug_log("journal blocks: %lu", info->journal_blocks);
    debug_log("features ro_compat: 0x%lX", info->feat_ro_compat);
    debug_log("features compat: 0x%lX", info->feat_compat);
    debug_log("features incompat: 0x%lX", info->feat_incompat);
    debug_log("BG desc reserve: %lu", info->bg_desc_reserve_blocks);
    debug_log("descriptor size: %u", info->dsc_size);
    log_uuid(info->uuid);
    debug_log("journal: %s", info->journal ? "yes" : "no");
    debug_log("label: %s", info->label);
}

static bool tmp_location_valid(const char *src_path, const char *tmp_path) {
    const size_t mp_len = strlen(src_path);
    if ((strncmp(src_path, tmp_path, mp_len) == 0) && ((tmp_path[mp_len] == '\0') || (tmp_path[mp_len] == '/'))) {
        return false;
    }
    return true;
}

static enum convert_fs_state_e convert_fs(const struct convert_fs_config_s *config) {
    int err;
    struct vfs_mount *mp;

    do {
        /* Get mount point */
        err = vfs_get_mnt_point(&mp, config->mount_point, NULL);
        if (err) {
            debug_log("Failed to get VFS mount point, error %d", err);
            err = CONVERSION_FAILED;
            break;
        }
        const int device = mp->storage_dev;
        const uint8_t fs_type = mp->type;

        /* Check if conversion required */
        if (fs_type == vfs_fs_ext4) {
            debug_log("Filesystem is ext4, conversion not required");
            err = CONVERSION_NOT_REQUIRED;
            break;
        }
        if (fs_type != vfs_fs_fat) {
            debug_log("Unsupported or corrupted filesystem %d", fs_type);
            err = CONVERSION_FAILED;
            break;
        }

        debug_log("Detected FAT OS partition, starting conversion");

        /* Flush logs and close log file before unmounting */
        flush_logs();

        /* Create a backup of current log directory, as it will be deleted */
        const char *const log_dir_path = get_log_directory();
        if (!tmp_location_valid(log_dir_path, config->log_tmp_dir)) {
            debug_log("Temporary directory cannot be placed on the partition being converted");
            err = CONVERSION_FAILED;
            break;
        }

        if (!recursive_cp(log_dir_path, config->log_tmp_dir)) {
            debug_log("Failed to create logs backup");
            err = CONVERSION_FAILED;
            break;
        }

        /* Unmount partitions */
        debug_log("Unmounting partitions");
        err = vfs_unmount_deinit();
        if (err) {
            debug_log("Failed to unmount partitions, error %d", err);
            err = CONVERSION_FAILED;
            break;
        }

        /* Erase FAT boot record */
        debug_log("Erasing FAT boot record");
        err = erase_fat_boot_record(device);
        if (err) {
            debug_log("Failed to erase FAT boot record, error %d", err);
            err = CONVERSION_FAILED;
            break;
        }

        /* Create ext4 block device */
        debug_log("Creating ext4 block device");
        struct ext4_blockdev *blockdev;
        err = vfs_ext4_append_volume(device, &blockdev);
        debug_log("(^^^ this warning is normal ^^^)");
        if (err) {
            debug_log("Failed to create block device, error: %d", err);
            err = CONVERSION_FAILED;
            break;
        }

        /* Format partition as ext4 */
        debug_log("Formatting partition as ext4");
        err = create_ext4_fs(blockdev, config->label, (const uint8_t *) config->uuid);
        if (err) {
            debug_log("Failed to format partition, error: %d", err);
            err = CONVERSION_FAILED;
            break;
        }

        /* Update partition info in MBR */
        debug_log("Updating partition type in MBR");
        err = mbr_set_partition_type(device, blk_part_type_ext4);
        if (err) {
            debug_log("MBR updating failed, error %d", err);
            err = CONVERSION_FAILED;
            break;
        }
        /* Get filesystem info */
        struct ext4_mkfs_info info;
        err = ext4_mkfs_read_info(blockdev, &info);
        if (err) {
            debug_log("Reading filesystem info failed, error %d", err);
            err = CONVERSION_FAILED;
            break;
        }

        /* Release ext4 block device */
        vfs_ext4_remove_volume(blockdev);

        /* Remount partitions */
        err = vfs_mount_init(config->fstab, config->fstab_size);
        if (err) {
            debug_log("Failed to remount partitions, error %d", err);
            err = CONVERSION_FAILED;
            break;
        }

        /* Restore the backup of the logs */
        debug_log("Restoring logs backup");
        if (!recursive_cp(config->log_tmp_dir, log_dir_path)) {
            debug_log("Failed to restore logs backup");
            err = CONVERSION_FAILED;
            break;
        }

        /* Remove the backup */
        debug_log("Removing logs backup");
        if (!recursive_unlink(config->log_tmp_dir)) {
            debug_log("Failed to remove logs backup");
            err = CONVERSION_FAILED;
            break;
        }

        /* Logging to file is available again */
        redirect_logs_to_file(get_log_filename());
        debug_log("Partitions remounted");
        debug_log("ext4 partition created successfully!");

        /* Log filesystem parameters */
        debug_log("Filesystem parameters:");
        log_partition_info(&info);

        err = CONVERSION_SUCCESS;
    } while (0);

    return err;
}

static bool change_partition_label(const struct change_ext4_label_config_s *config) {
    int err;
    struct vfs_mount *mp;
    bool success = false;

    do {
        /* Get mount point */
        err = vfs_get_mnt_point(&mp, config->mount_point, NULL);
        if (err) {
            debug_log("Failed to get VFS mount point, error %d", err);
            break;
        }
        const uint8_t fs_type = mp->type;

        /* Check if mount point is a valid ext4 partition */
        if (fs_type != vfs_fs_ext4) {
            debug_log("Mount point '%s' is not a valid ext4 partition!", config->mount_point);
            break;
        }

        /* Label change start */
        debug_log("Changing mount point '%s' label to '%s'", config->mount_point, config->new_label);
        struct ext4_blockdev *blockdev = (struct ext4_blockdev *) (mp->fs_data);

        /* Get block device cached superblock. Primary superblock is cached at mount time
         * and written back as primary superblock by ext4_fs_fini() in ext4_umount() */
        struct ext4_sblock *bdev_sblock = &blockdev->fs->sb;

        /* Change volume name */
        strncpy(bdev_sblock->volume_name, config->new_label, sizeof(bdev_sblock->volume_name));

        /* Compute and set checksum of the modified superblock */
        ext4_sb_set_csum(bdev_sblock);

        /* Read filesystem info */
        struct ext4_mkfs_info info;
        err = ext4_mkfs_read_info(blockdev, &info);
        if (err) {
            debug_log("Reading filesystem info failed, error %d", err);
            break;
        }

        /* Create filesystem auxiliary info */
        struct fs_aux_info aux_info;
        err = create_fs_aux_info(&aux_info, &info);
        if (err) {
            debug_log("Creating filesystem auxiliary info failed, error %d", err);
            break;
        }

        /* Copy modified superblock to aux_info superblock */
        memcpy(aux_info.sb, bdev_sblock, sizeof(struct ext4_sblock));

        /* Update all filesystem's superblocks */
        err = write_sblocks(blockdev, &aux_info, &info);
        if (err) {
            debug_log("Failed to update superblocks, error %d", err);
            break;
        }

        /* Release filesystem auxiliary info */
        release_fs_aux_info(&aux_info);

        debug_log("Label successfully changed to '%s'!", config->new_label);
        success = true;

    } while (0);

    return success;
}

static int check_temp_dir(const char *dir) {
    struct stat tstat = {};
    const int ret = stat(dir, &tstat);
    if (ret == 0) {
        return !recursive_unlink(dir);
    } else if (ret == -1 && errno == ENOENT) {
        return 0;
    }
    return ret;
}

static char *rand_string(char *str, size_t size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

static const char *generate_uuid() {
    static char buffer[UUID_SIZE + 1] = {0};

    srand(get_jiffiess());
    return rand_string(buffer, UUID_SIZE);
}


enum convert_fs_state_e repartition_fs(void) {
    const char *slot_a_label = "system_a";
    const char *slot_b_label = "system_b";
    vfs_mount_point_desc_t fstab[3] = {};
    int ret = get_mount_points(fstab);
    if (ret != 0) {
        debug_log("Failed to get mount points");
        return CONVERSION_FAILED;
    }

    const struct convert_fs_config_s cfsconfig = {
            .mount_point = get_system_mount_point(),
            .fstab = fstab,
            .fstab_size = sizeof fstab,
            .log_tmp_dir = "/user/tmp_backup",
            .uuid = generate_uuid(),
            .label = slot_a_label
    };

    ret = check_temp_dir(cfsconfig.log_tmp_dir);
    if (ret != 0) {
        debug_log("Failed to prepare temp directory");
        return CONVERSION_FAILED;
    }

    enum convert_fs_state_e err = convert_fs(&cfsconfig);
    if (err == CONVERSION_FAILED) {
        debug_log("Failed to convert MUDITAOS from FAT to ext4");
        return err;
    }

    const struct change_ext4_label_config_s clconfig = {
            .new_label = slot_b_label,
            .mount_point = get_prefix(Slot_B)
    };
    if (!change_partition_label(&clconfig)) {
        debug_log("Failed to change label of %s to %s", "/backup", clconfig.new_label);
        return CONVERSION_FAILED;
    }
    return err;
}
