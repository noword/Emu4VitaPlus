cmake_minimum_required(VERSION 3.12)

set(APP_NAME "Emu4VitaPlus")
set(APP_DIR_NAME "Arch")
set(SFO_TITLE_NAME "Emu4Vita++")
set(SFO_TITLE_ID "EMU4VPLUS")
set(PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pkg)

file(GLOB_RECURSE PKG_FILES ${PKG_DIR}/*)
set(PRIVATE_PKG_FILES "")

foreach(file ${PKG_FILES})
    file(RELATIVE_PATH rel ${PKG_DIR} ${file})
    list(APPEND PRIVATE_PKG_FILES FILE ${file} ${rel})
endforeach()

project("Arch")

set(CORE_SHORTS
    gpSP
    mGBA
    VBANext
    Gambatte
    FBALite
    FBNeo
    FBA2012
    Snes9x2002
    Snes9x2005
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
    DOSBoxSVN
    DOSBoxPure
    MAME2003Plus
    MAME2000
    MAME2003
    vecx
    uae4arm
    fuse
    neocd
    Supafaust

    # Hatari
    FBNeoXtreme
    MAME2003Xtreme)

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
    dosbox_svn
    dosbox_pure
    mame2003_plus
    mame2000
    mame2003
    vecx
    uae4arm
    fuse
    neocd
    supafaust

    # hatari
    fbneo_xtreme
    mame2003_xtreme)

set(CORE_EBOOTS "")
set(CORE_FILES "")

list(LENGTH CORES length)
math(EXPR length "${length} - 1")

foreach(index RANGE ${length})
    list(GET CORE_SHORTS ${index} core_short)
    list(GET CORES ${index} core)

    if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
        list(APPEND CORE_EBOOTS "eboot_${core_short}.bin")
    else()
        list(APPEND CORE_EBOOTS "eboot_${core_short}.bin-self")
    endif()

    list(APPEND CORE_FILES FILE "${CMAKE_CURRENT_BINARY_DIR}/../apps/${core}/app/eboot_${core_short}.bin" "eboot_${core_short}.self")
endforeach()

gen_arc_dat(${PKG_DIR})

include_directories(
    source
    ../share/source
    ../deps/libvita2d/libvita2d/include
    ../deps/simpleini)

file(GLOB SRC source/*.cpp)

add_compile_definitions(
    APP_NAME_STR="${APP_NAME}"
    APP_NAME_EXT_STR=${APP_NAME_EXT}
    APP_VER_STR="${APP_VER}"
    APP_TITLEID=${SFO_TITLE_ID}
    APP_DIR_NAME="${APP_DIR_NAME}"
    APP_TITLE_NAME="${SFO_TITLE_NAME}"
    CORE_SOFTWARE=${CORE_SOFTWARE})

add_executable(${APP_NAME}.elf ${SRC})

target_link_libraries(
    ${APP_NAME}.elf
    share
    vita2d
    imgui_vita2d
    SimpleIni
    png
    jpeg
    z
    pthread
    ScePvf_stub
    ScePgf_stub
    SceDisplay_stub
    SceGxm_stub
    SceCtrl_stub
    SceTouch_stub
    ScePower_stub
    SceAudio_stub
    SceRtc_stub
    SceCommonDialog_stub
    SceSysmodule_stub
    SceAppUtil_stub
    SceAppMgr_stub
    SceShellSvc_stub
    SceMotion_stub
    SceHid_stub
    SceFiber_stub)

set(VITA_MAKE_FSELF_FLAGS "-a 0x2800000000000001")
vita_create_self(eboot_${APP_NAME}.bin ${APP_NAME}.elf STRIPPED REL_OPTIMIZE)

if(WITH_LOG)
    set(VPK_NAME ${APP_NAME}_v${APP_VER}_log.vpk)
else()
    set(VPK_NAME ${APP_NAME}_v${APP_VER}.vpk)
endif()

set(VITA_MKSFOEX_FLAGS -d ATTRIBUTE2=12)
vita_create_vpk(
    ${VPK_NAME}
    ${SFO_TITLE_ID}
    eboot_${APP_NAME}.bin
    VERSION
    ${SFO_APP_VER}
    NAME
    "${SFO_TITLE_NAME}"
    ${PRIVATE_PKG_FILES}
    ${PUBLIC_PKG_FILES}
    ${CORE_FILES})

add_custom_target(VPK_${APP_NAME} ALL
    COMMAND ${CMAKE_COMMAND} -E rename ${VPK_NAME} ${OUT_PATH}/${VPK_NAME})

if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
    add_dependencies(${VPK_NAME} ${CORE_EBOOTS})
    add_dependencies(VPK_${APP_NAME} ${VPK_NAME} ${ARC_TAG})
else()
    add_dependencies(${VPK_NAME}-vpk ${CORE_EBOOTS})
    add_dependencies(VPK_${APP_NAME} ${VPK_NAME}-vpk ${ARC_TAG})
endif()