cmake_minimum_required(VERSION 3.10)

function(collect_files FILES_OUT SEARCH_PATH)
    set(FILES "")
    file(GLOB_RECURSE PKG_FILES ${SEARCH_PATH}/*)

    foreach(file ${PKG_FILES})
        file(RELATIVE_PATH rel ${SEARCH_PATH} ${file})
        list(APPEND FILES FILE ${file} ${rel})
    endforeach()

    set(${FILES_OUT} "${FILES}" PARENT_SCOPE)
endfunction()
