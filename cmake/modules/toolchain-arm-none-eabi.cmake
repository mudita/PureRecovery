# Append current directory to CMAKE_MODULE_PATH for making device specific cmake modules visible
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Target definition
set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Set system depended extensions
if(WIN32)
    set(TOOLCHAIN_EXT ".exe" )
else()
    set(TOOLCHAIN_EXT "" )
endif()

# Set toolchain paths
set(TOOLCHAIN arm-none-eabi)
if(NOT DEFINED TOOLCHAIN_PREFIX)
    set(CROSS_GCC_EXE ${TOOLCHAIN}-gcc${TOOLCHAIN_EXT})
    find_program(CROSS_GCC_FILE ${CROSS_GCC_EXE} PATHS $ENV{HOME} /usr /usr/local REQUIRED)
    get_filename_component(GCC_BASE_PATH ${CROSS_GCC_FILE} DIRECTORY)
    get_filename_component(TOOLCHAIN_PREFIX ${GCC_BASE_PATH} DIRECTORY)
endif()
set(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)
set(TOOLCHAIN_INC_DIR ${TOOLCHAIN_PREFIX}/${TOOLCHAIN}/include)
set(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_PREFIX}/${TOOLCHAIN}/lib)

# Target suffix to elf file
set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

# Perform compiler test with static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#################################
# Set compiler/linker flags
#################################

# Generic C/C++ options
set(OBJECT_GEN_FLAGS "-mthumb -fno-builtin -Wall -ffunction-sections -fdata-sections -fomit-frame-pointer -fstrict-aliasing")
set(CMAKE_C_FLAGS   "${OBJECT_GEN_FLAGS} -std=gnu17" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} " CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp " CACHE INTERNAL "ASM Compiler options")

# General linker flags
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections --specs=nosys.specs -mthumb -Wl,-Map=${CMAKE_PROJECT_NAME}.map" CACHE INTERNAL "Linker options")

##################################################
# Set debug/release build configuration Options
##################################################

# Options for DEBUG build
set(CMAKE_C_FLAGS_DEBUG "-Og -g -DDEBUG" CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g -DDEBUG" CACHE INTERNAL "C++ Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "-g -DDEBUG" CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Linker options for debug build type")

# Options for RELEASE build
set(CMAKE_C_FLAGS_RELEASE "-O2 -flto" CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -flto" CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O2 -flto" CACHE INTERNAL "Linker options for release build type")

# Flags for RELWITHDEBINFO build
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -flto -g" CACHE INTERNAL "C Compiler options for relwithdebinfo build type")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -flto -g" CACHE INTERNAL "C++ Compiler options for relwithdebinfo build type")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "ASM Compiler options for relwithdebinfo build type")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-O2 -flto" CACHE INTERNAL "Linker options for relwithdebinfo build type")


#############################
# Set compilers
#############################

set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc${TOOLCHAIN_EXT} CACHE INTERNAL "C Compiler")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-g++${TOOLCHAIN_EXT} CACHE INTERNAL "C++ Compiler")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-as${TOOLCHAIN_EXT} CACHE INTERNAL "ASM Compiler")
set(CMAKE_AR ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc-ar${TOOLCHAIN_EXT} CACHE INTERNAL "AR tool")
set(CMAKE_RANLIB ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc-ranlib${TOOLCHAIN_EXT} CACHE INTERNAL "RANLIB tool")

set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX}/${${TOOLCHAIN}} ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)


#############################
# Binary commands for generate binary file
#############################
# Set tools
set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-objcopy${TOOLCHAIN_EXT})
set(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-objdump${TOOLCHAIN_EXT})
set(CMAKE_SIZE ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-size${TOOLCHAIN_EXT})

# Print section sizes
function(print_section_sizes TARGET)
    set(TARGET_OUT ${TARGET}${CMAKE_EXECUTABLE_SUFFIX_CXX})
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_SIZE} ${TARGET_OUT})
endfunction()

# Create binary file
function(create_binary_output TARGET TYPE)
    if( ${TYPE} STREQUAL "bin")
        SET(OBJTYPE "binary")
    elseif( ${TYPE} STREQUAL "srec")
        SET(OBJTYPE "srec")
    elseif( ${TYPE} STREQUAL "hex")
        SET(OBJTYPE "hex")
    else()
        message(FATAL_ERROR "Unknown output format ${TYPE}") 
    endif()
    set(TARGET_OUT ${TARGET}${CMAKE_EXECUTABLE_SUFFIX_CXX})
    add_custom_target(${TARGET}.${TYPE}
    ${ARGV2} DEPENDS ${TARGET} 
    COMMAND ${CMAKE_OBJCOPY} -O${OBJTYPE} ${TARGET_OUT} ${TARGET}.${TYPE}
    COMMENT "Generating ${TARGET}.${TYPE}"
    )
endfunction()

function(create_signed_binary TARGET)
    set(BIN_FILE  ${TARGET}.bin)
    set(ELF_FILE  ${TARGET}.elf)
    add_custom_target( ${BIN_FILE} ${ARGV1}
        COMMENT "Generate signed ${TARGET}.bin (Secure Boot)"
        DEPENDS ${ELF_FILE}
        COMMAND python3 ${SIGN_CLIENT_PATH}/signclient.py --in_file ${ELF_FILE} --out_file=${BIN_FILE} --keystore ${KEYSTORE} --keyslot ${KEYSLOT} --server ${SERVER} --login ${LOGIN}
        VERBATIM
    )
endfunction()
