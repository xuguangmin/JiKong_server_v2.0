#!/bin/sh

make distclean
CURRENT_DIR=$(pwd)
CURRENT_DIR=${CURRENT_DIR%/src/libghttp*}
./configure --prefix=$CURRENT_DIR \
            --enable-static \
            --disable-shared

make CC=arm-none-linux-gnueabi-gcc
make install
make distclean
