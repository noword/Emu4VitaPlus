cmake_minimum_required(VERSION 3.10)

set(CORE_NAME "mame2003_plus")
set(CORE_SHORT_NAME "MAME2003Plus")
set(CORE_FULL_NAME "MAME 2003 Plus")
set(CONSOLE "ARC")

if(TARGET ${CORE_NAME}_build)
  return()
endif()

project(${CORE_NAME})

set(APP_NAME "${CORE_SHORT_NAME}4VitaPlus")
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../cores/mame2003-plus-libretro)
set(CORE_MAKEFILE_DIR ${CORE_DIR})
set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/Makefile)
set(SFO_TITLE_NAME "Emu4Vita++ (${CORE_FULL_NAME})")
set(SFO_TITLE_ID "MAME03P4V")

set(CORE_ARGS platform=vita EXTERNAL_ZLIB=1 SPLIT_UP_LINK=1)
set(CORE_SOFTWARE "Arcade")

set(PRIVATE_PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pkg)
file(GLOB_RECURSE PKG_FILES ${PRIVATE_PKG_DIR}/*)
set(PRIVATE_PKG_FILES "")

foreach(file ${PKG_FILES})
  file(RELATIVE_PATH rel ${PRIVATE_PKG_DIR} ${file})
  list(APPEND PRIVATE_PKG_FILES FILE ${file} ${rel})
endforeach()

add_compile_definitions(MAME2003PLUS_BUILD ARC_BUILD)

add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/app)