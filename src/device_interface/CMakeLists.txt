CONFIGURE_FILE(devi_protocol.h       ${CUSTOM_INCLUDE_DIR_MAIN}/device_interface/devi_protocol.h)
CONFIGURE_FILE(device_interface.h    ${CUSTOM_INCLUDE_DIR_MAIN}/device_interface/device_interface.h)

ADD_LIBRARY(device_interface STATIC  device_interface.c)
