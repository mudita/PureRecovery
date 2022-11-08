add_library(common STATIC
        path_opts.c
        log.c
        sqlite3_vfs.c
        database.c
        mbr.c
        dir_walker.c
        mount_points.c
        )

target_link_libraries(common PRIVATE hal-common PUBLIC sqlite cjson system pure-core)
target_include_directories(common PUBLIC .)