cmake_minimum_required(VERSION 3.12)

set(PUBLIC_PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pkg)

file(GLOB_RECURSE PKG_FILES ${PUBLIC_PKG_DIR}/*)
set(PUBLIC_PKG_FILES "")

foreach(file ${PKG_FILES})
    file(RELATIVE_PATH rel ${PUBLIC_PKG_DIR} ${file})
    list(APPEND PUBLIC_PKG_FILES FILE ${file} ${rel})
endforeach()

SET(PUBLIC_PKG_FILES "${PUBLIC_PKG_FILES}" CACHE INTERNAL "PUBLIC_PKG_FILES")

add_custom_command(
    OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.h

    COMMAND python export_share.py
    COMMAND mv language_define.h ${CMAKE_CURRENT_SOURCE_DIR}/source
    COMMAND mv language_define.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../script

    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/language.json
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/export_share.py
)

add_custom_target(
    LANG_SHARE
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.h)

add_compile_definitions(_APP_VER_STR="${APP_VER}")

file(GLOB SRC source/*.cpp)

if(NOT ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.cpp IN_LIST SRC)
    list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.cpp)
endif()

include_directories(
    ../deps/libretro-common/include
    ../deps/7-Zip/C
    ../deps/lz4/lib)

add_library(share STATIC ${SRC})
add_dependencies(share LANG_SHARE)