#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

enum dir_handling_err {
    DirHandlingOk,
    DirHandlingFS,
    DirHandlingRecursionLimit,
    DirHandlingPathTooLong,
    DirHandlingCallback,
};

enum dir_handling_type_e {
    DirHandlingDir,
    DirHandlingFile,
};

struct dir_handler_s {
    bool not_first_call;                /// flag to mark first recursion run
    enum dir_handling_err error;        /// dir handling error
    int error_callback;                 /// error passed from callback
    int (*callback)(const char *path,
                    enum dir_handling_type_e what,
                    struct dir_handler_s *h,
                    void *);            /// callback to execute on node
    int (*callback_dir_closed)(const char *path,
                               struct dir_handler_s *h,
                               void *); /// callback to execute after leaving catalog
    void *callback_data;                /// data passed to callback
    char *root_catalog;                 /// start catalog for recursion
    bool user_break;                    /// whether user requested stop in callback
};

/// requires:
/// - opendir
/// - readdir
/// exits on first error
void recursive_dir_walker_init(
        struct dir_handler_s *s,
        int (*callback)(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *),
        void *data);

void recursive_dir_walker(const char *name, struct dir_handler_s *h, unsigned int *recursion_limit);

void recursive_dir_walker_deinit(struct dir_handler_s *s);

const char *dir_handling_strerror(enum dir_handling_err err);

#ifdef __cplusplus
}
#endif
