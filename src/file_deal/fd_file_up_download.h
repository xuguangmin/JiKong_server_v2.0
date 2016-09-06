#ifndef __FD_FILE_UP_DOWNLOAD_H__
#define __FD_FILE_UP_DOWNLOAD_H__

#include "FLXCommon/flxtypes.h"
#include "FLXCommon/flxnettypes.h"

/*
 * ֹͣ����ʱ���õĻص�����
 */
typedef void (*pf_fd_download_stop)(FLXSocket sock, unsigned char protocol_file_type);

/*
 * �ϴ����ʱ���õĻص����������������ϴ��������ļ���ʷ
 */
typedef void (*pf_fd_upload_history)(unsigned char protocol_file_type, const char *filename, const char *filename_tx);

extern int fd_upload_create_file(FLXSocket sock, unsigned char protocol_file_type, unsigned char *data_buf, int data_len);
extern int fd_upload_write_file(FLXSocket sock, unsigned char *data_buf, int data_len);
extern int fd_upload_single_file_finish(FLXSocket sock);
extern unsigned int fd_upload_all_file_finish(FLXSocket sock);

extern int fd_download_check_file_version(const char *filename, char *fileVersion, int buf_size);
extern int fd_download_open_file(FLXSocket sock, const char *filename, long int *file_len);
extern int fd_download_get_data(FLXSocket sock, unsigned char *buffer, int buf_size);
extern int fd_download_stop_all(pf_fd_download_stop callback);


#endif  /* __FD_FILE_UP_DOWNLOAD_H__ */
