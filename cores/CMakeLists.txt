cmake_minimum_required(VERSION 3.12)

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${CORE})
set(CORE_MAKEFILE_NAME Makefile)
set(CORE_ARGS platform=vita EXTERNAL_ZLIB=1)

if(CORE STREQUAL "gpsp")
elseif(CORE STREQUAL "vba_next")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vba-next)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile.libretro)
elseif(CORE STREQUAL "gambatte")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gambatte-libretro)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "fba_lite")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-fba-lite)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/makefile.libretro)
    set(CORE_ARGS ${CORE_ARGS} HAVE_NEON=1)
elseif(CORE STREQUAL "fbneo")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-fbneo/src/burner/libretro)
    set(CORE_ARGS ${CORE_ARGS} STATIC_LINKING=1)
elseif(CORE STREQUAL "fbalpha2012")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fbalpha2012/svn-current/trunk)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/makefile.libretro)
elseif(CORE STREQUAL "snes9x2002")
elseif(CORE STREQUAL "snes9x2005_plus")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/snes9x2005)
    set(CORE_ARGS ${CORE_ARGS} USE_BLARGG_APU=1)
elseif(CORE STREQUAL "snes9x2010")
elseif(CORE STREQUAL "fceumm")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-fceumm)
elseif(CORE STREQUAL "nestopia")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nestopia/libretro)
elseif(CORE STREQUAL "genesis_plus_gx")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Genesis-Plus-GX)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "genesis_plus_gx_wide")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Genesis-Plus-GX-Wide)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "picodrive")
    set(CORE_MAKEFILE_NAME Makefile.libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile.libretro)
elseif(CORE STREQUAL "mednafen_pce_fast")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/beetle-pce-fast-libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
    set(CORE_ARGS ${CORE_ARGS} SYSTEM_ZLIB=1)
elseif(CORE STREQUAL "mednafen_supergrafx")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/beetle-supergrafx-libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
    set(CORE_ARGS ${CORE_ARGS} SYSTEM_ZLIB=1)
elseif(CORE STREQUAL "mednafen_ngp")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/beetle-ngp-libretro)
elseif(CORE STREQUAL "mednafen_wswan")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/beetle-wswan-libretro)
elseif(CORE STREQUAL "pcsx_rearmed")
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile.libretro)
    set(CORE_ARGS ${CORE_ARGS} DYNAREC=ari64)
elseif(CORE STREQUAL "stella2014")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stella2014-libretro)
elseif(CORE STREQUAL "prosystem")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/prosystem-libretro)
elseif(CORE STREQUAL "atari800")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-atari800)
elseif(CORE STREQUAL "dosbox_pure")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dosbox-pure)
elseif(CORE STREQUAL "mame2003_plus")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mame2003-plus-libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
    set(CORE_ARGS ${CORE_ARGS} SPLIT_UP_LINK=1)
elseif(CORE STREQUAL "mame2003")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mame2003-libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
    set(CORE_ARGS ${CORE_ARGS} SPLIT_UP_LINK=1)
elseif(CORE STREQUAL "vecx")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-vecx)
elseif(CORE STREQUAL "uae4arm")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/uae4arm-libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
elseif(CORE STREQUAL "fuse")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fuse-libretro)
elseif(CORE STREQUAL "neocd")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/neocd_libretro)
elseif(CORE STREQUAL "mednafen_supafaust")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/supafaust)
    set(CORE_ARGS ${CORE_ARGS} USE_BLARGG_APU=1)
elseif(CORE STREQUAL "km_fbneo_xtreme_amped")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/FBNeo-Xtreme-Amped/src/burner/libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
    set(CORE_ARGS ${CORE_ARGS} STATIC_LINKING=1)
elseif(CORE STREQUAL "km_mame2003_xtreme_amped")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mame2003-xtreme)
    set(CORE_ARGS ${CORE_ARGS} SPLIT_UP_LINK=1)
elseif(CORE STREQUAL "chimerasnes")
    set(CORE_ARGS ${CORE_ARGS} USE_BLARGG_APU=1)
elseif(CORE STREQUAL "tgbdual")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tgbdual-libretro)
elseif(CORE STREQUAL "nekop2")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-meowPC98/libretro)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "fmsx")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fmsx-libretro)
elseif(CORE STREQUAL "bluemsx")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/blueMSX-libretro)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "np2kai")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/NP2kai/sdl)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "vice")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vice-libretro)
    set(LIBRETRO_LIBRARY ${CORE_DIR}/vice_x64_libretro_vita.a)
elseif(CORE STREQUAL "px68k")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/px68k-libretro)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
else()
    message(FATAL_ERROR "unknown core: " ${CORE})
endif()

set(LIBRETRO_ELF libretro_${CORE}.elf)
set(LIBRETRO_SUPRX ${CORE}.suprx CACHE INTERNAL "${CORE}_SUPRX")

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

if(NOT LIBRETRO_LIBRARY)
    set(LIBRETRO_LIBRARY ${CORE_DIR}/${CORE}_libretro_vita.a)
endif()

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

add_dependencies(${LIBRETRO_ELF} ${CORE_TAG})

target_link_libraries(${LIBRETRO_ELF}
    ${LIBRETRO_LIBRARY}
    retro_comm
    ass
    pthread
    7zip
    zlibstatic
    FLAC
    ogg
    stdc++
    c
    m
    gcc
    k
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
    SceNet_stub
    SceProcessmgr_stub
    SceRtc_stub
)

# set_target_properties(${LIBRETRO_ELF} PROPERTIES COMPILE_FLAGS "-DSCE_LIBC_SIZE=0xA00000")
set_target_properties(${LIBRETRO_ELF} PROPERTIES LINK_FLAGS "-nostdlib")

# set(VITA_MAKE_FSELF_FLAGS "-a 0x2800000000000001")
vita_create_self(${LIBRETRO_SUPRX} ${LIBRETRO_ELF} UNSAFE CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/core.yml)

if(CREATE_STUB)
    vita_create_stubs(.. ${LIBRETRO_ELF} ${CMAKE_CURRENT_SOURCE_DIR}/core.yml)
endif()
