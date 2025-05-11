cmake_minimum_required(VERSION 3.10)

# sdl/taskmng.c
# line 109:
# #if !defined(__LIBRETRO__)
# int mx = 320, my = 240;
# #endif
set(CORE_NAME "np2kai")
set(CORE_SHORT_NAME "np2kai")
set(CORE_FULL_NAME "Neko Project II kai")
set(CONSOLE "PC98")

if(TARGET ${CORE_NAME}_build)
  return()
endif()

project(${CORE_NAME})

set(APP_NAME "${CORE_SHORT_NAME}4VitaPlus")
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../cores/NP2kai/sdl)
set(CORE_MAKEFILE_DIR ${CORE_DIR})
set(CORE_MAKEFILE_NAME Makefile.libretro)
set(SFO_TITLE_NAME "Emu4Vita++ (${CORE_NAME})")
set(SFO_TITLE_ID "NP2KAIVIT")

set(CORE_ARGS platform=vita EXTERNAL_ZLIB=1)
set(CORE_SOFTWARE, "PC98")

set(PRIVATE_PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pkg)
file(GLOB_RECURSE PKG_FILES ${PRIVATE_PKG_DIR}/*)
set(PRIVATE_PKG_FILES "")

foreach(file ${PKG_FILES})
  file(RELATIVE_PATH rel ${PRIVATE_PKG_DIR} ${file})
  list(APPEND PRIVATE_PKG_FILES FILE ${file} ${rel})
endforeach()

add_compile_definitions(NEKOP2KAI_BUILD PC98_BUILD)

add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/app)
