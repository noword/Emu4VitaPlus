cmake_minimum_required(VERSION 3.12)

project(${CORE}_APP)

set(APP_NAME "${CORE_SHORT}_Emu4VitaPlus")
set(SFO_TITLE_NAME "Emu4Vita++ (${CORE_SHORT})")
set(ELF_NAME ${CORE_SHORT}.elf)
set(EBOOT_NAME eboot_${CORE_SHORT}.bin)

string(TOUPPER ${CORE} UPPER_CORE)

if(WITH_LOG)
    set(VPK_NAME ${APP_NAME}_${APP_VER}_log.vpk)
else()
    set(VPK_NAME ${APP_NAME}_${APP_VER}.vpk)
endif()

add_compile_definitions(
    ${UPPER_CORE}_BUILD
    ${CONSOLE}_BUILD
    _CORE_NAME="${CORE}"
    _CORE_SHORT_NAME="${CORE_SHORT}"
    CONSOLE="${CONSOLE}"
    APP_VER_STR="${APP_VER}"
    SCE_LIBC_SIZE=0x400000
)

set(FBA
    "fba_lite"
    "fbneo"
    "fbalpha2012"
    "km_fbneo_xtreme_amped")

set(MAME
    "mame2000"
    "mame2003_plus"
    "mame2003"
    "km_mame2003_xtreme_amped")

if(CORE IN_LIST FBA)
    add_compile_definitions(FBA_BUILD)
    set(ARC_DAT_NAME fba_dat.zbin)
elseif(CORE IN_LIST MAME)
    add_compile_definitions(MAME_BUILD)
    set(ARC_DAT_NAME mame_dat.zbin)
endif()

include_directories(
    ../frontend/source
    ../frontend/source/emu
    ../frontend/source/audio
    ../frontend/source/core_spec
    ../deps/libretro-common/include
    ../deps/7-Zip/C
    ../share/source)

file(GLOB SRC ../frontend/source/core_spec/*.cpp)

add_executable(${ELF_NAME} ${SRC})
add_dependencies(${ELF_NAME} ${CORE_TAG})

target_link_libraries(
    ${ELF_NAME}
    ${LIBRETRO_LIBRARY}
    retro_comm
    share
    frontend
    curl
    ssl
    crypto
    vita2d
    vita2d_ext
    imgui_vita2d
    swresample
    swscale
    avutil
    SimpleIni
    png
    jpeg
    lzma
    minizip
    7zip
    lz4
    pthread
    FLAC
    ogg
    jsoncpp
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
    SceFiber_stub
    SceSsl_stub
    SceHttp_stub
)

set(VITA_MAKE_FSELF_FLAGS "-a 0x2800000000000001")

vita_create_self(${EBOOT_NAME} ${ELF_NAME} STRIPPED)

if(NOT BUILD_SELF)
    set(PRIVATE_PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${CORE}/pkg)
    set(PRIVATE_PKG_FILES "")
    collect_files(PRIVATE_PKG_FILES ${PRIVATE_PKG_DIR})

    set(VITA_MKSFOEX_FLAGS -d ATTRIBUTE2=12)
    vita_create_vpk(
        ${VPK_NAME}
        ${SFO_TITLE_ID}
        ${EBOOT_NAME}
        VERSION
        ${SFO_APP_VER}
        NAME
        "${SFO_TITLE_NAME}"
        ${PRIVATE_PKG_FILES}
        ${PUBLIC_PKG_FILES})

    add_custom_target(VPK_${APP_NAME} ALL
        COMMAND ${CMAKE_COMMAND} -E rename ${VPK_NAME} ${OUT_PATH}/${VPK_NAME})

    get_directory_property(DEFINES COMPILE_DEFINITIONS)

    if("ARC_BUILD" IN_LIST DEFINES)
        gen_arc_dat(${PRIVATE_PKG_DIR} ${ARC_DAT_NAME})
    endif()

    if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
        add_dependencies(VPK_${APP_NAME} ${VPK_NAME} ${ARC_TAG})
    else()
        add_dependencies(VPK_${APP_NAME} ${VPK_NAME}-vpk ${ARC_TAG})
    endif()
endif()