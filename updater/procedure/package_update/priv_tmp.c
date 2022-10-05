#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <common/path_opts.h>
#include <common/enum_s.h>
#include <common/boot_files.h>
#include <procedure/backup/dir_walker.h>
#include "priv_tmp.h"

struct unlink_data_s {
    bool factory_reset;
};

enum tmp_error_e {
    ErrorTmpOk = 0,
    ErrorTmpFs,
    ErrorTmpWalk,
};

static int unlink_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *d) {
    int ret = 0;
    (void) h;
    struct unlink_data_s *data = (struct unlink_data_s *) (d);

    switch (what) {
        case DirHandlingDir:
            break;
        case DirHandlingFile:
            if (data->factory_reset) {
                for (size_t i = 0; i < db_extensions_list_size; ++i) {
                    const char *extension = strrchr(path, '.');
                    if (extension == NULL) {
                        ret = unlink(path);
                        debug_log("Unlink: removed file without extension: %s %d", path, ret);
                        break;
                    } else if (strcmp(db_extensions[i], extension) == 0) {
                        debug_log("Unlink: removing file: %s %d", path, ret);
                        ret = unlink(path);
                        break;
                    }
                }
            } else {
                debug_log("Unlink: removing file: %s %d", path, ret);
                ret = unlink(path);
            }
            break;
        default:
            ret = 1;
            break;
    }
    if (ret != 0) {
        debug_log("Unlink: unlink error: %d", ret);
    }
    return ret;
}

static int remove_dir_callback(const char *path, struct dir_handler_s *h, void *d) {
    (void) h;
    struct unlink_data_s *data = (struct unlink_data_s *) (d);
    int ret = 0;

    if (data->factory_reset) {
        goto exit;
    }

    ret = rmdir(path);
    if (ret != 0) {
        debug_log("Rmdir: rmdir error: %d", ret);
    }
    debug_log("Rmdir: removed catalog: %s %d", path, ret);

    exit:
    return ret;
}

bool recursive_unlink(const char *what, bool factory_reset) {
    bool success = true;
    struct dir_handler_s handle_walk;
    memset(&handle_walk, 0, sizeof handle_walk);
    unsigned int recursion_limit = 100;

    do {
        struct unlink_data_s data = {factory_reset};
        recursive_dir_walker_init(&handle_walk, unlink_callback, &data);
        handle_walk.callback_dir_closed = remove_dir_callback;
        recursive_dir_walker(what, &handle_walk, &recursion_limit);
        recursive_dir_walker_deinit(&handle_walk);

        if (handle_walk.error) {
            debug_log("Unlink: walker error %d (%s)", errno, strerror(errno));
            success = false;
            break;
        }
    } while (0);

    return success;
}

static bool create_single(const char *what) {
    bool success = true;

    struct stat data;
    int ret = stat(what, &data);
    if (ret == 0 && !recursive_unlink(what, NULL)) {
        success = false;
        goto exit;
    }
    if (ret == 0 && rmdir(what) != 0) {
        debug_log("Create dir: unable to remove directory: %s", what);
        success = false;
        goto exit;
    }
    if (ret != 0 && errno == ENOENT) {
        ret = 0;
    }

    if (mkdir(what, 0666) != 0) {
        debug_log("Create dir: failed to create a directory: %s : %d", what, errno);
        success = false;
        goto exit;
    }

    exit:
    return success;
}

bool tmp_create_catalog(struct update_handle_s *handle) {
    bool retval = true;
    debug_log("TMP dir: create temp catalog");

    if (!create_single(handle->tmp_os)) {
        retval = false;
        goto exit;
    }

    if (!create_single(handle->tmp_user)) {
        retval = false;
        goto exit;
    }

    exit:
    return retval;
}


struct mv_data_s {
    const char *to;
};

int mv_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *d) {

    int ret = 0;
    (void) h;
    struct mv_data_s *data = (struct mv_data_s *) (d);

    char *path_to_sanitize = strndup(path, strlen(path));
    char *dir_root = strndup(h->root_catalog, strlen(h->root_catalog));
    char *sanitized_path = path_sanitize(dir_root, path_to_sanitize);
    size_t final_path_size = strlen(sanitized_path) + strlen(data->to) + 2;
    char *final_path = (char *) calloc(1, final_path_size);
    snprintf(final_path, final_path_size, "%s/%s", data->to, sanitized_path);

    debug_log("TMP move: path: %s", final_path);

    switch (what) {
        case DirHandlingDir:
            ret = mkdir(final_path, 0666);
            if (ret != 0 && errno == EEXIST) {
                debug_log("TMP dir: mkdir - directory already exists %s %d", final_path, ret);
                ret = 0;
            }
            debug_log("TMP dir: created directory %s %d", final_path, ret);
            break;
        case DirHandlingFile: {
            struct stat data;
            ret = stat(final_path, &data); // Check if file already exists and remove if it does
            if (ret == ENOENT) {
                ret = unlink(final_path);
                if (ret != 0) {
                    debug_log("TMP move: unlinking existing file failed %d", ret);
                    goto exit;
                }
            }
            ret = rename(path, final_path);
            if (ret)
                debug_log("TMP dir: rename %s -> %s failed: %d %d %s\n", path, final_path, ret, errno, strerror(errno));
        }
            break;
        default:
            break;
    }
    exit:
    free(final_path);
    free(dir_root);
    free(path_to_sanitize);
    return ret;
}

static bool recursive_mv(const char *what, const char *where) {
    (void) where;
    bool success = true;
    struct dir_handler_s handle_walk;
    memset(&handle_walk, 0, sizeof handle_walk);
    unsigned int recursion_limit = 100;

    do {
        struct mv_data_s data = {NULL};
        data.to = where;
        recursive_dir_walker_init(&handle_walk, mv_callback, &data);
        recursive_dir_walker(what, &handle_walk, &recursion_limit);
        recursive_dir_walker_deinit(&handle_walk);

        if (handle_walk.error) {
            debug_log("Move: walker error. errno: %d", errno);
            success = false;
            break;
        }

        debug_log("Move: removing data after moving: %s", what);
        if (!recursive_unlink(what, NULL) != 0) {
            success = false;
            break;
        }

    } while (0);

    return success;
}

bool tmp_files_move(struct update_handle_s *handle) {
    bool success = true;

    debug_log("Move: move user data from tmp...");
    if (!recursive_mv(handle->tmp_os, handle->update_os)) {
        success = false;
        goto exit;
    }

    debug_log("Move: move os data from tmp...");
    if (!recursive_mv(handle->tmp_user, handle->update_user)) {
        success = false;
        goto exit;
    }

    exit:
    return success;
}

bool user_files_move_test(const char* const from, const char* const to) {
    bool success = true;

    do {
        /* TODO the creation of catalog should be split from moving */
        debug_log("Move: creating test catalog for user files...");
        if(!create_single(to)) {
            debug_log("Move: failed to remove source catalog");
            success = false;
            break;
        }

        debug_log("Move: moving user data from %s to %s", from, to);
        if(!recursive_mv(from, to)) {
            debug_log("Move: failed to remove source catalog");
            success = false;
            break;
        }

        debug_log("Move: removing source catalog %s", from);

        int err = rmdir(from);
        if (err) {
            debug_log("Move: failed to remove source catalog, error %d (%s)", err, strerror(err));
            success = false;
            break;
        }

    } while(0);

    return success;
}
