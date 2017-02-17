CONFIGURE_FILE(ui_library_wrapper.h  ${CUSTOM_INCLUDE_DIR_MAIN}/ui_library/ui_library_wrapper.h)
ADD_LIBRARY(ui_library_wrapper STATIC  ui_library_wrapper.c)
