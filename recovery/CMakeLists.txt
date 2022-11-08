add_subdirectory(common)

add_library(recovery STATIC
        flash_bootloader.c
        flash_secure_fuses.c
        convert_fs.c
        security/pgmkeys.c
        )

target_link_libraries(recovery PRIVATE hal-common md5 common system pure-core)
target_include_directories(recovery PUBLIC .)