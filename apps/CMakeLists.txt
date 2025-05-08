cmake_minimum_required(VERSION 3.12)

# set(CORE_TAG ${CORE_NAME}_build)

# add_custom_target(
# ${CORE_TAG} ALL
# COMMAND make -f ${CORE_MAKEFILE_NAME} ${CORE_ARGS} -j4
# WORKING_DIRECTORY ${CORE_DIR})

# add_library(${CORE_NAME} STATIC IMPORTED)
# set_property(
# TARGET ${CORE_NAME}
# APPEND
# PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)

# set(LIBRETRO_LIBRARY ${CORE_DIR}/${CORE_NAME}_libretro_vita.a)
# set_target_properties(${CORE_NAME} PROPERTIES IMPORTED_LOCATION "${LIBRETRO_LIBRARY}")
# add_dependencies(${CORE_NAME} ${CORE_TAG})
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cores ${CMAKE_CURRENT_BINARY_DIR}/cores)
project(${CORE}_APP)

if(CORE STREQUAL "gpsp")
    set(SFO_TITLE_ID "GPSP4VITA")
    set(CONSOLE "GBA")
    add_compile_definitions(GPSP_BUILD GBA_BUILD)
elseif(CORE STREQUAL "snes9x2002")
else()
    message(FATAL_ERROR "unknown core: " ${CORE})
endif()

set(APP_NAME "${CORE_SHORT}4VitaPlus")
set(SFO_TITLE_NAME "Emu4Vita++ (${CORE_SHORT})")

set(ELF_NAME ${CORE_SHORT}.elf)
set(EBOOT_NAME eboot_${CORE_SHORT}.bin)

if(WITH_LOG)
    set(VPK_NAME ${APP_NAME}_${APP_VER}_log.vpk)
else()
    set(VPK_NAME ${APP_NAME}_${APP_VER}.vpk)
endif()

add_compile_definitions(

    # _APP_NAME_STR="${APP_NAME}"
    # _APP_NAME_EXT_STR=${APP_NAME_EXT}
    # _APP_VER_STR="${APP_VER}"
    # _APP_TITLEID=${SFO_TITLE_ID}
    # _APP_DIR_NAME="${CORE_NAME}"
    # _APP_TITLE_NAME="${SFO_TITLE_NAME}"
    # _CORE_FULL_NAME="${CORE_FULL_NAME}"
    # _CORE_SHORT_NAME="${CORE_SHORT_NAME}"
    # _CONSOLE="${CONSOLE}"
    DEFAULT_CORE_NAME=${CORE}
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

link_directories(AFTER ${CMAKE_CURRENT_BINARY_DIR}/../cores)

add_executable(${ELF_NAME} ${SRC})

# add_dependencies(${ELF_NAME} ${CORE_TAG})
target_link_libraries(
    ${ELF_NAME}
    libretro_stub_weak
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

set(PRIVATE_PKG_FILES "")
collect_files(PRIVATE_PKG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${CORE}/pkg)
list(APPEND PRIVATE_PKG_FILES FILE ${CMAKE_CURRENT_BINARY_DIR}/../cores/${LIBRETRO_SUPRX} ${LIBRETRO_SUPRX})

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

# add_executable(${LIBRETRO_ELF} core.cpp sbrk.c)
# target_link_libraries(${LIBRETRO_ELF}
# ${LIBRETRO_LIBRARY}
# retro_comm
# pthread
# 7zip
# zlibstatic
# FLAC
# ogg
# SceLibKernel_stub
# SceIofilemgr_stub
# SceDisplay_stub
# SceCtrl_stub
# ScePower_stub
# SceVideodec_stub
# SceAppMgr_stub
# SceSysmem_stub
# SceSysmodule_stub
# SceShellSvc_stub
# SceThreadmgrForDriver_stub
# SceKernelThreadMgr_stub
# SceMotion_stub
# SceFiber_stub
# )

# set_target_properties(${LIBRETRO_ELF} PROPERTIES COMPILE_FLAGS "-Wno-main")
# vita_create_self(${LIBRETRO_SUPRX} ${LIBRETRO_ELF} UNSAFE CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/core.yml)

# vita_create_stubs(${CORE_NAME}_stubs ${LIBRETRO_SUPRX} ${CMAKE_CURRENT_SOURCE_DIR}/core.yml GEN_EXPORTS)