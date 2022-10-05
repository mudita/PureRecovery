#include <prv/tinyvfs/vfs_device.h>
#include <prv/tinyvfs/vfs_priv_data.h>
#include <hal/tinyvfs.h>
#include <sys/_default_fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>

struct vfs_filesystem_entry
{
    const struct vfs_filesystem_ops *ops;
    vfs_filesystem_type_t type;
};

struct vfs_mount_entry
{

    struct vfs_mount *mnt;		  // List of mount points
    struct vfs_mount_entry *next; // Next mount entry
};

struct vfs_context
{
    struct vfs_mount_entry *fopsl;	   // FIlesystem entry list
    struct vfs_filesystem_entry *fste; // Filesystem entry
    size_t num_fse;					   // Filesystem ops
};

// VFS context state
static struct vfs_context ctx;

static const struct vfs_filesystem_ops *fs_type_get(vfs_filesystem_type_t type)
{
    for (size_t i = 0; i < ctx.num_fse; ++i)
    {
        if (ctx.fste[i].type == type)
        {
            return ctx.fste[i].ops;
        }
    }
    return NULL;
}

static int fs_get_mnt_point(struct vfs_mount **mnt_pntp, const char *name, size_t *match_len)
{
    struct vfs_mount *mnt_p = NULL;
    size_t longest_match = 0;
    size_t len, name_len = strlen(name);
    for (struct vfs_mount_entry *e = ctx.fopsl; e; e = e->next)
    {
        struct vfs_mount_entry *itr = e;
        len = itr->mnt->mountp_len;

        /*
         * Move to next node if mount point length is
         * shorter than longest_match match or if path
         * name is shorter than the mount point name.
         */
        if ((len < longest_match) || (len > name_len))
        {
            continue;
        }

        /*
         * Move to next node if name does not have a directory
         * separator where mount point name ends.
         */
        if ((len > 1) && (name[len] != '/') && (name[len] != '\0'))
        {
            continue;
        }

        /* Check for mount point match */
        if (strncmp(name, itr->mnt->mnt_point, len) == 0)
        {
            mnt_p = itr->mnt;
            longest_match = len;
        }
    }

    if (mnt_p == NULL)
    {
        return -ENOENT;
    }

    *mnt_pntp = mnt_p;
    if (match_len)
    {
        *match_len = mnt_p->mountp_len;
    }

    return 0;
}

/**
 * Register the filesystem
 * @param type Filesystem type
 * @param fops File system operation structure
 * @return error otherwvise success
 */
int vfs_register_filesystem(int type, const struct vfs_filesystem_ops *fops)
{
    int err = 0;
    if (fs_type_get(type) != NULL)
    {
        err = -EEXIST;
    }
    else
    {
        ctx.fste = reallocarray(ctx.fste, ctx.num_fse + 1, sizeof(struct vfs_filesystem_entry));
        if (!ctx.fste)
        {
            printf("vfs: %s: No more memory available\n", __PRETTY_FUNCTION__);
            return -ENOMEM;
        }
        ctx.fste[ctx.num_fse++] = (struct vfs_filesystem_entry){.ops = fops, .type = type};
    }
    return err;
}

/** Unregister all filesystems
*/
void vfs_unregister_all_filesystems(void)
{
    free(ctx.fste);
    ctx.fste = NULL;
    ctx.num_fse = 0;
}

