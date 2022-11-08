// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <hal/tinyvfs.h>
#include <prv/tinyvfs/vfs_priv_data.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/dirent.h>
#include <prv/tinyvfs/vfs_device.h>
#include <prv/tinyvfs/ext4_diskio.h>
#include <prv/tinyvfs/vfs_ext4.h>
#include <ext4.h>
#include <ext4_inode.h>
#include <ext4_super.h>


static const char* normalize_path(const char * path, const struct vfs_mount* mountp)
{
    char *mnt_path = malloc(strlen(path)+2);
    strcpy(mnt_path, path);
    if(strcmp(path, mountp->mnt_point) == 0)
    {
        strcat(mnt_path, "/");
    }
    return mnt_path;
}

static const char* normalize_mount_point(const struct vfs_mount* mountp)
{
    char *mnt_path = malloc(strlen(mountp->mnt_point)+2);
    strcpy(mnt_path, mountp->mnt_point);
    strcat(mnt_path, "/");
    return mnt_path;
}

static void _free_clean_up_str(const char **str)
{
    free((void *)*str);
    *str = NULL;
}

static void _free_clean_up_buf(char **str)
{
    free((void *)*str);
    *str = NULL;
}

#define AUTO_PATH(var) const char *var __attribute__((__cleanup__(_free_clean_up_str)))
#define AUTO_BUF(var) char *var __attribute__((__cleanup__(_free_clean_up_buf)))

// Ext filesystem mount
static int ext_mount(struct vfs_mount *mountp)
{
    struct ext4_blockdev *blkdev;
    int err = vfs_ext4_append_volume(mountp->storage_dev, &blkdev);
    if (err)
    {
        return err;
    }
    char devname[CONFIG_EXT4_MAX_BLOCKDEV_NAME];
    snprintf(devname, sizeof devname, "emmc%i", mountp->storage_dev);
    err = ext4_device_register(blkdev, devname);
    if (err)
    {
        vfs_ext4_remove_volume(blkdev);
        return -err;
    }
    AUTO_PATH(mnt_path) = normalize_mount_point(mountp);

    err = ext4_mount(devname, mnt_path, false);
    if (err)
    {
        ext4_device_unregister(devname);
        vfs_ext4_remove_volume(blkdev);
        return -err;
    }
    err = ext4_recover(mnt_path);
    if (err)
    {
        ext4_umount(mnt_path);
        ext4_device_unregister(devname);
        vfs_ext4_remove_volume(blkdev);
        return -err;
    }
    err = ext4_journal_start(mnt_path);
    if (err)
    {
        printf("Warning: Unable to start ext4 journal error: %i", err);
    }
    err = ext4_block_cache_write_back(blkdev, true);
    if (err)
    {
        printf("Warning: Unable to enable writeback on ext4 error: %i", err);
    }
    mountp->fs_data = blkdev;
    return -err;
}

// Umount filesystem
static int ext_unmount(struct vfs_mount *mountp)
{
    struct ext4_blockdev *blkdev = mountp->fs_data;
    if (!blkdev)
    {
        return -EIO;
    }
    int err = ext4_block_cache_write_back(blkdev, false);
    if (err)
    {
        printf("Warning: Unable to disable writeback on ext4 error: %i", err);
    }
    AUTO_PATH(mnt_path) = normalize_mount_point(mountp);
    err = ext4_journal_stop(mnt_path);
    if (err)
    {
        printf("Warning: Unable to stop ext4 journal error: %i", err);
    }
    err = ext4_umount(mnt_path);
    if (err)
    {
        return -err;
    }
    char devname[CONFIG_EXT4_MAX_BLOCKDEV_NAME];
    snprintf(devname, sizeof devname, "emmc%i", mountp->storage_dev);
    ext4_device_unregister(devname);
    vfs_ext4_remove_volume(blkdev);
    return -err;
}

static int ext_open(struct vfs_file *fp, const char *path, int flags, int mode)
{
    VFS_UNUSED(mode);
    fp->filep = calloc(1, sizeof(struct ext4_file));
    if (!fp->filep)
    {
        return -ENOMEM;
    }
    int ret = ext4_fopen2(fp->filep, path, flags);
    if (ret)
    {
        free(fp->filep);
        fp->filep = NULL;
    }
    return -ret;
}

static int ext_close(struct vfs_file *fp)
{
    int ret = ext4_fclose(fp->filep);
    free(fp->filep);
    fp->filep = NULL;
    return -ret;
}

