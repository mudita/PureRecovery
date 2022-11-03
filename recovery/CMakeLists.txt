add_subdirectory(common)

add_library(recovery STATIC
        flash_bootloader.c
        flash_secure_fuses.c
        security/pgmkeys.c
        )

target_link_libraries(recovery PRIVATE hal-common md5 common)
target_include_directories(recovery PUBLIC .)