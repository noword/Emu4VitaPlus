cmake_minimum_required(VERSION 3.12)

project(${CORE}_APP)

set(APP_NAME "${CORE_SHORT}4VitaPlus")
set(SFO_TITLE_NAME "Emu4Vita++ (${CORE_SHORT})")
set(ELF_NAME ${CORE_SHORT}.elf)
set(EBOOT_NAME eboot_${CORE_SHORT}.bin)

if(CORE STREQUAL "fba_lite" OR CORE STREQUAL "fbneo" OR CORE STREQUAL "fbalpha2012" OR CORE STREQUAL "fbneo_xtreme" OR CORE STREQUAL "mame2003_xtreme")
    add_compile_definitions(ARC_BUILD)
elseif(CORE STREQUAL "frontend")
    add_compile_definitions(ARCH_BUILD)
    set(EBOOT_NAME frontend.self)
endif()

if(WITH_LOG)
    set(VPK_NAME ${APP_NAME}_${APP_VER}_log.vpk)
else()
    set(VPK_NAME ${APP_NAME}_${APP_VER}.vpk)
endif()

add_compile_definitions(
    DEFAULT_CORE_NAME="${CORE}"
    APP_VER_STR="${APP_VER}"

    # SCE_LIBC_SIZE=0x400000
)

include_directories(
    ../frontend/source
    ../frontend/source/emu
    ../frontend/source/audio
    ../frontend/source/core_spec
    ../deps/libretro-common/include
    ../deps/7-Zip/C
    ../share/source
    ../apps)

file(GLOB SRC ../frontend/source/core_spec/*.cpp)

link_directories(AFTER ${CMAKE_CURRENT_BINARY_DIR}/../../cores)

add_executable(${ELF_NAME} ${SRC})

target_link_libraries(
    ${ELF_NAME}
    retro_stub_weak
    share
    frontend
    vita2d
    vita2d_ext
    imgui_vita2d
    swresample
    swscale
    speexdsp
    avutil
    SimpleIni
    png
    jpeg
    zlibstatic
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
    SceHttp_stub_weak
    SceSsl_stub_weak)

set(VITA_MAKE_FSELF_FLAGS "-a 0x2800000000000001")

vita_create_self(${EBOOT_NAME} ${ELF_NAME} STRIPPED)

if(NOT CORE STREQUAL "frontend")
    set(PRIVATE_PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${CORE}/pkg)
    set(PRIVATE_PKG_FILES "")
    collect_files(PRIVATE_PKG_FILES ${PRIVATE_PKG_DIR})
    list(APPEND PRIVATE_PKG_FILES FILE ${CMAKE_CURRENT_BINARY_DIR}/../../cores/${CORE}/${LIBRETRO_SUPRX} ${LIBRETRO_SUPRX})

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
        gen_arc_dat(${PRIVATE_PKG_DIR})
    endif()

    if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
        add_dependencies(VPK_${APP_NAME} ${VPK_NAME} ${ARC_TAG})
    else()
        add_dependencies(VPK_${APP_NAME} ${VPK_NAME}-vpk ${ARC_TAG})
    endif()
endif()