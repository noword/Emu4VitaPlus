cmake_minimum_required(VERSION 3.12)

# retro_comm
# ----------------------
set(RETRO_SRC
    "libretro-common/string/stdstring.c"
    "libretro-common/encodings/encoding_utf.c"
    "libretro-common/encodings/encoding_crc32.c"
    "libretro-common/compat/fopen_utf8.c"
    "libretro-common/compat/compat_strl.c"
    "libretro-common/compat/compat_snprintf.c"
    "libretro-common/compat/compat_strcasestr.c"
    "libretro-common/compat/compat_posix_string.c"
    "libretro-common/file/file_path.c"
    "libretro-common/file/file_path_io.c"
    "libretro-common/streams/file_stream.c"
    "libretro-common/streams/file_stream_transforms.c"
    "libretro-common/streams/memory_stream.c"
    "libretro-common/lists/string_list.c"
    "libretro-common/lists/file_list.c"
    "libretro-common/lists/dir_list.c"
    "libretro-common/file/retro_dirent.c"
    "libretro-common/memmap/memalign.c"
    "libretro-common/vfs/vfs_implementation.c"
    "libretro-common/time/rtime.c"
    "libretro-common/streams/interface_stream.c"
    "libretro-common/streams/rzip_stream.c"
    "libretro-common/streams/stdin_stream.c"
    "libretro-common/streams/trans_stream.c"
    "libretro-common/streams/trans_stream_pipe.c"
    "libretro-common/streams/trans_stream_zlib.c"
    "libretro-common/features/features_cpu.c"
    "libretro-common/file/config_file.c"
    "libretro-common/file/config_file_userdata.c"
    "libretro-common/rthreads/rthreads.c"
    "libretro-common/utils/md5.c"
    "libretro-common/audio/resampler/audio_resampler.c"
    "libretro-common/audio/resampler/drivers/sinc_resampler.c"
    "libretro-common/audio/resampler/drivers/nearest_resampler.c"
    "libretro-common/audio/conversion/s16_to_float.c"
    "libretro-common/audio/conversion/float_to_s16.c")

include_directories(" libretro-common/include ")

add_compile_definitions(VITA HAVE_NEON HAVE_THREADS HAVE_COMPRESSION HAVE_ZLIB HAVE_7ZIP __ARM_NEON__)
add_library(retro_comm STATIC ${RETRO_SRC})

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

# zlib-ng
# ----------------------
option(ZLIB_COMPAT "" ON)
option(ZLIB_ENABLE_TESTS "" OFF)
option(ZLIBNG_ENABLE_TESTS "" OFF)
option(WITH_GZFILEOP "" OFF)
option(WITH_GTEST "" OFF)
option(ZLIBNG_ENABLE_TESTS "" OFF)
add_subdirectory(zlib-ng)

# minizip-ng
# ----------------------
option(MZ_ZLIB "" ON)
option(MZ_BZIP2 "" OFF)
option(MZ_LZMA "" OFF)
option(MZ_ZSTD "" OFF)
option(MZ_PKCRYPT "" OFF)
option(MZ_WZAES "" OFF)
option(MZ_DECOMPRESS_ONLY "" ON)
set(MZ_ZLIB ON CACHE BOOL "" FORCE)
add_subdirectory(minizip-ng)

# lz4
# ----------------------
set(LZ4_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lib)
set(LZ4_SRC ${LZ4_PATH}/lz4.c ${LZ4_PATH}/lz4hc.c)
add_library(lz4 ${LZ4_SRC})
target_include_directories(lz4 PRIVATE ${LZ4_PATH})
