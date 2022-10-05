#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "dir_walker.h"

#define MAX_PATH_LENGTH 1024

void recursive_dir_walker_init(struct dir_handler_s *s,
                               int (*callback)(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h,
                                               void *), void *data) {
    memset(s, 0, sizeof *s);
    s->not_first_call = true;
    s->callback = callback;
    s->callback_data = data;
}

void recursive_dir_walker_deinit(struct dir_handler_s *s) {
    free(s->root_catalog);
    s->root_catalog = NULL;
    s->not_first_call = false;
}

/// requires:
/// - opendir
/// - readdir
void recursive_dir_walker(const char *name, struct dir_handler_s *h, unsigned int *recursion_limit) {
    DIR *dir = NULL;
    struct dirent *entry = NULL;

    if (h == NULL || recursion_limit == NULL) {
        return;
    }
    if (h->user_break) {
        h->error = DirHandlingOk;
        return;
    }
    if (*recursion_limit == 0) {
        h->error = DirHandlingRecursionLimit;
        return;
    }
    --*recursion_limit;
    if (h->error != 0) {
        return;
    }
    if (!(dir = opendir(name))) {
        h->error = DirHandlingFS;
        return;
    }

    if (h->not_first_call) {
        h->root_catalog = (char *) malloc(strlen(name) + 1);
        sprintf(h->root_catalog, "%s", name);
        h->not_first_call = false;
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[MAX_PATH_LENGTH] = {0};
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if (strlen(name) + strlen(entry->d_name) > MAX_PATH_LENGTH) {
            h->error = DirHandlingPathTooLong;
            break;
        }
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        if (entry->d_type == DT_DIR) {
            //printf("%s/", entry->d_name);
            if (h->callback != NULL) {
                h->error_callback = (*h->callback)(path, DirHandlingDir, h, h->callback_data);
                if (h->error_callback != 0) {
                    h->error = DirHandlingCallback;
                    break;
                }
            }
            recursive_dir_walker(path, h, recursion_limit);
        } else {
            //printf("%s\n", entry->d_name);
            if (h->callback != NULL) {
                h->error_callback = (*h->callback)(path, DirHandlingFile, h, h->callback_data);
                if (h->error_callback != 0) {
                    h->error = DirHandlingCallback;
                    break;
                }
            }
        }
    }
    closedir(dir);
    if (h->callback_dir_closed != NULL && strcmp(h->root_catalog, name) != 0) {
        h->error_callback = (*h->callback_dir_closed)(name, h, h->callback_data);
    }
}


const char *dir_handling_strerror(enum dir_handling_err err) {
    switch (err) {
        case DirHandlingOk:
            return "DirHandlingOk";
        case DirHandlingFS:
            return "DirHandlingFS";
        case DirHandlingRecursionLimit:
            return "DirHandlingRecursionLimit";
        case DirHandlingPathTooLong:
            return "DirHandlingPathTooLong";
        case DirHandlingCallback:
            return "DirHandlingCallback";
    }
    return "undefined";
}
