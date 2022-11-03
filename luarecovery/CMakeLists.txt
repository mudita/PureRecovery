add_library(luarecovery STATIC)

target_sources(luarecovery PRIVATE
        src/common.c
        src/lrecovery.c
        src/lgui.c
        src/lsys.c
        src/lkeyboard.c
        src/lbootctrl.c
        )

target_include_directories(luarecovery PUBLIC include PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(luarecovery PRIVATE lua hal-common pure-core recovery)

# Define where LUA should search for the external modules:
# /system/scripts
target_compile_definitions(lua
        PRIVATE
        LUA_PATH_DEFAULT="/system/scripts/?.lua\\\;/system/scripts/?/?.lua\\\;/system/scripts/?/init.lua"
        LUA_CPATH_DEFAULT=""
        )
# 'luafilesystem' library needs custom syscalls definitions
target_link_libraries(luafilesystem PRIVATE system)