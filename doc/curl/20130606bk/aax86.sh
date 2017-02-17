#!/bin/sh

CURRENT_DIR=`pwd`
./configure --prefix=$CURRENT_DIR/jia_x86 \
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
