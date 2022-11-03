# Unit tests on the target platform file

set( TARGET_TEST ${CMAKE_PROJECT_NAME}-test )
add_executable( ${TARGET_TEST} EXCLUDE_FROM_ALL)

target_compile_options( ${TARGET_TEST} PRIVATE -Wall -Wextra -Wno-unused)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "src/*.c*"
)

# Source directories
target_sources( ${TARGET_TEST} PRIVATE ${SRC_FILES} )

target_compile_features(${TARGET_TEST} PRIVATE
    c_std_11
)
target_compile_options( ${TARGET_TEST} PRIVATE
    -Wall -Wextra
)

target_link_libraries( ${TARGET_TEST}
    PRIVATE
    system
    system-runtime-objs
    hal
    seatest
    common
)

#include(secure-boot)

# Create binary format
print_section_sizes( ${TARGET_TEST})


if (ENABLE_SECURE_BOOT)
    create_signed_binary( ${TARGET_TEST})
else()
    create_binary_output( ${TARGET_TEST} "bin")
endif()
add_custom_target( on-target-tests  DEPENDS ${TARGET_TEST}.bin)
