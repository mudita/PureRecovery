#if !defined(SQLITE_TEST) || SQLITE_OS_UNIX

#include "sqlite3_vfs.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "hal/delay.h"

/* The maximum pathname length supported by this VFS */
#define SQLITE_VFS_MAX_PATH_LENGTH 512

/* Sector size returned by VFS */
#define SECTOR_SIZE 512

/* Size of array storing file open flags */
#define OFLAGS_SIZE 4

#define UNUSED(x) ((void)(x))

/*
 ** When using this VFS, the sqlite3_file* handles that SQLite uses are
 ** actually pointers to instances of type vfs_file.
 */
typedef struct vfs_file {
    sqlite3_file base;  // Base struct, must be first
    FILE *fd;
} vfs_file_s;

/* Basic operations */
static long sqlite3_size(FILE *fd) {
    const size_t pos = ftell(fd);
    fseek(fd, 0, SEEK_END);
    const size_t fsize = ftell(fd);
    fseek(fd, pos, SEEK_SET);
    return fsize;
}

/*
 ** Close a file.
 */
static int sqlite3_vfs_close(sqlite3_file *sqlite_file) {
    vfs_file_s *file = (vfs_file_s *) sqlite_file;
    return fclose(file->fd);
}

/*
 ** Read data from a file.
 */
static int sqlite3_vfs_read(sqlite3_file *sqlite_file, void *data, int size, sqlite_int64 offset) {
    vfs_file_s *file = (vfs_file_s *) sqlite_file;

    fseek(file->fd, (long) offset, SEEK_SET);
    int result = (int) fread(data, 1, size, file->fd);

    if (result == size) {
        return SQLITE_OK;
    } else if (result < size) {
        return SQLITE_IOERR_SHORT_READ;
    }

    return SQLITE_IOERR_READ;
}

/*
 ** Write data to a crash-file.
 */
static int sqlite3_vfs_write(sqlite3_file *sqlite_file, const void *data, int size, sqlite_int64 offset) {
    vfs_file_s *file = (vfs_file_s *) sqlite_file;

    fseek(file->fd, (long) offset, SEEK_SET);
    int result = (int) fwrite(data, 1, size, file->fd);
    if (result == size) {
        return SQLITE_OK;
    } else if (result < size) {
        return SQLITE_IOERR_SHORT_READ;
    }

    return SQLITE_IOERR_READ;
}

/*
 ** Truncate a file. This is a no-op for this VFS (see header comments at
 ** the top of the file).
 */
static int sqlite3_vfs_truncate(sqlite3_file *sqlite_file, sqlite_int64 size) {
    UNUSED(sqlite_file);
    UNUSED(size);
    return SQLITE_IOERR_TRUNCATE;
}

/*
 ** Sync the contents of the file to the persistent media.
 */
