cmake_minimum_required(VERSION 3.12)

# retro_comm
# ----------------------
set(LIBRETRO_PATH "libretro-common")
set(RETRO_SRC
    ${LIBRETRO_PATH}/string/stdstring.c
    ${LIBRETRO_PATH}/encodings/encoding_utf.c
    ${LIBRETRO_PATH}/encodings/encoding_crc32.c
    ${LIBRETRO_PATH}/compat/fopen_utf8.c
    ${LIBRETRO_PATH}/compat/compat_strl.c
    ${LIBRETRO_PATH}/compat/compat_snprintf.c
    ${LIBRETRO_PATH}/compat/compat_strcasestr.c
    ${LIBRETRO_PATH}/compat/compat_posix_string.c
    ${LIBRETRO_PATH}/file/file_path.c
    ${LIBRETRO_PATH}/file/file_path_io.c
    ${LIBRETRO_PATH}/streams/file_stream.c
    ${LIBRETRO_PATH}/streams/file_stream_transforms.c
    ${LIBRETRO_PATH}/streams/memory_stream.c
    ${LIBRETRO_PATH}/lists/string_list.c
    ${LIBRETRO_PATH}/lists/file_list.c
    ${LIBRETRO_PATH}/lists/dir_list.c
    ${LIBRETRO_PATH}/file/retro_dirent.c
    ${LIBRETRO_PATH}/memmap/memalign.c
    ${LIBRETRO_PATH}/vfs/vfs_implementation.c
    ${LIBRETRO_PATH}/time/rtime.c
    ${LIBRETRO_PATH}/streams/interface_stream.c
    ${LIBRETRO_PATH}/streams/rzip_stream.c
    ${LIBRETRO_PATH}/streams/stdin_stream.c
    ${LIBRETRO_PATH}/streams/trans_stream.c
    ${LIBRETRO_PATH}/streams/trans_stream_pipe.c
    ${LIBRETRO_PATH}/streams/trans_stream_zlib.c
    ${LIBRETRO_PATH}/features/features_cpu.c
    ${LIBRETRO_PATH}/file/config_file.c
    ${LIBRETRO_PATH}/file/config_file_userdata.c
    ${LIBRETRO_PATH}/rthreads/rthreads.c
    ${LIBRETRO_PATH}/audio/resampler/audio_resampler.c
    ${LIBRETRO_PATH}/audio/resampler/drivers/sinc_resampler.c
    ${LIBRETRO_PATH}/audio/resampler/drivers/nearest_resampler.c
    ${LIBRETRO_PATH}/audio/conversion/s16_to_float.c
    ${LIBRETRO_PATH}/audio/conversion/float_to_s16.c)

add_compile_definitions(VITA HAVE_NEON HAVE_THREADS HAVE_COMPRESSION HAVE_ZLIB HAVE_7ZIP __ARM_NEON__)
add_library(retro_comm STATIC ${RETRO_SRC})
target_include_directories(retro_comm PRIVATE ${LIBRETRO_PATH}/include)

