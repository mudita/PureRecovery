#include "mbr.h"
#include "hal/blk_dev.h"
#include "prv/blkdev/blk_dev.h"
#include <errno.h>
#include <stdlib.h>

static void autofree(uint8_t **ptr)
{
    free(*ptr);
}

#define AUTOFREE(var) uint8_t *var __attribute__((__cleanup__(autofree)))

bool mbr_signature_valid(const uint8_t *mbr_sect) {
    return ((mbr_sect[MBR_SIGNATURE_OFS] == 0x55) && (mbr_sect[MBR_SIGNATURE_OFS + 1] == 0xAA));
}

int mbr_set_partition_type(int device, uint8_t type) {
    int ret;
    int disk = blk_hwdisk(device);
    int part = blk_hwpart(device);

    do
    {
        blk_dev_info_t diskinfo;
        ret = blk_info(disk, &diskinfo);
        if (ret < 0) {
            break;
        }
        AUTOFREE(mbr_sect) = malloc(diskinfo.sector_size);
        if (mbr_sect == NULL) {
            ret = -ENOMEM;
            break;
        }
        ret = blk_read(disk, 0, 1, mbr_sect);
        if (ret < 0) {
            break;
        }
        if (!mbr_signature_valid(mbr_sect)) {
            ret = -ENXIO;
            break;
        }

        size_t part_type_ofs = MBR_PART_TAB_OFS + PART_TAB_SIZE * (part - 1) + PART_TAB_TYPE_OFS;
        mbr_sect[part_type_ofs] = type;

        ret = blk_write(disk, 0, 1, mbr_sect);
        if (ret < 0) {
            break;
        }

    } while (0);

    return ret;
}

int mbr_set_partition_boot_flag(int device, bool bootable) {
    int ret;
    int disk = blk_hwdisk(device);
    int part = blk_hwpart(device);

    do
    {
        blk_dev_info_t diskinfo;
        ret = blk_info(disk, &diskinfo);
        if (ret < 0) {
            break;
        }
        AUTOFREE(mbr_sect) = malloc(diskinfo.sector_size);
        if (mbr_sect == NULL) {
            ret = -ENOMEM;
            break;
        }
        ret = blk_read(disk, 0, 1, mbr_sect);
        if (ret < 0) {
            break;
        }
        if (!mbr_signature_valid(mbr_sect)) {
            ret = -ENXIO;
            break;
        }

        size_t part_type_ofs = MBR_PART_TAB_OFS + PART_TAB_SIZE * (part - 1) + PART_TAB_ACTIVE_OFS;
        if (bootable) {
            mbr_sect[part_type_ofs] |= PART_ACTIVE_MASK;
        }
        else {
            mbr_sect[part_type_ofs] &= ~PART_ACTIVE_MASK;
        }

        ret = blk_write(disk, 0, 1, mbr_sect);
        if (ret < 0) {
            break;
        }

    } while (0);

    return ret;
}
