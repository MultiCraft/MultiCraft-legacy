#!/bin/bash -e

. sdk.sh
CURL_VERSION=7.79.1

if [ ! -d libcurl-src ]; then
	wget https://curl.haxx.se/download/curl-$CURL_VERSION.tar.gz
	tar -xzvf curl-$CURL_VERSION.tar.gz
	mv curl-$CURL_VERSION libcurl-src
	rm curl-$CURL_VERSION.tar.gz
fi

cd libcurl-src

CFLAGS="$IOS_FLAGS" \
./configure --host=arm-apple-darwin --prefix=/ --disable-shared --enable-static \
	--disable-debug --disable-verbose --disable-versioned-symbols \
	--enable-hidden-symbols --disable-dependency-tracking \
	--disable-ares --disable-cookies --disable-crypto-auth --disable-manual \
	--disable-proxy --disable-unix-sockets --without-libidn --without-librtmp \
	--disable-ftp --disable-ldap --disable-ldaps --disable-rtsp \
	--disable-dict --disable-telnet --disable-tftp --disable-pop3 \
	--disable-imap --disable-smtp --disable-gopher --disable-sspi \
	--disable-libcurl-option --with-secure-transport
make -j

mkdir -p ../libcurl/{lib,include}
mkdir -p ../libcurl/include/curl
cp -v include/curl/*.h ../libcurl/include/curl
cp -v lib/.libs/libcurl.a ../libcurl/lib

echo "libcurl build successful"
