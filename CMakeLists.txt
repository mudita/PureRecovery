cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(Product)
include(Board)
include(GenerateDocs)
include(GenerateVersion)
include(GenerateConfigld)

validate_product_selection()
select_board()

message("Selected product: ${PRODUCT}")
message("Selected board:   ${BOARD}")

if (NOT DEFINED TARGET)
    message(WARNING "using default RT1051 platform")
    set(TARGET RT1051)
endif ()

macro(use_rt1051)
    message(STATUS "Select toolchain, linker and includes for platform rt1051")
    include(toolchain-arm-none-eabi)
    set(LINKER_SCRIPTS
            ldscripts/libs.ld
            ldscripts/memory.ld
            ldscripts/sections.ld
            )
    include(rt1051)
endmacro()

message("Target: ${TARGET}")

if (DEFINED TARGET AND ${TARGET} STREQUAL linux)
    add_custom_target(${CMAKE_PROJECT_NAME} DEPENDS test_backup)
else ()
    use_rt1051()
endif ()

project(PureRecovery LANGUAGES C CXX ASM)

add_subdirectory(products)
add_subdirectory(thirdparty)
add_subdirectory(platform)
add_subdirectory(hal)
add_subdirectory(recovery)
add_subdirectory(luarecovery)

add_executable(${CMAKE_PROJECT_NAME} recovery/main.c)
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE c_std_11)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra)
target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        PRIVATE
        system
        system-runtime-objs
        hal
        common
        recovery
        pure-core

        lua
        lualibs
        luarecovery
)

option(ENABLE_SECURE_BOOT "Build signed binary for Secure Boot" OFF)
set(SIGN_CLIENT_PATH "${CMAKE_SOURCE_DIR}/../sign_server/key_client" CACHE PATH "signclient.py path")
set(SERVER "https://172.17.0.1:4430" CACHE STRING "sign server address")
set(BIN_FILE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_RT.bin)

if (ENABLE_SECURE_BOOT)
    message(STATUS "Secure boot enabled")

    IF (EXISTS ${SIGN_CLIENT_PATH}/signclient.py)
        create_signed_binary(${CMAKE_PROJECT_NAME} ALL)
    else ()
        message(FATAL_ERROR "signclient.py not found in ${SIGN_CLIENT_PATH}")
    endif ()
else ()
    create_binary_output(${CMAKE_PROJECT_NAME} "bin" ALL)
endif ()

# Print elf section sizes
print_section_sizes(${CMAKE_PROJECT_NAME})

# Generate documentation
add_dependencies(docs ${CMAKE_PROJECT_NAME})

if (DEFINED TARGET AND ${TARGET} STREQUAL linux)
    add_subdirectory(unittest)
else ()
    add_subdirectory(tests)
endif ()


