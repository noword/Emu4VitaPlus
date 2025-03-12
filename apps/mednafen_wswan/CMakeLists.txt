cmake_minimum_required(VERSION 3.10)

set(CORE_NAME "mednafen_wswan")
set(CORE_SHORT_NAME "MednafenWswan")
set(CORE_FULL_NAME "Mednafen Wswan")
set(CONSOLE "WSC")

if(TARGET ${CORE_NAME}_build)
  return()
endif()

project(${CORE_NAME})

set(APP_NAME "${CORE_SHORT_NAME}4VitaPlus")
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../cores/beetle-wswan-libretro)
set(CORE_MAKEFILE_DIR ${CORE_DIR})
set(CORE_MAKEFILE_NAME Makefile)
set(SFO_TITLE_NAME "Emu4Vita++ (${CORE_FULL_NAME})")
set(SFO_TITLE_ID "WSWAN4VIT")

set(CORE_ARGS platform=vita)
set(CORE_SOFTWARE "Bandai - WonderSwan / Color")

set(PRIVATE_PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pkg)
file(GLOB_RECURSE PKG_FILES ${PRIVATE_PKG_DIR}/*)
set(PRIVATE_PKG_FILES "")

foreach(file ${PKG_FILES})
  file(RELATIVE_PATH rel ${PRIVATE_PKG_DIR} ${file})
  list(APPEND PRIVATE_PKG_FILES FILE ${file} ${rel})
endforeach()

add_compile_definitions(MEDNAFEN_WSW_BUILD WSC_BUILD)

add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/app)
