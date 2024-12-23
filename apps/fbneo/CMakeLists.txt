cmake_minimum_required(VERSION 3.10)

set(CORE_NAME "fbneo")
set(CORE_SHORT_NAME "FBNeo")
set(CORE_FULL_NAME "FinalBurn Neo")
set(CONSOLE "ARC")

if(TARGET ${CORE_NAME}_build)
  return()
endif()

project(${CORE_NAME})

set(APP_NAME "${CORE_SHORT_NAME}4VitaPlus")
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../cores/FBNeo-Xtreme-Amped/src/burner/libretro)
set(CORE_MAKEFILE_DIR ${CORE_DIR})
set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/Makefile)
set(SFO_TITLE_NAME "Emu4Vita++ (${CORE_FULL_NAME})")
set(SFO_TITLE_ID "FBNEO4VIT")

set(CORE_ARGS platform=vita EXTERNAL_ZLIB=1 STATIC_LINKING=1)
set(CORE_SOFTWARE "Arcade (CPS1 2 3, NEO GEO, PGM)")

set(PRIVATE_PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pkg)
file(GLOB_RECURSE PKG_FILES ${PRIVATE_PKG_DIR}/*)
set(PRIVATE_PKG_FILES "")

foreach(file ${PKG_FILES})
  file(RELATIVE_PATH rel ${PRIVATE_PKG_DIR} ${file})
  list(APPEND PRIVATE_PKG_FILES FILE ${file} ${rel})
endforeach()

add_compile_definitions(FBNEO_BUILD ARC_BUILD FBA_BUILD USE_CYCLONE INCLUDE_7Z_SUPPORT _7ZIP_PPMD_SUPPPORT _7ZIP_ST
  LSB_FIRST USE_SPEEDHACKS NO_VIZ _LARGEFILE64_SOURCE=0 _FILE_OFFSET_BITS=32)

add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/app)