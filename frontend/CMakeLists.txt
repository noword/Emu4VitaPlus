cmake_minimum_required(VERSION 3.12)

include_directories(
    source
    source/ui
    source/audio
    source/emu
    source/core_spec
    ../share/source
    ../deps/libretro-common/include
    ../deps/libvita2d/libvita2d/include
    ../deps/libvita2d_ext/libvita2d_ext/include
    ../deps/simpleini
    ../deps/minizip-ng
    ../deps/7-Zip/C)

file(GLOB SRC
    source/*.c
    source/*.cpp
    source/ui/*.cpp
    source/emu/*.cpp
    source/audio/*.cpp)

if(NOT ${CMAKE_CURRENT_SOURCE_DIR}/source/language_frontend.cpp IN_LIST SRC)
    list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/language_frontend.cpp)
endif()

add_compile_definitions(SCE_LIBC_SIZE=0xA00000)
add_library(frontend OBJECT ${SRC})

add_custom_target(FILE_TOUCHER
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/source/ui/tab_about.cpp)
add_dependencies(frontend FILE_TOUCHER)