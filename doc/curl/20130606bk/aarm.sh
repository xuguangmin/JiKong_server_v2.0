#!/bin/sh

CURRENT_DIR=`pwd`
./configure --prefix=$CURRENT_DIR/jia_arm \
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
