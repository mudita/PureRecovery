# System library for all targets
set( LIB_NAME system )

add_library( ${LIB_NAME} STATIC )

set(LIB_RUNTIME_OBJS ${LIB_NAME}-runtime-objs)
add_library( ${LIB_RUNTIME_OBJS} OBJECT)
target_link_libraries( ${LIB_RUNTIME_OBJS} PUBLIC ${LIB_NAME} )


file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS  
    "*.c*"
)

target_sources( ${LIB_NAME} PRIVATE ${SRC_FILES} )
target_link_libraries( ${LIB_NAME} PUBLIC hal-common )

file(GLOB_RECURSE SRC_RUNTIME CONFIGURE_DEPENDS
    "syscalls/*.c*"
    "boot/*.c"
)


target_sources( ${LIB_RUNTIME_OBJS} PRIVATE ${SRC_RUNTIME} )


# Include directories
target_include_directories( ${LIB_NAME} PUBLIC
    CMSIS
    include
    include/drivers
)
# User defines
target_compile_definitions(${LIB_NAME} PUBLIC
    FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1
    __STARTUP_INITIALIZE_NONCACHEDATA
    XIP_EXTERNAL_FLASH=0
    CPU_MIMXRT1051DVL6B_cm7
    CPU_MIMXRT1051DVL6B
    __FPU_PRESENT
    __MCUXPRESSO
    SKIP_SYSCLK_INIT
    _GNU_SOURCE
    DISABLE_WATCHDOG
)


if (NOT ENABLE_SECURE_BOOT)
    # Only enable the build-time boot header in a non-secure configuration,
    # a header for Secure Boot is added post-build when signing the binary
    target_compile_definitions(${LIB_NAME} PUBLIC
        XIP_BOOT_HEADER_ENABLE=1
        XIP_BOOT_HEADER_DCD_ENABLE=0
    )
endif()



#target_compile_options(
#    ${LIB_RUNTIME_OBJS} PRIVATE "-Werror"
#)
