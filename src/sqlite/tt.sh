#!/bin/sh

stringZ=123456789
echo $stringZ
echo ${stringZ:2}

CURRENT_DIR=`pwd`
echo $CURRENT_DIR
#CURRENT_DIR=$(dirname $CURRENT_DIR)
#CURRENT_DIR=$(dirname $CURRENT_DIR)
CURRENT_DIR=${CURRENT_DIR%/src/sqlite*}
echo $CURRENT_DIR
