CONFIGURE_FILE(fd_file_up_download.h  ${CUSTOM_INCLUDE_DIR_MAIN}/file_deal/fd_file_up_download.h)
ADD_LIBRARY(file_deal STATIC  fd_file_up_download.c upload_file.c download_file.c md5sum.c)
