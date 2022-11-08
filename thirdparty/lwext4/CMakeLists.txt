cmake_minimum_required(VERSION 3.13)
add_library(lwext4 STATIC)

include(lwext4_macros.cmake)

target_include_directories(lwext4
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)

target_compile_definitions(lwext4
    PRIVATE
    CONFIG_DEBUG_PRINTF=1
    CONFIG_DEBUG_ASSERT=0
    CONFIG_HAVE_OWN_OFLAGS=0
    CONFIG_HAVE_OWN_ERRNO=0
    CONFIG_BLOCK_DEV_CACHE_SIZE=256
    CONFIG_EXT4_BLOCKDEVS_COUNT=3
    CONFIG_EXT4_MOUNTPOINTS_COUNT=3
)
target_compile_options(lwext4
    PRIVATE
    -Wno-format
)

target_sources(lwext4
    PRIVATE
        src/src/ext4_balloc.c
        src/src/ext4_bcache.c
        src/src/ext4_bitmap.c
        src/src/ext4_blockdev.c
        src/src/ext4_block_group.c
        src/src/ext4.c
        src/src/ext4_crc32.c
        src/src/ext4_debug.c
        src/src/ext4_dir.c
        src/src/ext4_dir_idx.c
        src/src/ext4_extent.c
        src/src/ext4_fs.c
        src/src/ext4_hash.c
        src/src/ext4_ialloc.c
        src/src/ext4_inode.c
        src/src/ext4_journal.c
        src/src/ext4_mbr.c
        src/src/ext4_mkfs.c
        src/src/ext4_super.c
        src/src/ext4_trans.c
        src/src/ext4_xattr.c
    PUBLIC
        src/include/ext4_balloc.h
        src/include/ext4_bcache.h
        src/include/ext4_bitmap.h
        src/include/ext4_blockdev.h
        src/include/ext4_block_group.h
        src/include/ext4_config.h
        src/include/ext4_crc32.h
        src/include/ext4_debug.h
        src/include/ext4_dir.h
        src/include/ext4_dir_idx.h
        src/include/ext4_errno.h
        src/include/ext4_extent.h
        src/include/ext4_fs.h
        src/include/ext4.h
        src/include/ext4_hash.h
        src/include/ext4_ialloc.h
        src/include/ext4_inode.h
        src/include/ext4_journal.h
        src/include/ext4_mbr.h
        src/include/ext4_misc.h
        src/include/ext4_mkfs.h
        src/include/ext4_oflags.h
        src/include/ext4_super.h
        src/include/ext4_trans.h
        src/include/ext4_types.h
        src/include/ext4_xattr.h
        src/include/misc/queue.h
        src/include/misc/tree.h
)

lwext4_output_configure()

