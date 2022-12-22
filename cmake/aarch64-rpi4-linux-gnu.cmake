# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(TOOLCHAIN_PREFIX aarch64-linux-gnu-)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)

set(CMAKE_SYSROOT  $ENV{RASPBIAN_ROOTFS})
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT}) 
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)

set(CMAKE_LIBRARY_PATH ${CMAKE_SYSROOT}/usr/local/lib)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SYSROOT}/usr/local)
set(CMAKE_AR ${TOOLCHAIN_PREFIX}ar)
set(CMAKE_C_COMPILER_AR ${TOOLCHAIN_PREFIX}gcc-ar)
set(CMAKE_C_COMPILER_RANLIB ${TOOLCHAIN_PREFIX}gcc-ranlib)
set(CMAKE_LINKER ${TOOLCHAIN_PREFIX}ld)
set(CMAKE_NM ${TOOLCHAIN_PREFIX}nm)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump)
set(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}ranlib)
set(CMAKE_STRIP ${TOOLCHAIN_PREFIX}strip)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE arm64)
