cmake_minimum_required(VERSION 3.27)
project(chip_chap
        VERSION 0.0.1
        DESCRIPTION "a CHIP-8 emulator"
        HOMEPAGE_URL "n/a"
        LANGUAGES C CXX
)

if ("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed")
endif ()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build the unit tests" ON)

# don't allow the usage of compiler-specific extensions to improve portability
set(CMAKE_CXX_EXTENSIONS OFF)

# activate LTO (link-time optimizations) to increase the chance of catching ODR violations
include(cmake/lto.cmake)
try_enable_link_time_optimizations()

include(cmake/FasterBuild.cmake) # ccache
include(cmake/system_link.cmake)

include(dependencies.cmake)
find_packages()

add_subdirectory(vendor)
add_subdirectory(src)
