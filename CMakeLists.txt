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

# Compiler warnings
if (MSVC)
    target_compile_options(CliSAT PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(CliSAT PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Debug and Release configurations
target_compile_options(CliSAT PRIVATE
    $<$<CONFIG:Debug>:
        $<$<CXX_COMPILER_ID:MSVC>:/Zi>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g>
    >
    $<$<CONFIG:Release>:
        $<$<CXX_COMPILER_ID:MSVC>:/O2 /GL /DNDEBUG>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O3 -flto=auto -DNDEBUG>
    >
)

# Enable LTO and lld linker for Clang
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Clang detected: enabling LTO and lld")

    # For Release builds: use lld when LTO is active
    target_link_options(CliSAT PRIVATE
            $<$<CONFIG:Release>:-fuse-ld=lld>
    )
endif()

# Static linking
if (STATIC AND NOT MSVC)
    message(STATUS "Static Linking enabled (non-MSVC)")
    target_link_options(CliSAT PRIVATE -static -static-libgcc -static-libstdc++)
endif()