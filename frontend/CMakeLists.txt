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
    ../deps/7-Zip/C
    ../deps/rcheevos/include)

file(GLOB SRC
    source/*.c
    source/*.cpp
    source/ui/*.cpp
    source/emu/*.cpp
    source/audio/*.cpp)

add_custom_command(OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_frontend.h

    COMMAND python export_frontend.py
    COMMAND mv language_frontend.h ${CMAKE_CURRENT_SOURCE_DIR}/source/language_frontend.h

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../script

    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/language.json
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/translation.json
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/export_frontend.py
)

add_custom_target(
    LANG_FRONTEND
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_frontend.h
)

add_compile_definitions(
    APP_VER_STR="${APP_VER}"
    SCE_LIBC_SIZE=0xA00000)

add_library(frontend OBJECT ${SRC})

add_custom_target(FILE_TOUCHER COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/source/ui/tab_about.cpp)
add_dependencies(frontend FILE_TOUCHER LANG_FRONTEND)