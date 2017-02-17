CONFIGURE_FILE(junction.h      ${CUSTOM_INCLUDE_DIR_MAIN}/junction/junction.h)

ADD_LIBRARY(junction STATIC  junction.c)
