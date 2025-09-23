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
-DCMAKE_C_FLAGS=" -DNO_WRITEV -DSOMAXCONN=128 -marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mword-relocations -fno-optimize-sibling-calls -fsingle-precision-constant -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -ffast-math -ftree-vectorize -Os"
```

# openssl
download from https://github.com/d3m3vilurr/vita-openssl

```
./Configure -DOPENSSL_SMALL_FOOTPRINT \
no-err no-ssl3 no-dtls no-dtls1 no-shared no-threads \
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