cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
project(CliSAT LANGUAGES CXX)

add_executable(CliSAT
        main.cpp
)

target_include_directories(CliSAT PUBLIC
    "${PROJECT_BINARY_DIR}"
)

# statically linked
set(CMAKE_CXX_FLAGS "-Wall -Wextra -static")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
# set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto=auto")
# disable asserts in release builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto=auto -DNDEBUG")
