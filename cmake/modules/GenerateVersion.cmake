# This script generates the version.h containing project version
# information. It is meant to be run at build time by running CMake as a target.
include(Version)
configure_file(
        ${CMAKE_SOURCE_DIR}/templates/version.h.template
        ${CMAKE_BINARY_DIR}/version.h
)
