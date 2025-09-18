cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
project(CliSAT LANGUAGES CXX)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(CliSAT
    src/main.cpp
    src/CliSAT.cpp
    src/custom_bitset.cpp
    src/instructions.cpp
)

# statically linked
set(CMAKE_CXX_FLAGS "-Wall -Wextra -static")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
# set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto=auto")
# disable asserts in release builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto=auto -DNDEBUG")
