include(FetchContent)

FetchContent_Declare(
        lsqlite3_src
        URL      http://lua.sqlite.org/index.cgi/zip/lsqlite3_fsl09y.zip # v0.9.5
)
FetchContent_MakeAvailable(lsqlite3_src)
FetchContent_GetProperties(lsqlite3_src)

set_source_files_properties(${lsqlite3_src_SOURCE_DIR}/lsqlite3.c PROPERTIES COMPILE_OPTIONS "-Wno-unused-variable")

add_library(lsqlite3 ${lsqlite3_src_SOURCE_DIR}/lsqlite3.c)
target_link_libraries(lsqlite3 PRIVATE sqlite lua)


