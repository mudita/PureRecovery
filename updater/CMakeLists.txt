# Target
set(UPDATER ${CMAKE_PROJECT_NAME}_RT)
add_executable(${UPDATER})
target_compile_features(${UPDATER} PRIVATE
    c_std_11
)

target_compile_options(${UPDATER} PRIVATE -Wall -Wextra -Werror)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "**/*.c*"
    "*.c*"
)

# Source directories
target_sources(
    ${UPDATER}
    PRIVATE
    ${SRC_FILES}
    common/common/database.c
    )

# Libraries
target_link_libraries(
    ${UPDATER}
    PRIVATE 
    system
    system-runtime-objs 
    hal
    microtar
    klib
    common
    cjson
    md5
    gui
)

target_include_directories(
    ${UPDATER}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

option (ENABLE_SECURE_BOOT "Build signed binary for Secure Boot" OFF)
set(SIGN_CLIENT_PATH "${CMAKE_SOURCE_DIR}/../sign_server/key_client" CACHE PATH "signclient.py path")
set(SERVER "https://172.17.0.1:4430" CACHE STRING "sign server address")
set(BIN_FILE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_RT.bin)

# Create binary format
print_section_sizes(${UPDATER})

if (ENABLE_SECURE_BOOT)
    message(STATUS "Secure boot enabled")

    IF(EXISTS ${SIGN_CLIENT_PATH}/signclient.py)
        create_signed_binary( ${UPDATER} ALL)
    else()
        message( FATAL_ERROR "signclient.py not found in ${SIGN_CLIENT_PATH}" )
    endif()
else()
   create_binary_output( ${UPDATER} "bin" ALL)
endif()
