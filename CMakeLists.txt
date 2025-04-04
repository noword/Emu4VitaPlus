cmake_minimum_required(VERSION 3.12)

set(CMAKE_DEPENDS_USE_COMPILER FALSE)
set(CMAKE_TOOLCHAIN_FILE
    "$ENV{VITASDK}/share/vita.toolchain.cmake"
    CACHE PATH "toolchain file")

include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
include("script/arcade_dat.cmake" REQUIRED)
set(CMAKE_AR "$ENV{VITASDK}/bin/arm-vita-eabi-gcc-ar${TOOL_OS_SUFFIX}" CACHE PATH "archive")
set(CMAKE_RANLIB "$ENV{VITASDK}/bin/arm-vita-eabi-gcc-ranlib${TOOL_OS_SUFFIX}" CACHE PATH "ranlib")

option(WITH_LOG "log ouputting" OFF)

find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

set(CORES
    gpsp
    mgba
    vba_next
    gambatte
    fba_lite
    fbneo
    fbalpha2012
    snes9x2002
    snes9x2005
    snes9x2010
    fceumm
    nestopia
    genesis_plus_gx
    genesis_plus_gx_wide
    picodrive
    mednafen_pce_fast
    mednafen_supergrafx
    mednafen_ngp
    mednafen_wswan
    pcsx_rearmed
    stella2014
    prosystem
    atari800

    # dosbox_svn
    dosbox_pure
    mame2003_plus
    mame2003
    vecx
    uae4arm
    fuse
    neocd
    supafaust

    # hatari
    fbneo_xtreme
    mame2003_xtreme
    chimerasnes
    tgbdual
    nekop2
    fmsx
    bluemsx
    nekop2kai
    vice
    px68k
)

set(BUILD_LIST "All" "Arch" ${CORES})
string(REPLACE "|" " " BUILD_LIST_STR ${BUILD_LIST})
set(BUILD "Arch" CACHE STRING ${BUILD_LIST_STR})
set_property(CACHE BUILD PROPERTY STRINGS ${BUILD_LIST})

string(STRIP ${BUILD} BUILD)

if(NOT ${BUILD} IN_LIST BUILD_LIST)
    message(FATAL_ERROR "valid BUILD value: ${BUILD_LIST}")
endif()

if(BUILD STREQUAL "Arch")
    set(BUILD_SELF ON)
endif()

project("Emu4Vita++")

if(WITH_LOG)
    set(LOG_LEVEL LOG_LEVEL_DEBUG)
else()
    set(LOG_LEVEL LOG_LEVEL_INFO)
endif()

set(CMAKE_C_FLAGS "-marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard \
-DVITA -D__vita__ \
-mword-relocations -fno-optimize-sibling-calls -fsingle-precision-constant \
-fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables \
-ffast-math -ftree-vectorize -O3 \
-fdata-sections -ffunction-sections \
-DLOG_LEVEL=${LOG_LEVEL}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -std=gnu++17 -fno-exceptions")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-q,-no-enum-size-warning,--as-needed,--gc-sections") # ,--no-undefined,--as-needed,--hash-style=gnu")

set(OUT_PATH "${CMAKE_BINARY_DIR}/out")
file(MAKE_DIRECTORY ${OUT_PATH})

add_subdirectory(deps)
add_subdirectory(share)
add_subdirectory(frontend)

set(APP_VER "0.26")
set(SFO_APP_VER "00.26")

foreach(core ${CORES})
    if(BUILD STREQUAL "All" OR BUILD STREQUAL "Arch" OR BUILD STREQUAL ${core})
        add_subdirectory(apps/${core})
    endif()
endforeach()

if(BUILD STREQUAL "All" OR BUILD STREQUAL "Arch")
    add_subdirectory(arch)
endif()