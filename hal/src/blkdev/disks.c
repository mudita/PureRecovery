#include <hal/blk_dev.h>
#include <prv/blkdev/blk_dev.h>
#include <prv/blkdev/partscan.h>
#include <hal/emmc.h>
#include <fsl_mmc.h>
#include <fsl_sdmmc_host.h>
#include <errno.h>
#include <stdio.h>

#define BLKDEV_FIRST_PART 1 //! First logical partition

struct blk_disk
{
    struct blk_partition *parts; //! Partitions lists
    size_t n_parts;              //! Number of partitions
    void *hwdrv;                 //! Hardware driver pointer
    size_t sect_size;            //! Single sector size
    size_t erase_group_blks;     //! Erase group blocks
};

//! Partitions on disc table
struct blkdev_context
{
    // Disk and partitions
    struct blk_disk disks[_blkdev_eot_];
};
// Context of the disk subsystem
static struct blkdev_context ctx;

// Calculate part lba to disc LBA
static int part_lba_to_disc_lba(int idisk, int ipart, lba_t *lba, blk_size_t count)
{
    const blk_partition_t *part = &ctx.disks[idisk].parts[ipart];
    if (*lba + count > part->num_sectors)
    {
        return -ERANGE;
    }
    else
    {
        *lba = *lba + part->start_sector;
    }
    return 0;
}

/* Hardware disc initialize 
   first partition is a disc configuation
*/
static void *disk_hardware_init(int idisk, blk_partition_t *part, size_t *sect_size, size_t *erase_grp)
{
    mmc_card_t *card = emmc_card();
    if (!card)
    {
        return card;
    }
    part->type = blk_part_type_hardware;
    part->logical_num = 0;
    part->start_sector = 0;
    if (idisk == blkdev_emmc_user)
    {
        part->num_sectors = card->userPartitionBlocks;
    }
    else if (idisk == blkdev_emmc_boot1)
    {
        part->num_sectors = card->bootPartitionBlocks;
    }
    *sect_size = card->blockSize;
    *erase_grp = card->eraseGroupBlocks;
    return card;
}

//! Return disk device
int blk_disk_handle(uint16_t hwdisk, uint8_t partition)
{
    return ((int)hwdisk << BLKDEV_DISK_SHIFT) | partition;
}

// Initialize the block device
int blk_initialize(void)
{
    //Currently we are supporting only single emmc disk
    for (int i = 0; i < _blkdev_eot_; ++i)
    {
        struct blk_disk *disk = &ctx.disks[i];
        disk->n_parts = 0;
        disk->parts = calloc(1, sizeof(blk_partition_t));
        disk->hwdrv = disk_hardware_init(i, disk->parts, &disk->sect_size, &disk->erase_group_blks);
        if (!disk->hwdrv)
        {
            return -EIO;
        }
        if (i == blkdev_emmc_user)
        {
            //! Part scan for user partitions
            int nparts = blk_priv_scan_partitions(blk_disk_handle(i, 0), &disk->parts);
            if (nparts < 0)
            {
                return nparts;
            }
            disk->n_parts = nparts;
        }
    }
    return 0;
}

int blk_get_partitions(int device, blk_partition_t **parts)
{
    const int idisk = blk_hwdisk(device);
    if (idisk >= _blkdev_eot_)
    {
        return -ENXIO;
    }
    if (parts)
    {
        *parts = ctx.disks[idisk].parts;
    }
    return ctx.disks[idisk].n_parts;
}

int blk_get_partition(int device, blk_partition_t *part)
{
    const int ipart = blk_hwpart(device);
    blk_partition_t *parts;
    const int nparts = blk_get_partitions(device, &parts);
    if (ipart > nparts)
    {
        return -ENOENT;
    }
    *part = parts[ipart];
    return 0;
}

int blk_read(int device, lba_t lba, blk_size_t lba_count, void *buf)
{
    int ret;
    int idisk = blk_hwdisk(device);
    if (idisk >= _blkdev_eot_)
    {
        return -ENXIO;
    }
    const struct blk_disk *disk = &ctx.disks[idisk];
    size_t ipart = blk_hwpart(device);
    if (ipart > disk->n_parts)
    {
        return -ENXIO;
    }

    if (idisk == blkdev_emmc_boot1)
    {
        ret = MMC_SelectPartition((mmc_card_t *)disk->hwdrv, kMMC_AccessPartitionBoot1);
        if (ret != kStatus_Success)
        {
            return -EIO;
        }
    }
    ret = part_lba_to_disc_lba(idisk, ipart, &lba, lba_count);
    if (ret)
    {
        return ret;
    }
    ret = MMC_ReadBlocks((mmc_card_t *)disk->hwdrv, (uint8_t *)buf, lba, lba_count);
    if (ret != kStatus_Success)
    {
        return -EIO;
    }

    if (idisk == blkdev_emmc_boot1)
    {
        ret = MMC_SelectPartition((mmc_card_t *)disk->hwdrv, kMMC_AccessPartitionUserAera);
        if (ret != kStatus_Success)
        {
            return -EIO;
        }
    }
    return ret;
}

int blk_write(int device, lba_t lba, blk_size_t lba_count, const void *buf)
{
    int ret;
    int idisk = blk_hwdisk(device);
    if (idisk >= _blkdev_eot_)
    {
        return -ENXIO;
    }
    const struct blk_disk *disk = &ctx.disks[idisk];
    size_t ipart = blk_hwpart(device);
    if (ipart > disk->n_parts)
    {
        return -ENXIO;
    }

    if (idisk == blkdev_emmc_boot1)
    {
        ret = MMC_SelectPartition((mmc_card_t *)disk->hwdrv, kMMC_AccessPartitionBoot1);
        if (ret != kStatus_Success)
        {
            return -EIO;
        }
    }

    ret = part_lba_to_disc_lba(idisk, ipart, &lba, lba_count);
    if (ret)
    {
        return ret;
    }

    ret = MMC_WriteBlocks((mmc_card_t *)disk->hwdrv, (const uint8_t *)buf, lba, lba_count);
    if (ret != kStatus_Success)
    {
        return -EIO;
    }

    if (idisk == blkdev_emmc_boot1)
    {
        ret = MMC_SelectPartition((mmc_card_t *)disk->hwdrv, kMMC_AccessPartitionUserAera);
        if (ret != kStatus_Success)
        {
            return -EIO;
        }
    }
    return ret;
}

int blk_info(int device, blk_dev_info_t *info)
{
    if (!info)
    {
        return -EINVAL;
    }
    int idisk = blk_hwdisk(device);
    if (idisk >= _blkdev_eot_)
    {
        return -ENXIO;
    }
    size_t ipart = blk_hwpart(device);
    if (ipart > ctx.disks->n_parts)
    {
        return -ENXIO;
    }
    info->sector_size = ctx.disks[idisk].sect_size;
    info->erase_group = ctx.disks[idisk].erase_group_blks;
    info->sector_count = ctx.disks[idisk].parts[ipart].num_sectors;
    return 0;
}