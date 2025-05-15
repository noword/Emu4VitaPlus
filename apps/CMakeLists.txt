cmake_minimum_required(VERSION 3.12)

set(CORE_TAG ${CORE_NAME}_build)

add_custom_target(
    ${CORE_TAG} ALL
    COMMAND make -f ${CORE_MAKEFILE_NAME} ${CORE_ARGS} -j4
    WORKING_DIRECTORY ${CORE_DIR})

add_library(${CORE_NAME} STATIC IMPORTED)
set_property(
    TARGET ${CORE_NAME}
    APPEND
    PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)

set(LIBRETRO_LIBRARY ${CORE_DIR}/${CORE_NAME}_libretro_vita.a)
set_target_properties(${CORE_NAME} PROPERTIES IMPORTED_LOCATION "${LIBRETRO_LIBRARY}")
add_dependencies(${CORE_NAME} ${CORE_TAG})

set(ELF_NAME ${CORE_SHORT_NAME}.elf)
set(EBOOT_NAME eboot_${CORE_SHORT_NAME}.bin)

if(WITH_LOG)
    set(VPK_NAME ${APP_NAME}_${APP_VER}_log.vpk)
else()
    set(VPK_NAME ${APP_NAME}_${APP_VER}.vpk)
endif()

add_compile_definitions(
    _APP_NAME_STR="${APP_NAME}"
    _APP_NAME_EXT_STR=${APP_NAME_EXT}
    _APP_VER_STR="${APP_VER}"
    _APP_TITLEID=${SFO_TITLE_ID}
    _APP_DIR_NAME="${CORE_NAME}"
    _APP_TITLE_NAME="${SFO_TITLE_NAME}"
    _CORE_FULL_NAME="${CORE_FULL_NAME}"
    _CORE_SHORT_NAME="${CORE_SHORT_NAME}"
    _CONSOLE="${CONSOLE}"

    # SCE_LIBC_SIZE=0x400000
)

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
    share
    frontend
    ${LIBRETRO_LIBRARY}
    retro_comm
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
    SceHttp_stub
    SceSsl_stub)

set(VITA_MAKE_FSELF_FLAGS "-a 0x2800000000000001")

vita_create_self(${EBOOT_NAME} ${ELF_NAME} STRIPPED)

if(NOT BUILD_SELF)
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