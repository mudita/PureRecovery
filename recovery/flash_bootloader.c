#include "flash_bootloader.h"
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <hal/blk_dev.h>
#include <log.h>

//! Ecooboot partition handle
#define ECO_PARTITION blk_disk_handle(blkdev_emmc_boot1, 0)
// Number of sectors per transfer
#define SECTORS_PER_TRANSFER 32

//! For cleanup dynamic resources
static void free_clean_up(uint8_t **ptr) {
    free(*ptr);
}

//! Cleanup file descriptor
static void file_clean_up(FILE **fil) {
    if (*fil) {
        fclose(*fil);
    }
}

/** Open ecoboot file in read only mode
 * and verify ecoboot header
 * @param[in] path Path to the ecoboot image
 * @param[out] file STDIO file handle
 * @return if positive image size otherwise error
 */
static int open_ecoboot_file(const char *path, FILE **file) {
    const off_t minimal_size = 64 * 1024;
    struct stat st;
    if (!file) {
        debug_log("Ecoboot update: filename not provided");
        return -EINVAL;
    }
    if (stat(path, &st)) {
        debug_log("Ecoboot update: failed to stat file: %s, errno: %d", path, errno);
        return -errno;
    }
    if (!S_ISREG(st.st_mode)) {
        debug_log("Ecoboot update: %s is not a regular file", path);
        return -ENOEXEC;
    }
    if (st.st_size < minimal_size) {
        debug_log("Ecoboot update: %s is too small", path);
        return -ENOEXEC;
    }
    *file = fopen(path, "r");
    if (!*file) {
        debug_log("Ecoboot update: failed to open file %s", path);
        return -errno;
    }
    return st.st_size;
}

/** Flash the ecoboot device
 * @param[in] path Ecoboot path
 * @return error code
 */
static int flash_ecoboot(const char *path) {
    blk_dev_info_t blk;
    int err, fil_size;
    FILE *fil __attribute__((__cleanup__(file_clean_up))) = NULL;
    if ((err = blk_info(ECO_PARTITION, &blk))) {
        debug_log("Ecoboot update: %s", err == -ENXIO ? "can't find partition" : "parameter is invalid");
        return err;
    }
    uint8_t *sect_buf __attribute__((__cleanup__(free_clean_up))) = malloc(blk.sector_size * SECTORS_PER_TRANSFER);
    if (!sect_buf) {
        debug_log("Ecoboot update: failed to allocate memory");
        return -errno;
    }

    if ((fil_size = open_ecoboot_file(path, &fil)) <= 0) {
        return fil_size;
    }
    for (lba_t lba = 0, sz = fil_size; !feof(fil) && !ferror(fil);) {
        if (sz < blk.sector_size * SECTORS_PER_TRANSFER) {
            memset(sect_buf, 0, blk.sector_size);
            if (!fread(sect_buf, sz, 1, fil)) {
                break;
            }
            sz = 0;
        } else {
            const size_t nread = fread(sect_buf, blk.sector_size, SECTORS_PER_TRANSFER, fil);
            if (nread == 0) {
                if (!ferror(fil)) {
                    sz -= blk.sector_size * nread;
                    continue;
                } else {
                    break;
                }
            }
            sz -= blk.sector_size * nread;
        }
        if ((err = blk_write(ECO_PARTITION, lba, SECTORS_PER_TRANSFER, sect_buf))) {
            debug_log("Ecoboot update: failed to write block of data: %d", err);
            return err;
        }
        lba += SECTORS_PER_TRANSFER;
    }
    if (ferror(fil)) {
        debug_log("Ecoboot update: file error: %d", errno);
        return -errno;
    } else {
        return 0;
    }
}

/** Verify the ecoboot
 * @param[in] path Ecoboot path
 * @return error code
 */
static int verify_ecoboot(const char *path) {
    blk_dev_info_t blk;
    int err, fil_size;
    FILE *fil __attribute__((__cleanup__(file_clean_up))) = NULL;
    if ((err = blk_info(ECO_PARTITION, &blk))) {
        debug_log("Ecoboot update: %s", err == -ENXIO ? "can't find partition" : "parameter is invalid");
        return err;
    }
    uint8_t *buf1 __attribute__((__cleanup__(free_clean_up))) = malloc(blk.sector_size * SECTORS_PER_TRANSFER);
    if (!buf1) {
        debug_log("Ecoboot update: failed to allocate memory");
        return -errno;
    }
    uint8_t *buf2 __attribute__((__cleanup__(free_clean_up))) = malloc(blk.sector_size * SECTORS_PER_TRANSFER);
    if (!buf2) {
        debug_log("Ecoboot update: failed to allocate memory");
        return -errno;
    }
    if ((fil_size = open_ecoboot_file(path, &fil)) <= 0) {
        return fil_size;
    }
    for (lba_t lba = 0, sz = fil_size; !feof(fil) && !ferror(fil);) {
        if ((err = blk_read(ECO_PARTITION, lba, SECTORS_PER_TRANSFER, buf1))) {
            debug_log("Ecoboot update: failed to read block of data: %d", err);
            return err;
        }
        lba += SECTORS_PER_TRANSFER;

        if (sz < blk.sector_size * SECTORS_PER_TRANSFER) {
            memset(buf2, 0, blk.sector_size);
            if (!fread(buf2, sz, 1, fil)) {
                break;
            }
            sz = 0;
        } else {
            const size_t nread = fread(buf2, blk.sector_size, SECTORS_PER_TRANSFER, fil);
            if (nread == 0) {
                if (!ferror(fil)) {
                    sz -= blk.sector_size * nread;
                    continue;
                } else {
                    break;
                }
            }
            sz -= blk.sector_size * nread;
        }
        if (memcmp(buf1, buf2, blk.sector_size * SECTORS_PER_TRANSFER) != 0) {
            debug_log("Ecoboot update: files mismatch!");
            return error_eco_verify;
        }
    }
    if (ferror(fil)) {
        debug_log("Ecoboot update: file error: %d", errno);
        return -errno;
    } else {
        return 0;
    }
}

int flash_bootloader(const char *path) {
    int ret;
    do {
        // Program flash
        ret = flash_ecoboot(path);
        if (ret) {
            break;
        }
        // Verify flaash
        ret = verify_ecoboot(path);
        if (ret) {
            break;
        }
        // Remove file
        ret = unlink(path);
        if (ret) {
            ret = error_eco_vfs;
            break;
        }
    } while (0);
    return ret;
}
