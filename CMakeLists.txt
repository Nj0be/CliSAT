cmake_minimum_required(VERSION 3.30)
project(CliSAT)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(BitGraph)

add_executable(CliSAT main.cpp
        bitscan_benchmark.h
        custom_bitset.h
        custom_graph.h
        BBMC.h
        BBMCR.h
        CliSAT.h
        BBMC.cpp
)

target_link_libraries(CliSAT graph bitscan utils)
target_include_directories(CliSAT PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/BitGraph/src"
)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=x86-64-v4 -flto")
