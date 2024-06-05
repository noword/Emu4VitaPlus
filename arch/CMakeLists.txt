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

set(BUILD_SELF ON)

set(APPS gpSP fba_lite Snes9x2002 Snes9x2005 Snes9x2010 FCEUmm Nestopia)
set(APP_EBOOTS "")
set(APP_FILES "")

foreach(app ${APPS})
    if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
        list(APPEND APP_EBOOTS "eboot_${app}.bin")
    else()
        list(APPEND APP_EBOOTS "eboot_${app}.bin-self")
    endif()

    list(APPEND APP_FILES FILE "${CMAKE_CURRENT_BINARY_DIR}/../apps/${app}/frontend/eboot_${app}.bin" "eboot_${app}.self")
endforeach()

include_directories(
    source
    ../share/source
    ../deps/libvita2d/libvita2d/include
    ../deps/simpleini)

file(GLOB SRC
    source/*.cpp
    source/ui/*.cpp
    source/audio/*.cpp
    ../share/source/*.cpp)

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

set(VPK_NAME ${APP_NAME}_${APP_VER}.vpk)
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
    ${APP_FILES})

add_custom_target(VPK_${APP_NAME} ALL
    COMMAND ${CMAKE_COMMAND} -E rename ${VPK_NAME} ${OUT_PATH}/${VPK_NAME})

if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
    add_dependencies(${VPK_NAME} ${APP_EBOOTS})
    add_dependencies(VPK_${APP_NAME} ${VPK_NAME})
else()
    add_dependencies(${VPK_NAME}-vpk ${APP_EBOOTS})
    add_dependencies(VPK_${APP_NAME} ${VPK_NAME}-vpk)
endif()