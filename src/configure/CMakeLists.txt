# configure directory
CONFIGURE_FILE(configure.h        ${CUSTOM_INCLUDE_DIR_MAIN}/configure.h)
CONFIGURE_FILE(config_info.h      ${CUSTOM_INCLUDE_DIR_MAIN}/config_info.h)
AUX_SOURCE_DIRECTORY(. CONFIG_SRC_LIST)
ADD_LIBRARY(configure STATIC  ${CONFIG_SRC_LIST})
