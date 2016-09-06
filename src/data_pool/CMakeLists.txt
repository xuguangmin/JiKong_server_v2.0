CONFIGURE_FILE(ccc_data_pool.h        ${CUSTOM_INCLUDE_DIR_MAIN}/data_pool/ccc_data_pool.h)
AUX_SOURCE_DIRECTORY(. DATA_POOL_SRC_LIST)
ADD_LIBRARY(data_pool  ${DATA_POOL_SRC_LIST})
