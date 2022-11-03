# System library for all targets
set( LIB_NAME hal-common)
add_library( ${LIB_NAME} STATIC )

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS  
    "src/*.c*"
)

target_sources( ${LIB_NAME} PRIVATE ${SRC_FILES} )

#Libraries
target_link_libraries( ${LIB_NAME} 
PUBLIC
    hal
    fatfs
    littlefs
    lwext4
PRIVATE
    system
    segger::rtt
)

# Make function implementations from boot_reason.c available to the 'pure-core' library
target_link_libraries(pure-core PRIVATE hal-common)

# Includes
target_include_directories( ${LIB_NAME}
PUBLIC
    include
    src/display
)

target_compile_options( ${LIB_NAME} PRIVATE
    -Wall
    -Wextra
    -Wno-unused
)
