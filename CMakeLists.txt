cmake_minimum_required(VERSION 3.12)

# set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)
set(CMAKE_DEPENDS_USE_COMPILER FALSE)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")

include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
include("cmake/arcade_dat.cmake" REQUIRED)
include("cmake/collect_files.cmake" REQUIRED)
include("cmake/cores.cmake" REQUIRED)

set(CMAKE_AR "$ENV{VITASDK}/bin/arm-vita-eabi-gcc-ar${TOOL_OS_SUFFIX}" CACHE PATH "archive")
set(CMAKE_RANLIB "$ENV{VITASDK}/bin/arm-vita-eabi-gcc-ranlib${TOOL_OS_SUFFIX}" CACHE PATH "ranlib")

option(WITH_LOG "log ouputting" OFF)
option(LTO "enable gcc lto optimzatior" OFF)
option(GPROF "enable gprof. When exiting, generate gprof dump data to ux:/data/gmon.out" OFF)

find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

GET_CORES()

set(BUILD_LIST "All" "Arch" ${CORES})
string(REPLACE "|" " " BUILD_LIST_STR ${BUILD_LIST})
set(BUILD "Arch" CACHE STRING ${BUILD_LIST_STR})
set_property(CACHE BUILD PROPERTY STRINGS ${BUILD_LIST})
string(STRIP ${BUILD} BUILD)

if(NOT ${BUILD} IN_LIST BUILD_LIST)
    message(FATAL_ERROR "valid BUILD value: ${BUILD_LIST}")
endif()

if(BUILD STREQUAL "Arch")
    set(BUILD_SELF ON)
endif()

project("Emu4Vita++")

if(WITH_LOG)
    set(LOG_LEVEL LOG_LEVEL_DEBUG)
else()
    set(LOG_LEVEL LOG_LEVEL_INFO)
endif()

set(CMAKE_C_FLAGS "-marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard \
-DVITA -D__vita__ \
-mword-relocations -fno-optimize-sibling-calls -fsingle-precision-constant \
-fno-unwind-tables -fno-asynchronous-unwind-tables \
-ffast-math -ftree-vectorize -O3 \
-fdata-sections -ffunction-sections \
-DLOG_LEVEL=${LOG_LEVEL}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -std=gnu++17 -fno-exceptions")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-q,-no-enum-size-warning,--as-needed,--gc-sections -L${CMAKE_CURRENT_SOURCE_DIR}/deps/libs") # ,--no-undefined,--as-needed,--hash-style=gnu")

set(OUT_PATH "${CMAKE_BINARY_DIR}/out")
file(MAKE_DIRECTORY ${OUT_PATH})

set(APP_VER "0.54")
set(SFO_APP_VER "00.54")

if(LTO)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -fno-fat-lto-objects ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -fno-fat-lto-objects ")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto ")

    if(EXISTS $ENV{VITASDK}/lib/gcc/arm-vita-eabi/10.3.0/liblto_plugin-0.dll)
        set(CMAKE_STATIC_LINKER_FLAGS " --plugin=$ENV{VITASDK}/lib/gcc/arm-vita-eabi/10.3.0/liblto_plugin-0.dll ")
    else()
        set(CMAKE_STATIC_LINKER_FLAGS " --plugin=$ENV{VITASDK}/lib/gcc/arm-vita-eabi/10.3.0/liblto_plugin-0.so ")
    endif()
endif()

add_subdirectory(deps)

if(GPROF)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -DENABLE_GPROF ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -DENABLE_GPROF ")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg ")
    set(GPORF_LIB vitagprof SceLibKernel_stub)
    set(NOASLR NOASLR)
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fomit-frame-pointer ")
    set(GPORF_LIB "")
    set(NOASLR "")
endif()

add_subdirectory(share)
add_subdirectory(frontend)

list(LENGTH CORES length)
math(EXPR length " ${length} - 1 ")

foreach(index RANGE ${length})
    list(GET CORE_DIRS ${index} CORE_DIR)
    list(GET CONSOLES ${index} CONSOLE)
    list(GET CORES ${index} CORE)
    list(GET CORE_SHORTS ${index} CORE_SHORT)
    list(GET TITLE_IDS ${index} SFO_TITLE_ID)

    if(BUILD STREQUAL "All" OR BUILD STREQUAL "Arch" OR BUILD STREQUAL ${CORE})
        add_subdirectory(cores cores/${CORE})
        add_subdirectory(apps apps/${CORE})
    endif()
endforeach()

if(BUILD STREQUAL "All" OR BUILD STREQUAL "Arch")
    add_subdirectory(arch)
endif()