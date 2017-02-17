CONFIGURE_FILE(packet_pool.h        ${CUSTOM_INCLUDE_DIR_MAIN}/packet_pool/packet_pool.h)
CONFIGURE_FILE(ccc_packet.h         ${CUSTOM_INCLUDE_DIR_MAIN}/packet_pool/ccc_packet.h)
AUX_SOURCE_DIRECTORY(. PACKET_POOL_SRC_LIST)
ADD_LIBRARY(packet_pool  ${PACKET_POOL_SRC_LIST})