static ssize_t ext_read(struct vfs_file *fp, void *ptr, size_t len)
{
    size_t rcnt=0;
    ssize_t ret = ext4_fread(fp->filep, ptr, len, &rcnt);
    return (ret) ? (-ret) : ((ssize_t)rcnt);
}

static ssize_t ext_write(struct vfs_file *fp, const void *ptr, size_t len)
{
    size_t wcnt=0;
    ssize_t ret = ext4_fwrite(fp->filep, ptr, len, &wcnt);
    return (ret) ? (-ret) : ((ssize_t)wcnt);
}

static ssize_t ext_lseek(struct vfs_file *fp, off_t off, int whence)
{
    int ret = ext4_fseek(fp->filep, off, whence);
    if (ret)
    {
        return -ret;
    }
    return ext4_ftell(fp->filep);
}

static off_t ext_tell(struct vfs_file *fp)
{
    return ext4_ftell(fp->filep);
}

static int ext_truncate(struct vfs_file *fp, off_t length)
{
    int err = ext4_ftruncate(fp->filep, length);
    if(err==ENOTSUP)
    {
        //NOTE: Ext4 ftruncate supports only shrinking
        const size_t zbuf_len = 8192;
        AUTO_BUF(zero_buf) = calloc(1,zbuf_len);
        err = 0;
        for(size_t n=0; n<length/zbuf_len; ++n)
        {
            err = ext_write(fp, zero_buf, zbuf_len);
            if(err!=zbuf_len) {
                err = (err>0)?(-ENOSPC):(err);
                break;
            }
        }
        err = zbuf_len?0:err;
        if(!err)
        {
            const ssize_t remain = length%zbuf_len;
            if(remain>0)
            {
                err = ext_write(fp, zero_buf, remain);
                err = (err!=remain)?((err>0)?(-ENOSPC):(err)):(0);
            }
        }
    }
    return err;
}

static int ext_sync(struct vfs_file *fp)
{
    AUTO_PATH(mnt_path) = normalize_mount_point(fp->mp);
    return -ext4_cache_flush(mnt_path);
}

static int ext_opendir(struct vfs_dir *dp, const char *path)
{
    dp->dirp = calloc(1, sizeof(struct ext4_dir));
    if (!dp->dirp)
    {
        return -ENOMEM;
    }
    const char* mod_path;
    bool path_allocated;
    // Fix for buggy opendir on root path in lwext4
    if(!strcmp(path, dp->mp->mnt_point))
    {
        char* lpath = malloc(dp->mp->mountp_len+1);
        strcpy(lpath, path);
        strcat(lpath, "/");
        mod_path = lpath;
        path_allocated = true;
    }
    else
    {
        mod_path = path;
        path_allocated = false;
    }
    int err = ext4_dir_open(dp->dirp, mod_path);
    if(path_allocated)
    {
        free((void*)mod_path);
    }
    if (err)
    {
        free(dp->dirp);
        dp->dirp = NULL;
        return -err;
    }
    return -err;
}

static inline int ino_to_type(int dtype)
{
    switch (dtype)
    {
        case EXT4_DE_REG_FILE:
            return DT_REG;
        case EXT4_DE_DIR:
            return DT_DIR;
        case EXT4_DE_CHRDEV:
            return DT_CHR;
        case EXT4_DE_BLKDEV:
            return DT_BLK;
        case EXT4_DE_FIFO:
            return DT_FIFO;
        case EXT4_DE_SOCK:
            return DT_SOCK;
        default:
            return 0;
    }
}

static int ext_readdir(struct vfs_dir *dp, struct dirent *entry)
{
    if (!entry)
    {
        return -EINVAL;
    }
    const ext4_direntry *dentry = ext4_dir_entry_next(dp->dirp);
    if (dentry)
    {
        strncpy(entry->d_name, (const char *)dentry->name, sizeof(entry->d_name));
        entry->d_type = ino_to_type(dentry->inode_type);
    }
    else
    {
        entry->d_name[0] = '\0';
    }
    return 0;
}

static int ext_closedir(struct vfs_dir *dp)
{
    int err = ext4_dir_close(dp->dirp);
    free(dp->dirp);
    dp->dirp = NULL;
    return -err;
}

