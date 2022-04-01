SET(CMAKE_SYSTEM_NAME Linux)
#SET(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_SYSTEM_PROCESSOR arm)

# specify the cross compiler
set(toolchain armv6-rpi-linux-gnueabihf)
SET(CMAKE_SYSROOT /home/christophe/x-tools/${toolchain}/${toolchain}/sysroot/)
set(tools /home/christophe/x-tools/${toolchain})
SET(CMAKE_C_COMPILER ${tools}/bin/${toolchain}-gcc)
SET(CMAKE_CXX_COMPILER ${tools}/bin/${toolchain}-g++)

#set(CMAKE_C_FLAGS "-mfloat-abi=softfp")
#set(CMAKE_CXX_FLAGS "-mfloat-abi=softfp")

# where is the target environment
#SET(CMAKE_FIND_ROOT_PATH /home/chris/x-tools/arm-unknown-linux-gnueabi/arm-unknown-linux-gnueabi/sysroot/)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
#SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
