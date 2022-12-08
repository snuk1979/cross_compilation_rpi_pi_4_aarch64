# the name of the target operating system
include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# which compilers to use for C and C++
cmake_force_c_compiler(/usr/bin/aarch64-linux-gnu-gcc GNU)
cmake_force_cxx_compiler(/usr/bin/aarch64-linux-gnu-g++ GNU)
# set(CMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc GNU)
# set(CMAKE_CXX_COMPILER=/usr/bin/aarch64-linux-gnu-g++ GNU)

set(CMAKE_SYSROOT  $ENV{RASPBIAN_ROOTFS})
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT}) 
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)

set(CMAKE_LIBRARY_PATH=$ENV{RASPBIAN_ROOTFS}/usr/local/lib)
set(CMAKE_INSTALL_PREFIX=$ENV{RASPBIAN_ROOTFS}/usr/local)
set(CMAKE_AR=/usr/bin/aarch64-linux-gnu-ar)
set(CMAKE_C_COMPILER_AR=/usr/bin/aarch64-linux-gnu-gcc-ar)
set(CMAKE_C_COMPILER_RANLIB=/usr/bin/aarch64-linux-gnu-gcc-ranlib)
set(CMAKE_LINKER=/usr/bin/aarch64-linux-gnu-ld)
set(CMAKE_NM=/usr/bin/aarch64-linux-gnu-nm)
set(CMAKE_OBJCOPY=/usr/bin/aarch64-linux-gnu-objcopy)
set(CMAKE_OBJDUMP=/usr/bin/aarch64-linux-gnu-objdump)
set(CMAKE_RANLIB=/usr/bin/aarch64-linux-gnu-ranlib)
set(CMAKE_STRIP=/usr/bin/aarch64-linux-gnu-strip)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
