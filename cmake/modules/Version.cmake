execute_process(COMMAND git rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_REV
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

if ((NOT VERSION_MAJOR MATCHES [0-9]+) OR (NOT VERSION_MINOR MATCHES [0-9]+) OR (NOT VERSION_PATCH MATCHES [0-9]+))
    message(WARNING "Version not set properly! Setting version to 0.0.0! Requires MAJOR.MINOR.PATCH as parameters was: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    set(VERSION_MAJOR "0")
    set(VERSION_MINOR "0")
    set(VERSION_PATCH "0")
else ()
    message(STATUS "Version major: ${VERSION_MAJOR} minor: ${VERSION_MINOR} path: ${VERSION_PATCH} label: ${VERSION_LABEL}")
endif ()

if ((NOT DEFINED VERSION_LABEL) OR (VERSION_LABEL STREQUAL ""))
    set(VERSION_LABEL "")
    set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
else ()
    set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_LABEL})
endif ()

