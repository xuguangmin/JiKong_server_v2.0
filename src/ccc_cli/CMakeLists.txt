CONFIGURE_FILE(ccc_cli.h        ${CUSTOM_INCLUDE_DIR_MAIN}/ccc_cli.h)
#AUX_SOURCE_DIRECTORY(. CCCCLI_SRC_LIST)
ADD_LIBRARY(ccc_cli  ccc_cli.c ccc_command.c)

CONFIGURE_FILE(ipc_msg.h        ${CUSTOM_INCLUDE_DIR_MAIN}/ipc_msg.h)
ADD_LIBRARY(ipc_msg  ipc_msg.c)
