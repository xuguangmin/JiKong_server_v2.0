CONFIGURE_FILE(tcp_share_type.h     ${CUSTOM_INCLUDE_DIR_MAIN}/tcp_server/tcp_share_type.h)
CONFIGURE_FILE(tps_tcp_server.h     ${CUSTOM_INCLUDE_DIR_MAIN}/tcp_server/tps_tcp_server.h)
CONFIGURE_FILE(tps_client_manage.h  ${CUSTOM_INCLUDE_DIR_MAIN}/tcp_server/tps_client_manage.h)
CONFIGURE_FILE(connect_manager.h    ${CUSTOM_INCLUDE_DIR_MAIN}/tcp_server/connect_manager.h)
CONFIGURE_FILE(wol.h                ${CUSTOM_INCLUDE_DIR_MAIN}/tcp_server/wol.h)

AUX_SOURCE_DIRECTORY(. TPS_TCP_SRC_LIST)
ADD_LIBRARY(tps_tcp_server STATIC  ${TPS_TCP_SRC_LIST})
