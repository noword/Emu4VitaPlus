cmake_minimum_required(VERSION 3.12)

set(CORE_ARGS platform=vita EXTERNAL_ZLIB=1)
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${CORE})
set(CORE_MAKEFILE_NAME Makefile)

if(CORE STREQUAL "gpsp")
elseif(CORE STREQUAL "vba_next")
elseif(CORE STREQUAL "gambatte")
elseif(CORE STREQUAL "fba_lite")
    set(CORE_ARGS ${CORE_ARGS} HAVE_NEON=1)
elseif(CORE STREQUAL "fbneo")
    set(CORE_ARGS ${CORE_ARGS} STATIC_LINKING=1)
elseif(CORE STREQUAL "fbalpha2012")
elseif(CORE STREQUAL "snes9x2002")
elseif(CORE STREQUAL "snes9x2005")
    set(CORE_ARGS ${CORE_ARGS} USE_BLARGG_APU=1)
elseif(CORE STREQUAL "snes9x2010")
elseif(CORE STREQUAL "fceumm")
elseif(CORE STREQUAL "nestopia")
elseif(CORE STREQUAL "genesis_plus_gx")
elseif(CORE STREQUAL "genesis_plus_gx_wide")
elseif(CORE STREQUAL "picodrive")
elseif(CORE STREQUAL "mednafen_pce_fast")
    set(CORE_ARGS ${CORE_ARGS} SYSTEM_ZLIB=1)
elseif(CORE STREQUAL "mednafen_supergrafx")
    set(CORE_ARGS ${CORE_ARGS} SYSTEM_ZLIB=1)
elseif(CORE STREQUAL "mednafen_ngp")
elseif(CORE STREQUAL "mednafen_wswan")
elseif(CORE STREQUAL "pcsx_rearmed")
    set(CORE_ARGS ${CORE_ARGS} DYNAREC=ari64)
elseif(CORE STREQUAL "stella2014")
elseif(CORE STREQUAL "prosystem")
elseif(CORE STREQUAL "atari800")
elseif(CORE STREQUAL "dosbox_pure")
elseif(CORE STREQUAL "mame2003_plus")
    set(CORE_ARGS ${CORE_ARGS} SPLIT_UP_LINK=1)
elseif(CORE STREQUAL "mame2003")
    set(CORE_ARGS ${CORE_ARGS} SPLIT_UP_LINK=1)
elseif(CORE STREQUAL "vecx")
elseif(CORE STREQUAL "uae4arm")
elseif(CORE STREQUAL "fuse")
elseif(CORE STREQUAL "neocd")
elseif(CORE STREQUAL "supafaust")
    set(CORE_ARGS ${CORE_ARGS} USE_BLARGG_APU=1)
elseif(CORE STREQUAL "fbneo_xtreme")
    set(CORE_ARGS ${CORE_ARGS} STATIC_LINKING=1)
elseif(CORE STREQUAL "mame2003_xtreme")
    set(CORE_ARGS ${CORE_ARGS} SPLIT_UP_LINK=1)
elseif(CORE STREQUAL "chimerasnes")
    set(CORE_ARGS ${CORE_ARGS} USE_BLARGG_APU=1)
elseif(CORE STREQUAL "tgbdual")
elseif(CORE STREQUAL "nekop2")
elseif(CORE STREQUAL "fmsx")
elseif(CORE STREQUAL "bluemsx")
elseif(CORE STREQUAL "nekop2kai")
elseif(CORE STREQUAL "vice")
elseif(CORE STREQUAL "px68k")
else()
    message(FATAL_ERROR "unknown core: " ${CORE})
endif()

set(LIBRETRO_ELF libretro_${CORE}.elf)
set(LIBRETRO_SUPRX ${CORE}_libretro.suprx CACHE INTERNAL "LIBRETRO_SUPRX")

set(CORE_TAG ${CORE}_build)
add_custom_target(
    ${CORE_TAG} ALL
    COMMAND make -f ${CORE_MAKEFILE_NAME} ${CORE_ARGS} -j4
    WORKING_DIRECTORY ${CORE_DIR})

add_library(${CORE} STATIC IMPORTED)
set_property(
    TARGET ${CORE}
    APPEND
    PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)

set(LIBRETRO_LIBRARY ${CORE_DIR}/${CORE}_libretro_vita.a)
set_target_properties(${CORE} PROPERTIES IMPORTED_LOCATION "${LIBRETRO_LIBRARY}")
add_dependencies(${CORE} ${CORE_TAG})

include_directories(
    ../frontend/source
    ../frontend/source/emu
    ../frontend/source/audio
    ../frontend/source/core_spec
    ../deps/libretro-common/include
    ../deps/7-Zip/C
    ../share/source)

add_executable(${LIBRETRO_ELF} core.cpp sbrk.c)

set(VITA_MAKE_FSELF_FLAGS "-a 0x2800000000000001")
target_link_libraries(${LIBRETRO_ELF}
    ${LIBRETRO_LIBRARY}
    retro_comm
    pthread
    7zip
    zlibstatic
    FLAC
    ogg
    SceLibKernel_stub
    SceIofilemgr_stub
    SceDisplay_stub
    SceCtrl_stub
    ScePower_stub
    SceVideodec_stub
    SceAppMgr_stub
    SceSysmem_stub
    SceSysmodule_stub
    SceShellSvc_stub
    SceThreadmgrForDriver_stub
    SceKernelThreadMgr_stub
    SceMotion_stub
    SceFiber_stub
)

set_target_properties(${LIBRETRO_ELF} PROPERTIES COMPILE_FLAGS "-Wno-main -DSCE_LIBC_SIZE=0xA00000")

vita_create_self(${LIBRETRO_SUPRX} ${LIBRETRO_ELF} STRIPPED UNSAFE CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/core.yml)

if(CREATE_STUB)
    vita_create_stubs(stub_${CORE} ${LIBRETRO_ELF} ${CMAKE_CURRENT_SOURCE_DIR}/core.yml)
endif()