# rcheevos
# ----------------------
set(RCHEEVOS_PATH rcheevos)
file(GLOB RCHEEVOS_SRC
    ${RCHEEVOS_PATH}/src/*.c
    ${RCHEEVOS_PATH}/src/rapi/*.c
    ${RCHEEVOS_PATH}/src/rcheevos/*.c
    ${RCHEEVOS_PATH}/src/rhash/*.c)
add_library(rcheevos STATIC ${RCHEEVOS_SRC})
target_include_directories(rcheevos PRIVATE
    ${RCHEEVOS_PATH}/include
    ${RCHEEVOS_PATH}/src
    ${RCHEEVOS_PATH}/src/rapi
    ${RCHEEVOS_PATH}/src/rcheevos
    ${RCHEEVOS_PATH}/src/rhash
    ${LIBRETRO_PATH}/include
)
target_compile_definitions(rcheevos PRIVATE
    RC_CLIENT_SUPPORTS_EXTERNAL
    RC_CLIENT_SUPPORTS_RAINTEGRATION
    RC_CLIENT_SUPPORTS_HASH
    md5_finish=_md5_finish)
target_compile_options(rcheevos PRIVATE -Os)

# vita2d
# ----------------------
set(VITA2D_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libvita2d/libvita2d)
set(VITA2D_SOURCE_PATH ${VITA2D_PATH}/source)
set(VITA2D_SHADER_PATH ${VITA2D_PATH}/shader/compiled)
set(VITA2D_SRC
    ${VITA2D_SOURCE_PATH}/int_htab.c
    ${VITA2D_SOURCE_PATH}/texture_atlas.c
    ${VITA2D_SOURCE_PATH}/utils.c
    ${VITA2D_SOURCE_PATH}/vita2d.c
    ${VITA2D_SOURCE_PATH}/vita2d_draw.c
    ${VITA2D_SOURCE_PATH}/vita2d_font.c
    ${VITA2D_SOURCE_PATH}/vita2d_image_bmp.c
    ${VITA2D_SOURCE_PATH}/vita2d_image_jpeg.c
    ${VITA2D_SOURCE_PATH}/vita2d_image_png.c
    ${VITA2D_SOURCE_PATH}/vita2d_pgf.c
    ${VITA2D_SOURCE_PATH}/vita2d_pvf.c
    ${VITA2D_SOURCE_PATH}/vita2d_texture.c)

set(VITA2D_OBJ
    ${VITA2D_SHADER_PATH}/clear_v_gxp.o
    ${VITA2D_SHADER_PATH}/clear_f_gxp.o
    ${VITA2D_SHADER_PATH}/color_v_gxp.o
    ${VITA2D_SHADER_PATH}/color_f_gxp.o
    ${VITA2D_SHADER_PATH}/texture_v_gxp.o
    ${VITA2D_SHADER_PATH}/texture_f_gxp.o
    ${VITA2D_SHADER_PATH}/texture_tint_f_gxp.o)

add_library(vita2d STATIC ${VITA2D_SRC})
target_include_directories(vita2d PRIVATE
    ${VITA2D_PATH}/include
    $ENV{VITASDK}/arm-vita-eabi/include/freetype2)
target_link_libraries(vita2d ${VITA2D_OBJ})

# vita2d_ext
# ----------------------
set(VITA2D_EXT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libvita2d_ext/libvita2d_ext)
set(VITA2D_EXT_SOURCE_PATH ${VITA2D_EXT_PATH}/source)
set(VITA2D_EXT_SRC ${VITA2D_EXT_SOURCE_PATH}/utils.c
    ${VITA2D_EXT_SOURCE_PATH}/vita2d_ext.c
    ${VITA2D_EXT_SOURCE_PATH}/vita2d_ext_draw.c)

add_library(vita2d_ext STATIC ${VITA2D_EXT_SRC})

target_include_directories(vita2d_ext PRIVATE
    ${VITA2D_EXT_PATH}/include
    ${VITA2D_PATH}/include)

# 7zip
# ----------------------
set(SEVENZIP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/7-Zip/C)
set(SEVEN_SRC ${SEVENZIP_PATH}/7zAlloc.c
    ${SEVENZIP_PATH}/7zArcIn.c
    ${SEVENZIP_PATH}/7zBuf.c
    ${SEVENZIP_PATH}/7zCrc.c
    ${SEVENZIP_PATH}/7zCrcOpt.c
    ${SEVENZIP_PATH}/7zDec.c
    ${SEVENZIP_PATH}/7zFile.c
    ${SEVENZIP_PATH}/7zStream.c
    ${SEVENZIP_PATH}/Bcj2.c
    ${SEVENZIP_PATH}/Bra.c
    ${SEVENZIP_PATH}/Bra86.c
    ${SEVENZIP_PATH}/BraIA64.c
    ${SEVENZIP_PATH}/CpuArch.c
    ${SEVENZIP_PATH}/Delta.c
    ${SEVENZIP_PATH}/LzFind.c
    ${SEVENZIP_PATH}/LzFindMt.c
    ${SEVENZIP_PATH}/LzFindOpt.c
    ${SEVENZIP_PATH}/Lzma2Dec.c
    ${SEVENZIP_PATH}/Lzma2Enc.c
    ${SEVENZIP_PATH}/LzmaDec.c
    ${SEVENZIP_PATH}/LzmaEnc.c
    ${SEVENZIP_PATH}/MtCoder.c
    ${SEVENZIP_PATH}/MtDec.c
    ${SEVENZIP_PATH}/Threads.c)

# add_compile_definitions(USE_FOPEN)
add_library(7zip ${SEVEN_SRC})
target_include_directories(7zip PRIVATE ${SEVENZIP_PATH})

# simpleini
# ----------------------
add_subdirectory(simpleini)

# minizip-ng
# ----------------------
option(MZ_BZIP2 " " OFF)
option(MZ_LZMA " " OFF)
option(MZ_ZSTD " " OFF)
option(MZ_PKCRYPT " " OFF)
option(MZ_WZAES " " OFF)
option(MZ_DECOMPRESS_ONLY " " ON)
option(MZ_FETCH_LIBS " " ON)

# set(MZ_ZLIB_FLAVOR zlib-ng)

# option(MZ_FORCE_FETCH_LIBS " " ON)
option(ZLIB_COMPAT " " ON)
option(ZLIB_ENABLE_TESTS " " OFF)
option(ZLIBNG_ENABLE_TESTS " " OFF)
option(WITH_GZFILEOP " " OFF)
option(WITH_GTEST " " OFF)
add_subdirectory(minizip-ng)

# lz4
# ----------------------
set(LZ4_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lib)
set(LZ4_SRC ${LZ4_PATH}/lz4.c ${LZ4_PATH}/lz4hc.c)
add_library(lz4 ${LZ4_SRC})
target_include_directories(lz4 PRIVATE ${LZ4_PATH})

# gprof
# ----------------------
if(GPROF)
    project(vitagprof C ASM)
    set(GPROF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gprof)
    set(GPROF_SRC ${GPROF_PATH}/prof.c ${GPROF_PATH}/mcount.s)
    add_library(vitagprof ${GPROF_SRC})
    target_include_directories(vitagprof PRIVATE ${GPROF_PATH})
endif()