static int ext_unlink(struct vfs_mount *mountp, const char *path)
{
    VFS_UNUSED(mountp);
    if(ext4_inode_exist(path, EXT4_DE_DIR)==0) {
        return -ext4_dir_rm(path);
    } else {
        return -ext4_fremove(path);
    }
}

static int ext_rmdir(struct vfs_mount *mountp, const char *path)
{
    VFS_UNUSED(mountp);
    return -ext4_dir_rm(path);
}

static int ext_rename(struct vfs_mount *mountp, const char *from, const char *to)
{
    VFS_UNUSED(mountp);
    return -ext4_frename(from, to);
}

static int ext_mkdir(struct vfs_mount *mountp, const char *path)
{
    VFS_UNUSED(mountp);
    return -ext4_dir_mk(path);
}
inline int ino_to_st_mode(int dtype)
{
    switch (dtype)
    {
        case EXT4_DE_REG_FILE:
            return S_IFREG;
        case EXT4_DE_DIR:
            return S_IFDIR;
        case EXT4_DE_CHRDEV:
            return S_IFCHR;
        case EXT4_DE_BLKDEV:
            return S_IFBLK;
        case EXT4_DE_FIFO:
            return S_IFIFO;
        case EXT4_DE_SOCK:
            return S_IFSOCK;
        default:
            return 0;
    }
}

static int ext_stat(struct vfs_mount *mountp, const char *path, struct stat *entry)
{
    uint32_t inonum;
    struct ext4_inode ino;
    struct ext4_sblock *sb;
    AUTO_PATH(normalized_path) = normalize_path(path, mountp);
    int err = ext4_raw_inode_fill(normalized_path, &inonum, &ino);
    if (err)
    {
        return -err;
    }
    AUTO_PATH(mnt_path) = normalize_mount_point(mountp);
    err = ext4_get_sblock(mnt_path, &sb);
    if (err)
    {
        return -err;
    }
    memset(entry, 0, sizeof(*entry));
    entry->st_ino = inonum;
    const uint32_t btype = ext4_inode_type(sb, &ino);
    entry->st_mode = ext4_inode_get_mode(sb, &ino) | ino_to_st_mode(btype);
    // Update file type
    entry->st_nlink = ext4_inode_get_links_cnt(&ino);
    entry->st_uid = ext4_inode_get_uid(&ino);
    entry->st_gid = ext4_inode_get_gid(&ino);
    entry->st_blocks = ext4_inode_get_blocks_count(sb, &ino);
    entry->st_size = ext4_inode_get_size(sb, &ino);
    entry->st_blksize = ext4_sb_get_block_size(sb);
    entry->st_dev = ext4_inode_get_dev(&ino);
    return -err;
}

static int ext_statvfs(struct vfs_mount *mountp, const char *path, struct statvfs *stat)
{
    VFS_UNUSED(path);
    struct ext4_mount_stats estat;
    AUTO_PATH(mnt_path) = normalize_mount_point(mountp);
    int err = ext4_mount_point_stats(mnt_path, &estat);
    if (err)
    {
        return -err;
    }
    memset(stat, 0, sizeof(*stat));
    stat->f_bsize = estat.block_size;
    stat->f_frsize = estat.block_size;
    stat->f_blocks = estat.blocks_count;
    stat->f_bfree = estat.free_blocks_count;
    return -err;
}

static int ext_chmod(struct vfs_mount *mountp, const char *path, mode_t mode)
{
    VFS_UNUSED(mountp);
    return -ext4_mode_set(path, mode);
}

// Littlefs filesystem operations private structure
static const struct vfs_filesystem_ops ext4_fops =
{
    .open = ext_open,
    .read = ext_read,
    .write = ext_write,
    .lseek = ext_lseek,
    .tell = ext_tell,
    .truncate = ext_truncate,
    .sync = ext_sync,
    .close = ext_close,
    .opendir = ext_opendir,
    .readdir = ext_readdir,
    .closedir = ext_closedir,
    .mount = ext_mount,
    .unmount = ext_unmount,
    .unlink = ext_unlink,
    .rmdir  = ext_rmdir,
    .rename = ext_rename,
    .mkdir = ext_mkdir,
    .stat = ext_stat,
    .statvfs = ext_statvfs,
    .chmod = ext_chmod
};

/** Enable littlefs filesystem
 * @return error code
 */
int vfs_priv_enable_ext4_filesystem(void)
{
    return vfs_register_filesystem(vfs_fs_ext4, &ext4_fops);
}
