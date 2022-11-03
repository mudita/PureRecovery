add_library(common STATIC
        path_opts.c
        log.c
        sqlite3_vfs.c
        database.c
        )

target_link_libraries(common PRIVATE hal-common PUBLIC sqlite cjson)
target_include_directories(common PUBLIC .)