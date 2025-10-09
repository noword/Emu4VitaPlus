cmake_minimum_required(VERSION 3.12)

set(PUBLIC_PKG_FILES "")
collect_files(PUBLIC_PKG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/pkg)
SET(PUBLIC_PKG_FILES "${PUBLIC_PKG_FILES}" CACHE INTERNAL "PUBLIC_PKG_FILES")

add_custom_command(
    OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pkg/assets/language.zbin
    ${CMAKE_CURRENT_SOURCE_DIR}/pkg/assets/translation.zbin

    COMMAND python export_share.py
    COMMAND mv language_define.h ${CMAKE_CURRENT_SOURCE_DIR}/source
    COMMAND mv language_define.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source
    COMMAND mv language.zbin ${CMAKE_CURRENT_SOURCE_DIR}/pkg/assets
    COMMAND mv translation.zbin ${CMAKE_CURRENT_SOURCE_DIR}/pkg/assets

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../script

    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/language.json
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/translation.json
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/export_share.py
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/language.py
)

add_custom_target(
    LANG_SHARE
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pkg/assets/language.zbin
    ${CMAKE_CURRENT_SOURCE_DIR}/pkg/assets/translation.zbin
)

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