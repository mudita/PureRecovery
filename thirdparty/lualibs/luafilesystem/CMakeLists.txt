add_library(luafilesystem src/src/lfs.c)
target_link_libraries(luafilesystem PRIVATE lua)