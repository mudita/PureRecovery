add_subdirectory(luafilesystem)
add_subdirectory(lsqlite3)
add_subdirectory(ltar)

add_library(lualibs INTERFACE)
target_include_directories(lualibs INTERFACE .)
target_link_libraries(lualibs INTERFACE luafilesystem lsqlite3 ltar)
