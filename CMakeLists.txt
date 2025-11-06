cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
project(CliSAT LANGUAGES CXX)

option(STATIC "Enable static linking of libraries" OFF)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(CliSAT
    src/main.cpp
    src/CliSAT.cpp
)

# add CLI11 library for args parsing
add_subdirectory(third_party/CLI11)
target_link_libraries(CliSAT PRIVATE CLI11::CLI11)

# statically linked
target_compile_options(CliSAT PRIVATE -Wall -Wextra)

if(STATIC)
    message(STATUS "Static Linking enabled")
    target_link_options(CliSAT PRIVATE -static -static-libgcc -static-libstdc++)
endif()

target_compile_options(CliSAT PRIVATE
    $<$<CONFIG:Debug>:-g>
)

# disable asserts in release builds
target_compile_options(CliSAT PRIVATE
    # $<$<CONFIG:Release>:-O3 -flto=auto -march=native -DNDEBUG>
    $<$<CONFIG:Release>:-O3 -flto=auto -DNDEBUG>
)
