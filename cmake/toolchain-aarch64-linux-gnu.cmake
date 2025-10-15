# Cross-compilation toolchain for 64-bit ARM Linux (AArch64)
# Works with Arch's aarch64-linux-gnu-gcc / g++

# Target system information
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the cross compilers
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_ASM_COMPILER aarch64-linux-gnu-as)

# Optional: specify sysroot if you have a target root filesystem
# (e.g. extracted from a Raspberry Pi, Jetson, or other ARM board)
# set(CMAKE_SYSROOT /path/to/aarch64/sysroot)
# set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})

# Ensure cmake searches in the sysroot for headers and libs first
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set compiler flags (tune for your target if known)
set(CMAKE_C_FLAGS_INIT "-O2 -pipe")
set(CMAKE_CXX_FLAGS_INIT "-O2 -pipe")

# Use static linking or dynamic depending on your target
# set(CMAKE_EXE_LINKER_FLAGS "-static")

# Disable running executables during configuration (they won't run on host)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
