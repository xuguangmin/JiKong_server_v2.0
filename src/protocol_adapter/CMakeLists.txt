CONFIGURE_FILE(protocol_adapter.h      ${CUSTOM_INCLUDE_DIR_MAIN}/protocol_adapter/protocol_adapter.h)

ADD_LIBRARY(protocol_adapter STATIC  protocol_adapter.c devi_proto_data.c telnet_info.c)
