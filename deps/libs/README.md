Compile with `CFLAGS="-marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mword-relocations -fno-optimize-sibling-calls -fsingle-precision-constant -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -ffast-math -ftree-vectorize -fdata-sections -ffunction-sections"`  -Os or -O3

`CXXFLAGS="-marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mword-relocations -fno-optimize-sibling-calls -fsingle-precision-constant -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -ffast-math -ftree-vectorize -fdata-sections -ffunction-sections -fno-rtti -std=gnu++17 -fno-exceptions"` -Os or O3

# tiny-curl
download from https://curl.se/tiny/

version 8.4.0

```cmake
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
-DCURL_DISABLE_COOKIES=ON \
-DCURL_DISABLE_BASIC_AUTH=ON \
-DCURL_DISABLE_BEARER_AUTH=ON \
-DCURL_DISABLE_DIGEST_AUTH=ON \
-DCURL_DISABLE_KERBEROS_AUTH=ON \
-DCURL_DISABLE_NEGOTIATE_AUTH=ON \
-DCURL_DISABLE_SRP=ON \
-DCURL_DISABLE_VERBOSE_STRINGS=ON \
-DCURL_DISABLE_HTTP_AUTH=ON \
-DCURL_CA_BUNDLE="vs0:data/external/cert/CA_LIST.cer" \
-DCMAKE_C_FLAGS=" -DNO_WRITEV -DSOMAXCONN=128 -marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mword-relocations -fno-optimize-sibling-calls -fsingle-precision-constant -fomit-frame-pointer -fno-unwind-tables -fdata-sections -ffunction-sections -fno-asynchronous-unwind-tables -ffast-math -ftree-vectorize -Os"
```
