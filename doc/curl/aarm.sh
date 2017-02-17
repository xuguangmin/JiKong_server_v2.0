#!/bin/bash

make distclean
CURRENT_DIR=$(pwd)
CURRENT_DIR=${CURRENT_DIR%/src/curl*}
./configure --prefix=$CURRENT_DIR \
            --host=arm-none-linux-gnueabi \
            --build=i686-linux \
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
            --disable-file
make
make install
make distclean
