cmake_minimum_required(VERSION 3.12)

# set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)
set(CMAKE_DEPENDS_USE_COMPILER FALSE)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
include("script/arcade_dat.cmake" REQUIRED)
include("script/collect_files.cmake" REQUIRED)
set(CMAKE_AR "$ENV{VITASDK}/bin/arm-vita-eabi-gcc-ar${TOOL_OS_SUFFIX}" CACHE PATH "archive")
set(CMAKE_RANLIB "$ENV{VITASDK}/bin/arm-vita-eabi-gcc-ranlib${TOOL_OS_SUFFIX}" CACHE PATH "ranlib")

option(WITH_LOG "log ouputting" OFF)
option(LTO "enable gcc lto optimzatior" OFF)

find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

set(CORES
    gpsp
    vba_next
    gambatte
    fba_lite
    fbneo
    fbalpha2012
    snes9x2002
    snes9x2005_plus
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
    mame2000
    mame2003_plus
    mame2003
    vecx
    uae4arm
    fuse
    neocd
    mednafen_supafaust

    # hatari
    km_fbneo_xtreme_amped
    km_mame2003_xtreme_amped
    chimerasnes
    tgbdual
    nekop2
    fmsx
    bluemsx
    np2kai
    vice
    px68k
    mednafen_lynx
    handy
    cap32
    crocods
)

set(CORE_SHORTS
    gpSP
    VBANext
    Gambatte
    FBALite
    FBNeo
    FBA2012
    Snes9x2002
    Snes9x2005Plus
    Snes9x2010
    FCEUmm
    Nestopia
    GenesisPlusGX
    GenesisPlusGXWide
    PicoDrive
    MednafenPCEFast
    MednafenPCESuperGrafx
    MednafenNgp
    MednafenWswan
    PCSXReARMed
    Stella2014
    ProSystem
    Atari800

    # DOSBoxSVN
    DOSBoxPure
    MAME2000
    MAME2003Plus
    MAME2003
    vecx
    uae4arm
    fuse
    neocd
    Supafaust

    # Hatari
    FBNeoXtreme
    MAME2003Xtreme
    ChimeraSNES
    TGBDual
    nekop2
    fMSX
    blueMSX
    np2kai
    vice
    px68k
    Lynx
    Handy
    Caprice32
    CrocoDS)

set(CONSOLES
    GBA
    GBA
    GBC
    ARC
    ARC
    ARC
    SNES
    SNES
    SNES
    NES
    NES
    MD
    MD
    MD
    PCE
    PCE
    NGP
    WSC
    PS
    ATARI2600
    ATARI7800
    ATARI5200
    DOS
    ARC
    ARC
    ARC
    VECTREX
    AMIGA
    ZXS
    NEOGEOCD
    SNES
    ARC
    ARC
    SNES
    GBC
    PC98
    MSX
    MSX
    PC98
    C64
    X68000
    LYNX
    LYNX
    CPC
    CPC)

set(TITLE_IDS
    GPSP4VITA
    VBANEXT4V
    GAMBATTEV
    FBALITE4V
    FBNEO4VIT
    FBA20124V
    SNES024VT
    SNES054VT
    SNES104VT
    FCEUMM4VT
    NESTP4VIT
    GENESIS4V
    GENWIDE4V
    PICODR4VT
    MPCEF4VIT
    MPCESG4VT
    MNGP4VITA
    WSWAN4VIT
    PCSX4V001
    STELLA4VI
    PROSYS4PV
    A8004VITA
    DOSPURE4V
    MAME2K4VI
    MAME03P4V
    MAME2K34V
    VECX4VITA
    UAE4VITAP
    FUSE4VITA
    NEOCD4VIT
    SUPA4VITA
    FBNEOX4VP
    MAME2K3XV
    CHIMERA4V
    TGBD4VITA
    NEKOP2VIT
    FMSX4VITA
    BLUEMSX4V
    NP2KAIVIT
    VICE4VITA
    PX68K4VIT
    LYNX4VITA
    HANDY4VIT
    CAP324VIT
    CROCODS4V
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

set(APP_VER "0.47")
set(SFO_APP_VER "00.47")

if(LTO)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -fno-fat-lto-objects")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -fno-fat-lto-objects")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")

    if(EXISTS $ENV{VITASDK}/lib/gcc/arm-vita-eabi/10.3.0/liblto_plugin-0.dll)
        set(CMAKE_STATIC_LINKER_FLAGS "--plugin=$ENV{VITASDK}/lib/gcc/arm-vita-eabi/10.3.0/liblto_plugin-0.dll")
    else()
        set(CMAKE_STATIC_LINKER_FLAGS "--plugin=$ENV{VITASDK}/lib/gcc/arm-vita-eabi/10.3.0/liblto_plugin-0.so")
    endif()
endif()

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/libs)

add_subdirectory(deps)
add_subdirectory(share)
add_subdirectory(frontend)

list(LENGTH CORES length)
math(EXPR length "${length} - 1")

foreach(index RANGE ${length})
    list(GET CONSOLES ${index} CONSOLE)
    list(GET CORES ${index} CORE)
    list(GET CORE_SHORTS ${index} CORE_SHORT)
    list(GET TITLE_IDS ${index} SFO_TITLE_ID)

    if(BUILD STREQUAL "All" OR BUILD STREQUAL "Arch" OR BUILD STREQUAL ${CORE})
        add_subdirectory(cores cores/${CORE})
        add_subdirectory(apps apps/${CORE})
    endif()
endforeach()

if(BUILD STREQUAL "All" OR BUILD STREQUAL "Arch")
    add_subdirectory(arch)
endif()