int vfs_mount(struct vfs_mount *mp, int device)
{
    const struct vfs_filesystem_ops *fs;
    size_t len = 0;

    /* API sanity check 
    */
    if ((mp == NULL) || (mp->mnt_point == NULL))
    {
        printf("vfs: %s mount point not initialized\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    len = strlen(mp->mnt_point);

    if ((len <= 1) || (mp->mnt_point[0] != '/'))
    {
        printf("vfs: %s invalid mount point\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    /* Check if mount point already exists */
    for (struct vfs_mount_entry *e = ctx.fopsl; e; e = e->next)
    {
        struct vfs_mount *itr = e->mnt;
        /* continue if length does not match */
        if (len != itr->mountp_len)
        {
            continue;
        }

        if (strncmp(mp->mnt_point, itr->mnt_point, len) == 0)
        {
            printf("vfs: %s mount point already exists\n", __PRETTY_FUNCTION__);
            return -EBUSY;
        }
    }
    //Assign storage device
    mp->storage_dev = device;
    /* Get file system information */
    fs = fs_type_get(mp->type);
    if (fs == NULL)
    {
        printf("vfs: %s Filesystem not registered\n", __PRETTY_FUNCTION__);
        return -ENOENT;
    }

    if (fs->mount == NULL)
    {
        printf("vfs: %s Filesystem %i doesn't support mount\n", __PRETTY_FUNCTION__, mp->type);
        return -ENOTSUP;
    }

    if (fs->unmount == NULL)
    {
        printf("vfs warn: %s Filesystem %i doesn't support unmount\n", __PRETTY_FUNCTION__, mp->type);
    }

    int err = fs->mount(mp);
    if (err < 0)
    {
        printf("vfs warn: %s Filesystem mount error %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        return err;
    }
    /* Update mount point data and append it to the list */
    mp->mountp_len = len;
    mp->fs = fs;
    {
        struct vfs_mount_entry *ins = calloc(1, sizeof(struct vfs_mount_entry));
        ins->mnt = mp;
        ins->next = NULL;
        if (!ctx.fopsl)
        {
            ctx.fopsl = ins;
        }
        else
        {
            struct vfs_mount_entry *curr = ctx.fopsl;
            while (curr->next)
            {
                curr = curr->next;
            }
            curr->next = ins;
        }
    }
    return err;
}

int vfs_unmount(struct vfs_mount *mp)
{
    int err = -EINVAL;

    if (mp == NULL)
    {
        return err;
    }

    if (mp->fs == NULL)
    {
        printf("vfs: %s Filesystem not mounted %p\n", __PRETTY_FUNCTION__, mp);
        return -EINVAL;
    }

    if (mp->fs->unmount == NULL)
    {
        printf("vfs: %s Umount is not supported\n", __PRETTY_FUNCTION__);
        return -ENOTSUP;
    }

    err = mp->fs->unmount(mp);
    if (err < 0)
    {
        printf("vfs: %s Umount error %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        return -EINVAL;
    }

    /* clear file system interface */
    mp->fs = NULL;

    /* remove mount node from the list */
    for (struct vfs_mount_entry *c = ctx.fopsl; c; c = c->next)
    {
        if (c->mnt == mp)
        {
            struct vfs_mount_entry *n = c->next;
            free(c);
            if (c == ctx.fopsl)
            {
                ctx.fopsl = n;
            }
            c = n;
            break;
        }
    }
    return err;
}

/* File operations */
int vfs_open(struct vfs_file *filp, const char *file_name, int flags, mode_t mode)
{
    struct vfs_mount *mp;

    /* COpy flags to zfp for use with other fs_ API calls */
    filp->flags = flags;
    filp->mode = mode;

    if ((file_name == NULL) ||
            (strlen(file_name) <= 1) || (file_name[0] != '/'))
    {
        printf("vfs: %s Invalid file name\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    int err = fs_get_mnt_point(&mp, file_name, NULL);
    if (err < 0)
    {
        printf("vfs: %s Mount point not found\n", __PRETTY_FUNCTION__);
        return err;
    }

    filp->mp = mp;

    if (filp->mp->fs->open != NULL)
    {
        err = filp->mp->fs->open(filp, file_name, flags, mode);
        if (err < 0)
        {
            return err;
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

int vfs_close(struct vfs_file *filp)
{
    int err = -EINVAL;
    if (filp->mp == NULL)
    {
        return 0;
    }
    if (filp->mp->fs->close != NULL)
    {
        err = filp->mp->fs->close(filp);
        if (err < 0)
        {
            printf("vfs: %s File close error %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
            return err;
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    filp->mp = NULL;
    return err;
}

ssize_t vfs_read(struct vfs_file *filp, void *ptr, size_t size)
{
    int err = -EINVAL;
    if (filp->mp == NULL)
    {
        return -EBADF;
    }
    if (filp->mp->fs->read != NULL)
    {
        err = filp->mp->fs->read(filp, ptr, size);
        if (err < 0)
        {
            printf("vfs: %s File read error %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

ssize_t vfs_write(struct vfs_file *filp, const void *ptr, size_t size)
{
    int err = -EINVAL;
    if (filp->mp == NULL)
    {
        return -EBADF;
    }
    if (filp->mp->fs->write != NULL)
    {
        err = filp->mp->fs->write(filp, ptr, size);
        if (err < 0)
        {
            printf("vfs: %s File write error %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

ssize_t vfs_seek(struct vfs_file *filp, off_t offset, int whence)
{
    int err = -ENOTSUP;
    if (filp->mp == NULL)
    {
        return -EBADF;
    }
    if (filp->mp->fs->lseek != NULL)
    {
        err = filp->mp->fs->lseek(filp, offset, whence);
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

off_t vfs_tell(struct vfs_file *filp)
{
    int err = -ENOTSUP;
    if (filp->mp == NULL)
    {
        return -EBADF;
    }
    if (filp->mp->fs->tell != NULL)
    {
        err = filp->mp->fs->tell(filp);
        if (err < 0)
        {
            printf("vfs: %s File tell error %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

int vfs_ftruncate(struct vfs_file *filp, off_t length)
{
    int err = -EINVAL;
    if (filp->mp == NULL)
    {
        return -EBADF;
    }
    if (filp->mp->fs->truncate != NULL)
    {
        err = filp->mp->fs->truncate(filp, length);
        if (err < 0)
        {
            printf("vfs: %s File truncate error %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

int vfs_truncate(const char *abs_path, off_t length)
{
    struct vfs_file fil;
    int err = vfs_open(&fil, abs_path, O_WRONLY | O_CREAT, 0666);
    if (err)
    {
        return err;
    }
    err = vfs_ftruncate(&fil, length);
    int err2 = vfs_close(&fil);
    return (err) ? (err) : (err2);
}

int vfs_sync(struct vfs_file *filp)
{
    int err = -EINVAL;
    if (filp->mp == NULL)
    {
        return -EBADF;
    }
    if (filp->mp->fs->sync != NULL)
    {
        err = filp->mp->fs->sync(filp);
        if (err < 0)
        {
            printf("vfs: %s File sync error %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

/* Directory operations */
int vfs_opendir(struct vfs_dir *dirp, const char *abs_path)
{
    struct vfs_mount *me;
    int err = -EINVAL;

    if ((abs_path == NULL) ||
            (strlen(abs_path) < 1) || (abs_path[0] != '/'))
    {
        printf("vfs: %s Invalid filename \n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    if (strcmp(abs_path, "/") == 0)
    {

        dirp->mp = NULL;
        dirp->next_mnt = ctx.fopsl;
        return 0;
    }
    err = fs_get_mnt_point(&me, abs_path, NULL);
    if (err < 0)
    {
        printf("vfs: %s Mount point not found\n", __PRETTY_FUNCTION__);
        return err;
    }
    dirp->mp = me;
    if (dirp->mp->fs->opendir != NULL)
    {
        err = dirp->mp->fs->opendir(dirp, abs_path);
        if (err < 0)
        {
            printf("vfs: %s Directory open error %i (%s) path %s\n", __PRETTY_FUNCTION__, err, strerror(-err), abs_path);
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

int vfs_readdir(struct vfs_dir *dirp, struct dirent *entry)
{
    if (dirp->mp)
    {
        /* Delegate to mounted filesystem */
        int err = -EINVAL;

        if (dirp->mp->fs->readdir != NULL)
        {
            /* Loop until error or not special directory */
            while (true)
            {
                err = dirp->mp->fs->readdir(dirp, entry);
                if (err < 0)
                {
                    break;
                }
                if (entry->d_name[0] == 0)
                {
                    break;
                }
                if (entry->d_type != DT_DIR)
                {
                    break;
                }
                if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
                {
                    break;
                }
            }
            if (err < 0)
            {
                printf("vfs: %s Directory read error %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
            }
        }
        else
        {
            err = -ENOTSUP;
        }
        return err;
    }

    /* VFS root dir */
    if (dirp->next_mnt == NULL)
    {
        /* No more entries */
        entry->d_name[0] = 0;
        return 0;
    }

    /* Find the current and next entries in the mount point dlist */
    bool found = false;
    struct vfs_mount_entry *next = NULL;
    for (struct vfs_mount_entry *e = ctx.fopsl; e; e = e->next)
    {
        if (e == dirp->next_mnt)
        {
            found = true;

            /* Pull info from current entry */
            struct vfs_mount *mnt = e->mnt;
            entry->d_type = DT_DIR;
            strncpy(entry->d_name, mnt->mnt_point + 1, sizeof(entry->d_name) - 1);
            entry->d_name[sizeof(entry->d_name) - 1] = 0;
            /* Save pointer to the next one, for later */
            next = e->next;
            break;
        }
    }

    if (!found)
    {
        /* Current entry must have been removed before this
         * call to readdir -- return an error
         */
        return -ENOENT;
    }

    dirp->next_mnt = next;
    return 0;
}

int vfs_closedir(struct vfs_dir *dirp)
{
    int err = -EINVAL;

    if (dirp->mp == NULL)
    {
        /* VFS root dir */
        dirp->next_mnt = NULL;
        return 0;
    }

    if (dirp->mp->fs->closedir != NULL)
    {
        err = dirp->mp->fs->closedir(dirp);
        if (err < 0)
        {
            printf("vfs: %s Directory close error %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
            return err;
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    dirp->mp = NULL;
    return err;
}

/* Filesystem operations */
int vfs_mkdir(const char *abs_path)
{
    struct vfs_mount *mp;
    int err = -EINVAL;

    if ((abs_path == NULL) ||
            (strlen(abs_path) <= 1) || (abs_path[0] != '/'))
    {

        printf("vfs: %s Invalid filename\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    err = fs_get_mnt_point(&mp, abs_path, NULL);
    if (err < 0)
    {
        printf("vfs: %s Mount point not found\n", __PRETTY_FUNCTION__);
        return err;
    }

    if (mp->fs->mkdir != NULL)
    {
        err = mp->fs->mkdir(mp, abs_path);
        if (err < 0)
        {
            printf("vfs: %s Unable to create directory %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

int vfs_unlink(const char *abs_path)
{
    struct vfs_mount *mp;
    int err = -EINVAL;

    if ((abs_path == NULL) ||
            (strlen(abs_path) <= 1) || (abs_path[0] != '/'))
    {
        printf("vfs: %s Invalid filename\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    err = fs_get_mnt_point(&mp, abs_path, NULL);
    if (err < 0)
    {
        printf("vfs: %s Mount point not found\n", __PRETTY_FUNCTION__);
        return err;
    }

    if (mp->fs->unlink != NULL)
    {
        err = mp->fs->unlink(mp, abs_path);
        if (err < 0)
        {
            printf("vfs: %s Failed to unlink %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

int vfs_rmdir(const char *abs_path)
{
    struct vfs_mount *mp;
    int err = -EINVAL;

    if ((abs_path == NULL) ||
            (strlen(abs_path) <= 1) || (abs_path[0] != '/'))
    {
        printf("vfs: %s Invalid filename\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    err = fs_get_mnt_point(&mp, abs_path, NULL);
    if (err < 0)
    {
        printf("vfs: %s Mount point not found\n", __PRETTY_FUNCTION__);
        return err;
    }

    if (mp->fs->rmdir != NULL)
    {
        err = mp->fs->rmdir(mp, abs_path);
        if (err < 0)
        {
            printf("vfs: %s Failed to rmdir %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

int vfs_rename(const char *from, const char *to)
{
    struct vfs_mount *mp;
    size_t match_len;
    int err = -EINVAL;

    if ((from == NULL) || (strlen(from) <= 1) || (from[0] != '/') ||
            (to == NULL) || (strlen(to) <= 1) || (to[0] != '/'))
    {
        printf("vfs: %s Invalid filename\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    err = fs_get_mnt_point(&mp, from, &match_len);
    if (err < 0)
    {
        printf("vfs: %s Mount point not found\n", __PRETTY_FUNCTION__);
        return err;
    }

    /* Make sure both files are mounted on the same path */
    if (strncmp(from, to, match_len) != 0)
    {
        printf("vfs: %s Mount point not the same\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    if (mp->fs->rename != NULL)
    {
        err = mp->fs->rename(mp, from, to);
        if (err < 0)
        {
            printf("vfs: %s Failed to rename %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

int vfs_stat(const char *abs_path, struct stat *entry)
{
    struct vfs_mount *mp;
    int err = -EINVAL;

    if ((abs_path == NULL) ||
            (strlen(abs_path) <= 1) || (abs_path[0] != '/'))
    {
        printf("vfs: %s Invalid filename\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    err = fs_get_mnt_point(&mp, abs_path, NULL);
    if (err < 0)
    {
        printf("vfs: %s Mount point not found\n", __PRETTY_FUNCTION__);
        return err;
    }

    if (mp->fs->stat != NULL)
    {
        err = mp->fs->stat(mp, abs_path, entry);
        if (err < 0)
        {
            printf("vfs: %s Failed to stat %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

int vfs_statvfs(const char *abs_path, struct statvfs *stat)
{
    struct vfs_mount *mp;
    int err;

    if ((abs_path == NULL) ||
            (strlen(abs_path) <= 1) || (abs_path[0] != '/'))
    {
        printf("vfs: %s Invalid filename\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    err = fs_get_mnt_point(&mp, abs_path, NULL);
    if (err < 0)
    {
        printf("vfs: %s Mount point not found\n", __PRETTY_FUNCTION__);
        return err;
    }

    if (mp->fs->statvfs != NULL)
    {
        err = mp->fs->statvfs(mp, abs_path, stat);
        if (err < 0)
        {
            printf("vfs: %s Failed to vfstat %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }

    return err;
}

int vfs_readmount(int *index, const char **name)
{
    int err = -ENOENT;
    int cnt = 0;
    struct vfs_mount *itr = NULL;
    *name = NULL;
    for (struct vfs_mount_entry *e = ctx.fopsl; e; e = e->next)
    {
        if (*index == cnt)
        {
            itr = e->mnt;
            break;
        }
        ++cnt;
    }
    if (itr != NULL)
    {
        err = 0;
        *name = itr->mnt_point;
        ++(*index);
    }
    return err;
}

int vfs_chmod(const char *abs_path, mode_t mode)
{
    struct vfs_mount *mp;
    int err;

    if ((abs_path == NULL) ||
            (strlen(abs_path) <= 1) || (abs_path[0] != '/'))
    {
        printf("vfs: %s Invalid filename\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    err = fs_get_mnt_point(&mp, abs_path, NULL);
    if (err < 0)
    {
        printf("vfs: %s Mount point not found\n", __PRETTY_FUNCTION__);
        return err;
    }

    if (mp->fs->statvfs != NULL)
    {
        err = mp->fs->chmod(mp, abs_path, mode);
        if (err < 0)
        {
            printf("vfs: %s Failed to chmod %i (%s)\n", __PRETTY_FUNCTION__, err, strerror(-err));
        }
    }
    else
    {
        err = -ENOTSUP;
    }
    return err;
}

