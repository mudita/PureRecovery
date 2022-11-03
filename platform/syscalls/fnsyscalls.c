#include <errno.h>
#include <hal/tinyvfs.h>
#include <stdlib.h>
#include <reent.h>

int mkdir(const char *path, mode_t mode) {
    (void) mode;
    int err = vfs_mkdir(path);
    if (err < 0) {
        errno = -err;
        err = -1;
    }
    return err;
}

int _unlink(char *name) {
    int err = vfs_unlink(name);
    if (err < 0) {
        errno = -err;
        err = -1;
    }
    return err;
}

int rmdir(char *name) {
    int err = vfs_rmdir(name);
    if (err < 0) {
        errno = -err;
        err = -1;
    }
    return err;
}

int _rename_r(struct _reent *reent, const char *oldpath, const char *newpath) {
    int err = vfs_rename(oldpath, newpath);
    if (err < 0) {
        reent->_errno = -err;
        err = -1;
    }
    return err;
}

int _stat(const char *file, struct stat *st) {
    int err = vfs_stat(file, st);
    if (err < 0) {
        errno = -err;
        err = -1;
    }
    return err;
}

int chmod(const char *path, mode_t mode) {
    int err = vfs_chmod(path, mode);
    if (err < 0) {
        errno = -err;
        err = -1;
    }
    return err;
}

int statvfs(const char *path, struct statvfs *buf) {
    int err = vfs_statvfs(path, buf);
    if (err < 0) {
        errno = -err;
        err = -1;
    }
    return err;
}

int truncate(const char *path, off_t length) {
    int err = vfs_truncate(path, length);
    if (err < 0) {
        errno = -err;
        err = -1;
    }
    return err;
}

char *getcwd(char *__buf, size_t __size) {
    int err= vfs_getcwd(__buf,__size);
    if (err < 0) {
        errno = -err;
        return NULL;
    }
    return __buf;
}

int symlink(const char *__name1, const char *__name2) {
    errno = ENOSYS;
    return -1;
}

ssize_t readlink(const char *__restrict __path,
                 char *__restrict __buf, size_t __buflen) {
    errno = ENOSYS;
    return -1;
}

int chdir(const char *__path) {
    int err= vfs_chdir(__path);
    if (err < 0) {
        errno = -err;
        err = -1;
    }
    return err;
}
