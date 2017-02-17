#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "fd_file_up_download.h"
#include "util_log.h"
#include "configure.h"
#include "upload_file.h"
#include "download_file.h"
#include "md5sum.h"
#include "protocol/protocol_define.h"


int fd_upload_create_file(FLXSocket sock, unsigned char protocol_file_type, unsigned char *data_buf, int data_len)
{
	int result;
	char *filename;
    if(!data_buf || data_len <= 0)
    	return 0;

    filename = (char *)malloc(data_len+1);
    if(!filename)
    	return 0;
    memcpy(filename, data_buf, data_len);
    filename[data_len] = '\0';

    /* save to link list */
    result = add_upload_file_node(sock, protocol_file_type, filename);

    free(filename);
	return result;
}

int fd_upload_write_file(FLXSocket sock, unsigned char *data_buf, int data_len)
{	
	FILE *pfile = get_upload_file_handle(sock);
	if(pfile)
	{
		size_t iwrited = fwrite(data_buf, sizeof(unsigned char), data_len, pfile);
		if(iwrited == data_len)
			return 1;
	}
	return 0;
}

/* 单个文件上传完 */
FLXInt32 fd_upload_single_file_finish(FLXSocket sock)
{
	return clear_single_upload(sock);
}
/*
 * mask，只使用低四位
 *  1     1   1  1
 *  IRDA LUA SO ZIP
 */
static unsigned int file_type_to_mask(unsigned char protocol_file_type)
{
	switch(protocol_file_type)
	{
	case PROTOCOL_FILE_TYPE_ZIP     :return 0x1;
	case PROTOCOL_FILE_TYPE_SO      :return 0x2;
	case PROTOCOL_FILE_TYPE_LUA     :return 0x4;
	case PROTOCOL_FILE_TYPE_IRDA    :return 0x8;
	case PROTOCOL_FILE_TYPE_ZIP_IOS :return 0x10;
	}
	return 0x0;
}

/*
 * 本次上传任务的所有文件上传完成
 * 把上传文件的历史信息放到配置部分的一个列表
 * 返回值
 *       上传文件类型的mask
 */
unsigned int fd_upload_all_file_finish(FLXSocket sock)
{
	unsigned int file_type_mask = 0x0;
	UTIL_QUEUE *upload_history;

	upload_history = get_upload_file_history(sock);
	if(upload_history)
	{
		void *data;
		int user_data;
		struct ul_file_info *file_info;
		while(util_queue_get_head_data(upload_history, &data, &user_data))
		{
			file_info = (struct ul_file_info *)data;
			if(!file_info)
				continue;

			new_config_file(file_info->protocol_file_type, file_info->filename_tx, file_info->filename);
			file_type_mask |= file_type_to_mask(file_info->protocol_file_type);
		}
	}

	delete_upload_file_node(sock);
	return file_type_mask;
}

/* 获取文件版本信息 */
int fd_download_check_file_version(const char *filename, char *fileVersion, int buf_size)
{
	/*struct stat fileAttr;*/
	if(!filename || !fileVersion || buf_size <= 0)
		return 0;
#if 0
	if (0 == stat(filename, &fileAttr))
	{
		struct tm *tm;
		int year, mon;
		//tm = localtime(&fileAttr.st_ctime);
		tm = localtime((time_t *)&fileAttr.st_mtim);
		year = tm->tm_year + 1900;
		mon  = tm->tm_mon + 1;
		sprintf(fileVersion, "%04d%02d%02d%02d%02d%02d", year, mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
	else
	{
		printf("%s stat failed.\n", __FUNCTION__);
		return 0;
	}
#endif
	return calcu_md5_checksum(filename, fileVersion, buf_size);
}

/*
 * file_len ：返回文件长度
 */
static FILE *open_file(const char* filename, long int *file_len)
{
	FILE *pfile = fopen(filename, "r");
    if(NULL == pfile)
	{
    	CCC_LOG_OUT("open file %s error: %s\n", filename, strerror(errno));
    	return NULL;
	}

	if(file_len)
	{
		long int len = 0;
		fseek(pfile, 0, SEEK_END);
		len = ftell(pfile);
		*file_len = len;
    }

	rewind(pfile);
	return pfile;
}
/*
 * 参数：
 *     file_len ：返回文件长度
 *
 * 返回值：
 *     0 错误，否则为正常
 */
int fd_download_open_file(FLXSocket sock, const char *filename, long int *file_len)
{
	FILE *file_handle;
	if(!filename || strlen(filename) <= 0)
		return 0;

	file_handle = open_file(filename, file_len);
	if(file_handle == NULL)
		return 0;

	return add_download_file_node(sock, file_handle, filename);
}

/*****************************************************************************
 功能描述 ： 从文件中读取一段数据
 输入参数 :
           buffer   数据缓存
           buf_size 缓存大小

 返回值：
        < 0 错误
        = 0 数据已传完
        > 0 正常有数据
*****************************************************************************/
int fd_download_get_data(FLXSocket sock, unsigned char *buffer, int buf_size)
{
	int len;
	FILE *pfile;
	if(!buffer || buf_size <= 0)
		return -1;

	pfile = get_download_file_handle(sock);
	if(!pfile)
		return -1;

	len = fread(buffer, sizeof(unsigned char), buf_size, pfile);
	if(len <= 0)
	{
		delete_download_file_node(sock);
	}
	return len;
}

/*
 * 停止所有下载
 * callback 回调函数指针
 */
int fd_download_stop_all(pf_fd_download_stop callback)
{
	FLXSocket sock;
	unsigned char protocol_file_type;
	while(pop_download_file_node(&sock, &protocol_file_type))
	{
		CCC_LOG_OUT("fd_download_stop_all pop_download_file_node!\n");
		if(callback) callback(sock, protocol_file_type);
	}
	return 1;
}


