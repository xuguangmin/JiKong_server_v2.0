CONFIGURE_FILE(tcpserver.h   ${CUSTOM_INCLUDE_DIR_FLX_COMMON}/tcpserver.h)
SET(TCP_SRC_LIST tcpserver.c)
ADD_LIBRARY(tcp_server STATIC  ${TCP_SRC_LIST})
