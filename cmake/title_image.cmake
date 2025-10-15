cmake_minimum_required(VERSION 3.10)

macro(gen_title_image)
    set(OUTPUT_PATH ${PROJECT_SOURCE_DIR}/share/pkg/assets)
    set(TITLE_IMAGE_PATH ${OUTPUT_PATH}/emu4vita++.png)
    file(RELATIVE_PATH rel ${OUTPUT_PATH} ${TITLE_IMAGE_PATH})
    list(APPEND PUBLIC_PKG_FILES FILE ${TITLE_IMAGE_PATH} ${rel})

    set(TITLE_IMAGE_SCRIPT gen_title.py)

    add_custom_command(
        OUTPUT
        ${TITLE_IMAGE_PATH}

        COMMAND python ${TITLE_IMAGE_SCRIPT}
        COMMAND cp emu4vita++.png ${TITLE_IMAGE_PATH}

        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/script

        DEPENDS
        ${PROJECT_SOURCE_DIR}/script/${TITLE_IMAGE_SCRIPT}
    )

    add_custom_target(TITLE_IMAGE_BUILD
        DEPENDS
        ${TITLE_IMAGE_PATH})

endmacro()