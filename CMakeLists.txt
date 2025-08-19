cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)

project(CliSAT LANGUAGES CXX)

add_executable(CliSAT)

# Add main.cpp as a normal source
target_sources(CliSAT
    PRIVATE
        main.cpp
)

# Add your modules as a FILE_SET (CMake 3.28+ feature)
target_sources(CliSAT
    PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
            custom_bitset.ixx
            custom_graph.ixx
            AMTS.ixx
            bitscan_benchmark.ixx
            BBMC.ixx
            BBMCR.ixx
            CliSAT.ixx
            sorting.ixx
            coloring.ixx
)

target_include_directories(CliSAT PUBLIC
    "${PROJECT_BINARY_DIR}"
)

# statically linked
set(CMAKE_CXX_FLAGS "-Wall -Wextra -fmodules -static")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
# set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto=auto")
# disable asserts in release builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto=auto -DNDEBUG")
