CONFIGURE_FILE(protocol_define.h       ${CUSTOM_INCLUDE_DIR_MAIN}/protocol/protocol_define.h)
CONFIGURE_FILE(protocol_core.h         ${CUSTOM_INCLUDE_DIR_MAIN}/protocol/protocol_core.h)
CONFIGURE_FILE(protocol_http.h         ${CUSTOM_INCLUDE_DIR_MAIN}/protocol/protocol_http.h)
CONFIGURE_FILE(protocol_infrared.h     ${CUSTOM_INCLUDE_DIR_MAIN}/protocol/protocol_infrared.h)
CONFIGURE_FILE(protocol.h              ${CUSTOM_INCLUDE_DIR_MAIN}/protocol/protocol.h)

ADD_LIBRARY(protocol STATIC  protocol_core.c protocol_http.c protocol_infrared.c)
