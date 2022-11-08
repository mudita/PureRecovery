#include "path_opts.h"
#include "dir_walker.h"
#include "log.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

struct unlink_data_s {
};

struct mv_data_s {
    const char *to;
};

static int cp_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *d) {

    int ret = 0;
    (void) h;
    struct mv_data_s *data = (struct mv_data_s *) (d);

    char *path_to_sanitize = strndup(path, strlen(path));
    char *dir_root = strndup(h->root_catalog, strlen(h->root_catalog));
    char *sanitized_path = path_sanitize(dir_root, path_to_sanitize);
    size_t final_path_size = strlen(sanitized_path) + strlen(data->to) + 2;
    char *final_path = (char *) calloc(1, final_path_size);
    snprintf(final_path, final_path_size, "%s/%s", data->to, sanitized_path);

    switch (what) {
        case DirHandlingDir:
            ret = mkdir(final_path, 0666);
            if (ret != 0 && errno == EEXIST) {
                debug_log("TMP dir: mkdir - directory already exists %s %d", final_path, ret);
                ret = 0;
            }
            break;
        case DirHandlingFile: {
            int src_fd = open(path, O_RDONLY);
            if (src_fd <= 0) {
                debug_log("Failed to open file %s to read", path);
                ret = 1;
                break;
            }

            int dst_fd = open(final_path, O_WRONLY | O_CREAT, 0666);
            if (dst_fd <= 0) {
                close(src_fd);
                debug_log("Failed to open file %s to write", final_path);
                ret = 1;
                break;
            }

            size_t buffer_size = 1024 * 1024;
            char *buffer = malloc(buffer_size); // 1MiB
            if (buffer == NULL) {
                close(dst_fd);
                close(src_fd);
                debug_log("Failed to allocate copy buffer");
                ret = 1;
                break;
            }

            ssize_t bytes_read;
            size_t bytes_left = lseek(src_fd, 0, SEEK_END);
            lseek(src_fd, 0, SEEK_SET);

            do {
                bytes_read = read(src_fd, buffer, buffer_size);
                if (bytes_read < 0) {
                    debug_log("Failed to read from source file\n");
                    ret = 1;
                    break;
                }
                if (write(dst_fd, buffer, bytes_read) < 0) {
                    debug_log("Failed to write to destination file\n");
                    ret = 1;
                    break;
                }
                bytes_left -= bytes_read;
            } while (bytes_left > 0);

            free(buffer);
            close(dst_fd);
            close(src_fd);
        }
            break;
        default:
            break;
    }

    free(final_path);
    free(dir_root);
    free(path_to_sanitize);
    return ret;
}

static int unlink_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *d) {

    int ret = 0;
    (void) h;

    switch (what) {
        case DirHandlingDir:
            break;
        case DirHandlingFile:
            ret = unlink(path);
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

    return rmdir(path);
}

void path_remove_dup_slash(char *from) {
    for (size_t i = 0; i < strlen(from) - 1; ++i) {
        if (from[i] == '/' && from[i + 1] == '/') {
            memcpy(from + i, from + i + 1, strlen(from + i + 1) + 1);
        }
    }
}

void path_remove_cwd(char *from) {
    if (strlen(from) >= 2 && strncmp(from, "./", 2) == 0) {
        memcpy(from, from + 2, strlen(from) + 1);
    }
}

/// remove dup from path and entry /
char *path_sanitize(char *from, char *path) {
    if (strlen(from) > strlen(path)) {
        return 0;
    }
    path_remove_dup_slash(from);
    path_remove_dup_slash(path);
    char *ret = path + strlen(from);
    if (ret[0] == '/') {
        return ret + 1;
    }
    return ret;
}

void path_remove_trailing_slash(char *out) {
    if (out == NULL) {
        return;
    }
    if (strlen(out) < 2) {
        return;
    }
    if (out[strlen(out) - 1] == '/') {
        out[strlen(out) - 1] = 0;
    }
}

const char *path_basename_const(const char *path) {
    const char *ret;
    return (ret = strrchr(path, '/')) ? ++ret : (ret = path);
}

bool path_check_if_exists(const char *path) {
    struct stat buf;
    return (stat(path, &buf) == 0);
}

bool recursive_unlink(const char *what) {
    bool success = true;
    struct dir_handler_s handle_walk;
    memset(&handle_walk, 0, sizeof handle_walk);
    unsigned int recursion_limit = 100;

    do {
        struct unlink_data_s data = {};
        recursive_dir_walker_init(&handle_walk, unlink_callback, &data);
        handle_walk.callback_dir_closed = remove_dir_callback;
        recursive_dir_walker(what, &handle_walk, &recursion_limit);
        recursive_dir_walker_deinit(&handle_walk);

        if (handle_walk.error) {
            debug_log("Unlink: walker error. errno: %d", errno);
            success = false;
            break;
        }
    } while (0);

    return success;
}

bool recursive_cp(const char *what, const char *where) {
    bool success = true;
    struct dir_handler_s handle_walk;
    memset(&handle_walk, 0, sizeof handle_walk);
    unsigned int recursion_limit = 100;

    struct mv_data_s data = {NULL};
    data.to = where;
    recursive_dir_walker_init(&handle_walk, cp_callback, &data);
    recursive_dir_walker(what, &handle_walk, &recursion_limit);
    recursive_dir_walker_deinit(&handle_walk);

    if (handle_walk.error) {
        debug_log("Copy: walker error. errno: %d", errno);
        success = false;
    }

    return success;
}
