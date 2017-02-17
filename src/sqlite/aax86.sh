#!/bin/bash
#/usr/local/arm/arm-none-linux-gnueabi/bin

make distclean
CURRENT_DIR=$(pwd)
CURRENT_DIR=${CURRENT_DIR%/src/sqlite*}
./configure --prefix=$CURRENT_DIR \
            --enable-static \
            --enable-threadsafe
make
make install
make distclean            
