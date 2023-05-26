add_library(hal STATIC)

target_sources(hal
    PRIVATE
        system.c
        charger.c
    PUBLIC
        include/hal/system.h
)

target_include_directories(hal
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(hal
    PRIVATE
        hal-common
        system)
