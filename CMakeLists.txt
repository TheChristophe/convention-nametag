cmake_minimum_required(VERSION 3.16)
project(convention_nametag)

set(CMAKE_CXX_STANDARD 20)
include(CheckIncludeFileCXX)
#set(CMAKE_VERBOSE_MAKEFILE on)

include_directories(SYSTEM ${CMAKE_SYSROOT}/usr/include/freetype2)
include_directories(SYSTEM include)
include_directories(SYSTEM include/cpptoml/include)
include_directories(SYSTEM include/json/include)
include_directories(SYSTEM include/uWebSockets/src)
include_directories(SYSTEM include/uWebSockets/uSockets/src)
include_directories(source)
include_directories(source/drawers)
include_directories(source/net)
include_directories(source/util)
include_directories(source/wrappers)

set(CMAKE_CXX_FLAGS "-pipe -Winvalid-pch -fexceptions -Wno-psabi -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -ggdb -Wall -Wextra -Wformat -Wfloat-equal -Wshadow -Wpointer-arith -Wcast-qual  -Wno-unused-parameter -Wfatal-errors -DDEBUGGING -Wsuggest-final-types -Wpedantic -Wnull-dereference -fno-omit-frame-pointer -fdiagnostics-color -ftemplate-depth=128 -fconstexpr-depth=128 -ftemplate-backtrace-limit=8 -Wreorder -Wold-style-cast -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=armv6 -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard")

set(SOURCE_FILES
        source/net/server.cpp
        source/net/server.hpp
        source/wrappers/driver.hpp
        source/wrappers/hardware.cpp
        source/wrappers/hardware.hpp
        source/main.cpp
        source/video/decoder.hpp
        source/video/videoDecoder.cpp
        source/video/videoDecoder.hpp
        source/wrappers/sh1106.cpp
        source/wrappers/ssd1322.cpp
        source/wrappers/ssd1305.cpp
        source/video/videoPlayer.cpp
        source/video/videoPlayer.hpp
        source/video/idleDecoder.cpp
        source/video/idleDecoder.hpp)

add_executable(nametag ${SOURCE_FILES})

CHECK_INCLUDE_FILE_CXX("bcm2835.h" HAVE_BCM2835 "-I${PREFIX}/include")
include_directories(SYSTEM ${CMAKE_SYSROOT}/opt/vc/include)

target_link_directories(nametag PUBLIC ${CMAKE_SYSROOT}/opt/vc/lib)
target_link_libraries(nametag PUBLIC bcm2835 rt z)

find_library(USOCKETS_LIB uSockets.a HINT include/uWebSockets/uSockets)
target_link_libraries(nametag PUBLIC ${USOCKETS_LIB})

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY avdevice)

find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h)
find_library(SWRESAMPLE_LIBRARY swresample)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)


target_include_directories(nametag PUBLIC ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR} ${SWRESAMPLE_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR})
target_link_libraries(nametag PUBLIC ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY} ${SWRESAMPLE_LIBRARY} ${SWSCALE_LIBRARY} -static-libgcc -static-libstdc++ -static)
