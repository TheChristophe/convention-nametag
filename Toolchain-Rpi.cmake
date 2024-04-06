SET(CMAKE_SYSTEM_NAME Linux)
#SET(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_SYSTEM_PROCESSOR arm)

# specify the cross compiler
set(TOOLCHAIN aarch64-rpi3-linux-gnu)
SET(CMAKE_SYSROOT /build/x-tools/${TOOLCHAIN}/${TOOLCHAIN}/sysroot/)
set(TOOLS /build/x-tools/${TOOLCHAIN})
SET(CMAKE_C_COMPILER ${TOOLS}/bin/${TOOLCHAIN}-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLS}/bin/${TOOLCHAIN}-g++)

#set(CMAKE_C_FLAGS "-mfloat-abi=softfp")
#set(CMAKE_CXX_FLAGS "-mfloat-abi=softfp")

# where is the target environment
#SET(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
#SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
