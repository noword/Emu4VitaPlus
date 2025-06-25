cmake_minimum_required(VERSION 3.10)

macro(gen_arc_dat OUTPUT_PATH ARC_DAT_NAME)
    set(ARC_DAT_PATH ${OUTPUT_PATH}/assets/${ARC_DAT_NAME})
    file(MAKE_DIRECTORY ${OUTPUT_PATH}/assets)
    file(RELATIVE_PATH rel ${OUTPUT_PATH} ${ARC_DAT_PATH})
    list(APPEND PRIVATE_PKG_FILES FILE ${ARC_DAT_PATH} ${rel})

    set(ARC_SCRIPT grab_arc_dat.py)
    set(ARC_TAG ${APP_NAME}_ARC_DAT)

    add_custom_command(
        OUTPUT
        ${ARC_DAT_PATH}

        COMMAND python ${ARC_SCRIPT}
        COMMAND cp ${ARC_DAT_NAME} ${ARC_DAT_PATH}

        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/script

        DEPENDS
        ${CMAKE_SOURCE_DIR}/script/${ARC_SCRIPT}
        ${CMAKE_SOURCE_DIR}/cores/libretro-fbneo/dats/*
    )

    add_custom_target(${ARC_TAG}
        DEPENDS
        ${ARC_DAT_PATH})
endmacro()