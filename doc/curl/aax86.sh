#!/bin/bash

make distclean
CURRENT_DIR=$(pwd)
CURRENT_DIR=${CURRENT_DIR%/src/curl*}
./configure --prefix=$CURRENT_DIR \
            --enable-static \
            --disable-manual \
            --enable-hidden-symbols \
            --disable-cookies \
            --disable-pop3 \
            --disable-smtp \
            --disable-ftp \
            --disable-imap \
            --disable-dict \
            --disable-rtsp \
            --disable-telnet \
            --disable-tftp \
            --disable-file \
            --without-zlib
make
make install
make distclean
