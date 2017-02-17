CONFIGURE_FILE(device_manager.h      ${CUSTOM_INCLUDE_DIR_MAIN}/device_controller/device_manager.h)

AUX_SOURCE_DIRECTORY(. DEVC_SRC_LIST)
ADD_LIBRARY(device_controller STATIC  ${DEVC_SRC_LIST})
