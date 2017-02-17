# ------------ custom variables ------------
# custom include directory from top
SET(CUSTOM_INCLUDE_DIR_FLX_COMMON   ${CUSTOM_INCLUDE_DIR_MAIN}/FLXCommon)

# ------------ set system variable ------------
# set include directory
INCLUDE_DIRECTORIES(${CUSTOM_INCLUDE_DIR_FLX_COMMON})

# ------------ target ------------
ADD_SUBDIRECTORY(FLXCommon)
ADD_SUBDIRECTORY(FLXNetWork)
ADD_SUBDIRECTORY(FLXThread)

