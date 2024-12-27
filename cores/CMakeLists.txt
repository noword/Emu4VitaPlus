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

set(SUPRX_NAME ${CORE_SHORT_NAME}.suprx)
set(SO_NAME ${CORE_SHORT_NAME})

include_directories(
    ../frontend/source
    ../frontend/source/emu
    ../frontend/source/audio
    ../frontend/source/core_spec
    ../deps/libretro-common/include
    ../deps/7-Zip/C
    ../share/source)

set(CMAKE_EXE_LINKER_FLAGS "-u retro_run -Wl,-q")

add_executable(${SO_NAME} core.c)
add_dependencies(${SO_NAME} ${CORE_TAG})

# set_target_properties(${SO_NAME}
# PROPERTIES LINK_FLAGS "-u retro_run"
# COMPILE_FLAGS "-D__PSP2_USER__"
# )
target_link_libraries(
    ${SO_NAME}
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
    ScePvf_stub_weak
    ScePgf_stub_weak
    SceDisplay_stub_weak
    SceGxm_stub_weak
    SceCtrl_stub_weak
    SceTouch_stub_weak
    ScePower_stub_weak
    SceAudio_stub_weak
    SceRtc_stub_weak
    SceCommonDialog_stub_weak
    SceSysmodule_stub_weak
    SceAppUtil_stub_weak
    SceAppMgr_stub_weak
    SceShellSvc_stub_weak
    SceMotion_stub_weak
    SceHid_stub_weak
    SceFiber_stub_weak)

vita_create_self(${SUPRX_NAME} ${SO_NAME}
    CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/exports.yml
    UNSAFE
)
vita_create_stubs(stub_${CORE_SHORT_NAME} ${SO_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/exports.yml GEN_EXPORTS)