static int sqlite3_vfs_sync(sqlite3_file *sqlite_file, int flags) {
    UNUSED(flags);

    vfs_file_s *file = (vfs_file_s *) sqlite_file;
    return (fflush(file->fd) == 0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
 ** Write the size of the file in bytes to *pSize.
 */
static int sqlite3_vfs_file_size(sqlite3_file *sqlite_file, sqlite_int64 *size) {
    vfs_file_s *file = (vfs_file_s *) sqlite_file;

    *size = sqlite3_size(file->fd);

    return SQLITE_OK;
}

/*
 ** Locking functions. The xLock() and xUnlock() methods are both no-ops.
 ** The xCheckReservedLock() always indicates that no other process holds
 ** a reserved lock on the database file. This ensures that if a hot-journal
 ** file is found in the file-system it is rolled back.
 */
static int sqlite3_vfs_lock(sqlite3_file *sqlite_file, int lock) {
    UNUSED(sqlite_file);
    UNUSED(lock);
    return SQLITE_OK;
}

static int sqlite3_vfs_unlock(sqlite3_file *sqlite_file, int lock) {
    UNUSED(sqlite_file);
    UNUSED(lock);
    return SQLITE_OK;
}

static int sqlite3_vfs_check_reserved_lock(sqlite3_file *sqlite_file, int *res_out) {
    UNUSED(sqlite_file);
    *res_out = 0;
    return SQLITE_OK;
}

/*
 ** No xFileControl() verbs are implemented by this VFS.
 */
static int sqlite3_vfs_file_control(sqlite3_file *sqlite_file, int op, void *arg) {
    UNUSED(sqlite_file);
    UNUSED(op);
    UNUSED(arg);
    return SQLITE_NOTFOUND;
}

/*
 ** The xSectorSize() and xDeviceCharacteristics() methods. These two
 ** may return special values allowing SQLite to optimize file-system
 ** access to some extent. But it is also safe to simply return 0.
 */
static int sqlite3_vfs_sector_size(sqlite3_file *sqlite_file) {
    UNUSED(sqlite_file);
    return SECTOR_SIZE;
}

static int sqlite3_vfs_device_characteristics(sqlite3_file *sqlite_file) {
    UNUSED(sqlite_file);
    return SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN;
}

/*
 ** Query the file-system to see if the named file exists, is readable or
 ** is both readable and writable.
 */
static int sqlite3_vfs_access(sqlite3_vfs *vfs, const char *path, int flags, int *res_out) {
    FILE *fp;
    UNUSED(vfs);

    assert(flags == SQLITE_ACCESS_EXISTS       // access(zPath, F_OK)
           || flags == SQLITE_ACCESS_READ      // access(zPath, R_OK)
           || flags == SQLITE_ACCESS_READWRITE // access(zPath, R_OK|W_OK)
    );

    fp = fopen(path, "r");
    if (fp != NULL) {
        if (res_out) {
            *res_out = flags;
        }
        fclose(fp);
    } else if (res_out) {
        *res_out = 0;
    }

    return SQLITE_OK;
}

static int sqlite3_vfs_open(sqlite3_vfs *vfs,           // VFS
                            const char *name,           // File to open, or 0 for a temp file
                            sqlite3_file *sqlite_file,  // Pointer to VFSFile struct to populate
                            int flags,                  // Input SQLITE_OPEN_XXX flags
                            int *out_flags              // Output SQLITE_OPEN_XXX flags (or NULL)
) {
    UNUSED(vfs);

    static const sqlite3_io_methods io_methods = {
            1,                                  // iVersion
            sqlite3_vfs_close,                  // xClose
            sqlite3_vfs_read,                   // xRead
            sqlite3_vfs_write,                  // xWrite
            sqlite3_vfs_truncate,               // xTruncate
            sqlite3_vfs_sync,                   // xSync
            sqlite3_vfs_file_size,              // xFileSize
            sqlite3_vfs_lock,                   // xLock
            sqlite3_vfs_unlock,                 // xUnlock
            sqlite3_vfs_check_reserved_lock,    // xCheckReservedLock
            sqlite3_vfs_file_control,           // xFileControl
            sqlite3_vfs_sector_size,            // xSectorSize
            sqlite3_vfs_device_characteristics,  // xDeviceCharacteristics
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
    };

    vfs_file_s *file = (vfs_file_s *) sqlite_file; // Populate this structure
    char *buffer = NULL;

    if (name == NULL) {
        return SQLITE_IOERR;
    }

    memset(file, 0, sizeof(vfs_file_s));
    char oflags[OFLAGS_SIZE];

    if (flags & SQLITE_OPEN_READONLY) {
        strncpy(oflags, "r", OFLAGS_SIZE);
    } else if ((flags & SQLITE_OPEN_READWRITE) && (flags & SQLITE_OPEN_CREATE)) {
        /* Check if database specified exists */
        file->fd = fopen(name, "r");
        if (file->fd == NULL) {
            strncpy(oflags, "w+", OFLAGS_SIZE); // Database doesn't exist, create new one with read&write permissions
        } else {
            fclose(file->fd);
            strncpy(oflags, "r+", OFLAGS_SIZE); // Database exists, open it with read&write permissions
        }
    } else {
        strncpy(oflags, "r+", OFLAGS_SIZE);
    }

    file->fd = fopen(name, oflags);
    if (file->fd == NULL) {
        sqlite3_free(buffer);
        return SQLITE_CANTOPEN;
    }

    if (out_flags) {
        *out_flags = flags;
    }
    file->base.pMethods = &io_methods;

    return SQLITE_OK;
}

/*
 ** Delete the file identified by argument zPath. If the dirSync parameter
 ** is non-zero, then ensure the file-system modification to delete the
 ** file has been synced to disk before returning.
 */
static int sqlite3_vfs_delete(sqlite3_vfs *vfs, const char *path, int dir_sync) {
    UNUSED(dir_sync);
    UNUSED(vfs);
    return (remove(path) == 0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
}

/*
 ** Argument zPath points to a nul-terminated string containing a file path.
 ** If zPath is an absolute path, then it is copied as is into the output
 ** buffer. Otherwise, if it is a relative path, then the equivalent full
 ** path is written to the output buffer.
 **
 ** This function assumes that paths are UNIX style. Specifically, that:
 **
 **   1. Path components are separated by a '/'. and
 **   2. Full paths begin with a '/' character.
 */
static int sqlite3_vfs_full_pathname(sqlite3_vfs *vfs, /* VFS */
                                     const char *path, /* Input path (possibly a relative path) */
                                     int size,      /* Size of output buffer in bytes */
                                     char *path_out     /* Pointer to output buffer */
) {
    UNUSED(vfs);
    sqlite3_snprintf(size, path_out, "%s", path);
    path_out[size - 1] = '\0';
    return SQLITE_OK;
}

/*
 ** The following four VFS methods:
 **
 **   xDlOpen
 **   xDlError
 **   xDlSym
 **   xDlClose
 **
 ** are supposed to implement the functionality needed by SQLite to load
 ** extensions compiled as shared objects. This simple VFS does not support
 ** this functionality, so the following functions are no-ops.
 */
static void *sqlite3_vfs_DlOpen(sqlite3_vfs *vfs, const char *path) {
    UNUSED(vfs);
    UNUSED(path);
    return NULL;
}

static void sqlite3_vfs_DlError(sqlite3_vfs *vfs, int size, char *err_msg) {
    UNUSED(vfs);
    sqlite3_snprintf(size, err_msg, "Loadable extensions are not supported");
    err_msg[size - 1] = '\0';
}

static void (*sqlite3_vfs_DlSym(sqlite3_vfs *vfs, void *pH, const char *z))(void) {
    UNUSED(vfs);
    UNUSED(pH);
    UNUSED(z);
    return NULL;
}

static void sqlite3_vfs_DlClose(sqlite3_vfs *vfs, void *handle) {
    UNUSED(vfs);
    UNUSED(handle);
}

/*
 ** Parameter zByte points to a buffer nByte bytes in size. Populate this
 ** buffer with pseudo-random data.
 */
static int sqlite3_vfs_randomness(sqlite3_vfs *vfs, int size, char *buffer) {
    UNUSED(vfs);
    UNUSED(size);
    UNUSED(buffer);
    return SQLITE_PERM;
}

/*
 ** Sleep for at least nMicro microseconds. Return the (approximate) number
 ** of microseconds slept for.
 */
static int sqlite3_vfs_sleep(sqlite3_vfs *vfs, int microseconds) {
    UNUSED(vfs);
    UNUSED(microseconds);

    const int delay = microseconds / 1000;

    msleep(delay);

    return 0;// microseconds;
}

/*
 ** Set *pTime to the current UTC time expressed as a Julian day. Return
 ** SQLITE_OK if successful, or an error code otherwise.
 **
 **   http://en.wikipedia.org/wiki/Julian_day
 */
static int sqlite3_vfs_current_time(sqlite3_vfs *vfs, double *pTime) {
    UNUSED(vfs);

    time_t timestamp;
    time(&timestamp);

    const double julianDayAtEpoch = 2440587.5;
    const double secondsPerDay = 86400.0;

    *pTime = timestamp / secondsPerDay + julianDayAtEpoch;

    return SQLITE_OK;
}

/*
 ** This function returns a pointer to the VFS implemented in this file.
 ** To make the VFS available to SQLite:
 **
 **   sqlite3_vfs_register(sqlite3_ecophonevfs(), 0);
 */
sqlite3_vfs *sqlite3_vfs_wrap(void) {
    static sqlite3_vfs vfs = {
            1,                    /* iVersion */
            sizeof(vfs_file_s), /* szOsFile */
            SQLITE_VFS_MAX_PATH_LENGTH,          /* mxPathname */
            0,                    /* pNext */
            "PurePhone",           /* zName */
            0,                    /* pAppData */
            sqlite3_vfs_open,         /* xOpen */
            sqlite3_vfs_delete,       /* xDelete */
            sqlite3_vfs_access,       /* xAccess */
            sqlite3_vfs_full_pathname, /* xFullPathname */
            sqlite3_vfs_DlOpen,       /* xDlOpen */
            sqlite3_vfs_DlError,      /* xDlError */
            sqlite3_vfs_DlSym,        /* xDlSym */
            sqlite3_vfs_DlClose,      /* xDlClose */
            sqlite3_vfs_randomness,   /* xRandomness */
            sqlite3_vfs_sleep,        /* xSleep */
            sqlite3_vfs_current_time,  /* xCurrentTime */
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
    };
    return &vfs;
}

#endif /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


