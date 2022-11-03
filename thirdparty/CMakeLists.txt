add_subdirectory(fatfs)
add_subdirectory(littlefs)
add_subdirectory(seatest)
add_subdirectory(microtar)
add_subdirectory(klib)
add_subdirectory(md5)
add_subdirectory(lwext4)
add_subdirectory(sqlite)
add_subdirectory(segger/rtt)
add_subdirectory(lua)
add_subdirectory(lualibs)

include(FetchContent)

FetchContent_Declare(
        pure-core
        GIT_REPOSITORY git@github.com:mudita/pure-core.git
)
FetchContent_MakeAvailable(pure-core)

