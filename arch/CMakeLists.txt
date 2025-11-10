cmake_minimum_required(VERSION 3.12)

set(APP_NAME "Emu4VitaPlus")
set(APP_DIR_NAME "Arch")
set(SFO_TITLE_NAME "Emu4Vita++")
set(SFO_TITLE_ID "EMU4VPLUS")
set(PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pkg)

set(PRIVATE_PKG_FILES "")
collect_files(PRIVATE_PKG_FILES ${PKG_DIR})

project("Arch")

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

    list(APPEND CORE_FILES FILE "${CMAKE_CURRENT_BINARY_DIR}/../apps/${core}/eboot_${core_short}.bin" "eboot_${core}.self")
endforeach()

gen_arc_dat(${PKG_DIR} fba_dat.zbin)
gen_arc_dat(${PKG_DIR} mame_dat.zbin)

include_directories(
    source
    ../share/source
    ../deps/libvita2d/libvita2d/include
    ../deps/simpleini)

file(GLOB SRC source/*.cpp)

add_custom_command(OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_arch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_arch.cpp

    COMMAND python export_arch.py
    COMMAND mv language_arch.h ${CMAKE_CURRENT_SOURCE_DIR}/source/language_arch.h
    COMMAND mv language_arch.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/language_arch.cpp

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../script

    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/arch.json
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/export_arch.py
)

add_custom_target(
    LANG_ARCH
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_arch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_arch.cpp)

if(NOT ${CMAKE_CURRENT_SOURCE_DIR}/source/language_arch.cpp IN_LIST SRC)
    list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/language_arch.cpp)
endif()

add_compile_definitions(
    APP_NAME_STR="${APP_NAME}"
    APP_NAME_EXT_STR=${APP_NAME_EXT}
    APP_VER_STR="${APP_VER}"
    APP_TITLEID=${SFO_TITLE_ID}
    APP_DIR_NAME="${APP_DIR_NAME}"
    APP_TITLE_NAME="${SFO_TITLE_NAME}"
    CORE_SOFTWARE=${CORE_SOFTWARE})

add_executable(${APP_NAME}.elf ${SRC})
add_dependencies(${APP_NAME}.elf LANG_ARCH)

target_link_libraries(
    ${APP_NAME}.elf
    ${GPORF_LIB}
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
    SceFiber_stub
    SceHttp_stub
    SceSsl_stub)

set(VITA_MAKE_FSELF_FLAGS "-a 0x2800000000000001")
set(EBOOT_NAME "eboot_${APP_NAME}.bin")
vita_create_self(${EBOOT_NAME} ${APP_NAME}.elf STRIPPED REL_OPTIMIZE)

if(WITH_LOG)
    set(VPK_NAME ${APP_NAME}_v${APP_VER}_log.vpk)
else()
    set(VPK_NAME ${APP_NAME}_v${APP_VER}.vpk)
endif()

set(VITA_MKSFOEX_FLAGS -d ATTRIBUTE2=12)
vita_create_vpk(
    ${VPK_NAME}
    ${SFO_TITLE_ID}
    ${EBOOT_NAME}
    VERSION
    ${SFO_APP_VER}
    NAME
    " ${SFO_TITLE_NAME} "
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

add_custom_target(send COMMAND curl -T ${EBOOT_NAME} " ftp://\${PSVITAIP}:1337/ux0:/app/${SFO_TITLE_ID}/eboot.bin ")
add_dependencies(send VPK_${APP_NAME})

add_custom_target(sendvpk COMMAND curl -T ${OUT_PATH}/${VPK_NAME} " ftp://\${PSVITAIP}:1337/ux0: ")
add_dependencies(sendvpk VPK_${APP_NAME})