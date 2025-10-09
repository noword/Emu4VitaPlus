# Pre compiled static libs

The main purpose is to increase the running speed or reduce the code size.

Compile with `CFLAGS="-marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mword-relocations -fno-optimize-sibling-calls -fsingle-precision-constant -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -ffast-math -ftree-vectorize -fdata-sections -ffunction-sections"`  -Os or -O3

`CXXFLAGS="-marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mword-relocations -fno-optimize-sibling-calls -fsingle-precision-constant -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -ffast-math -ftree-vectorize -fdata-sections -ffunction-sections -fno-rtti -std=gnu++17 -fno-exceptions"` -Os or O3

## tiny-curl
download from https://curl.se/tiny/

version 8.4.0

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake \
-DCMAKE_BUILD_TYPE=Release \
-DBUILD_CURL_EXE=OFF \
-DBUILD_SHARED_LIBS=OFF \
-DBUILD_STATIC_LIBS=ON \
-DBUILD_TESTING=OFF \
-DENABLE_MANUAL=OFF \
-DHTTP_ONLY=ON \
-DENABLE_IPV6=OFF \
-DCURL_DISABLE_SOCKETPAIR=ON \
-DENABLE_THREADED_RESOLVER=OFF \
-DHAVE_FCNTL_O_NONBLOCK=OFF \
-DCURL_DISABLE_NTLM=ON \
-DCURL_DISABLE_HSTS=ON \
-DCURL_DISABLE_SRP=ON \
-DCURL_DISABLE_VERBOSE_STRINGS=ON \
-DCURL_CA_BUNDLE="vs0:data/external/cert/CA_LIST.cer" \
-DCMAKE_C_FLAGS=" -DNO_WRITEV -DSOMAXCONN=128 -marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mword-relocations -fno-optimize-sibling-calls -fsingle-precision-constant -fomit-frame-pointer -fno-unwind-tables -fdata-sections -ffunction-sections -fno-asynchronous-unwind-tables -ffast-math -ftree-vectorize -Os"
```

## openssl

```bash
./Configure -DOPENSSL_SMALL_FOOTPRINT \
no-err no-dtls no-dtls1 no-shared no-threads \
no-psk no-srp no-weak-ssl-ciphers no-argon2 no-aria no-async \
no-bf no-blake2 no-camellia no-cast no-cmp no-cms \
no-comp no-deprecated no-des no-dgram no-dh no-dsa \
no-ec2m no-gost no-http no-idea no-legacy no-md4 \
no-mdc2 no-multiblock no-ocb no-quic no-rc2 no-rc4 \
no-rmd160 no-scrypt no-seed no-siphash no-siv no-sm2 \
no-sm3 no-sm4 no-srp no-srtp no-ts no-whirlpool \
no-ssl2 no-ssl3 no-hw \
-DOPENSSL_USE_IPV6=0 -DNO_FORK \
-Os --prefix=$VITASDK/arm-vita-eabi/ vita-cross
```

## ffmpeg
download from https://www.ffmpeg.org/download.html#releases

version 7.1.2

```bash
./configure --prefix=$prefix \
    --enable-cross-compile \
    --cross-prefix=$VITASDK/bin/arm-vita-eabi- \
    --disable-shared \
    --disable-runtime-cpudetect \
    --disable-armv5te \
    --disable-programs \
    --disable-doc \
    --disable-network \
    --enable-libmp3lame \
    --disable-everything \
    --enable-decoder=pcm_s16le,pcm_s24le,pcm_s32le,pcm_s8,pcm_u8,wavpack \
    --enable-demuxer=pcm_s16le,pcm_s24le,pcm_s32le,pcm_s8,pcm_u8 \
    --enable-muxer=pcm_s24le,pcm_s32le,pcm_s8,pcm_u8 \
    --enable-encoder=pcm_s16le,pcm_s24le,pcm_s32le,pcm_s8,pcm_u8,wavpack \
    --enable-protocol=file \
    --enable-static \
    --enable-small \
    --disable-debug \
    --arch=armv7-a \
    --cpu=cortex-a9 \
    --disable-armv6t2 \
    --target-os=none \
    --extra-cflags=" -Wl,-q -marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mword-relocations -fno-optimize-sibling-calls -fsingle-precision-constant -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -ffast-math -ftree-vectorize -fdata-sections -ffunction-sections -O3 -D_BSD_SOURCE" \
    --extra-cxxflags=" -Wl,-q -marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mword-relocations -fno-optimize-sibling-calls -fsingle-precision-constant -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -ffast-math -ftree-vectorize -fdata-sections -ffunction-sections -fno-rtti -std=gnu++17 -fno-exceptions -O3 -D_BSD_SOURCE" \
    --extra-ldflags=" -L$VITASDK/lib " \
    --disable-bzlib \
    --disable-iconv \
    --disable-lzma \
    --disable-sdl2 \
    --disable-securetransport \
    --disable-xlib \
    --enable-pthreads
```