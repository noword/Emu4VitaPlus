cmake_minimum_required(VERSION 3.10)

set(CORE_NAME "uae4arm")
set(CORE_SHORT_NAME "uae4arm")
set(CORE_FULL_NAME "uae4arm")
set(CONSOLE "AMIGA")

if(TARGET ${CORE_NAME}_build)
  return()
endif()

project(${CORE_NAME})

set(APP_NAME "${CORE_SHORT_NAME}4VitaPlus")
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../cores/uae4arm-libretro)
set(CORE_MAKEFILE_DIR ${CORE_DIR})
set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/Makefile)
set(SFO_TITLE_NAME "Emu4Vita++ (${CORE_NAME})")
set(SFO_TITLE_ID "UAE4VITAP")

set(CORE_ARGS platform=vita EXTERNAL_ZLIB=1)
set(CORE_SOFTWARE, "Amiga")

set(PRIVATE_PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pkg)
file(GLOB_RECURSE PKG_FILES ${PRIVATE_PKG_DIR}/*)
set(PRIVATE_PKG_FILES "")

foreach(file ${PKG_FILES})
  file(RELATIVE_PATH rel ${PRIVATE_PKG_DIR} ${file})
  list(APPEND PRIVATE_PKG_FILES FILE ${file} ${rel})
endforeach()

add_compile_definitions(UAE4ARM_BUILD AMIGA_BUILD)

add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/app)
