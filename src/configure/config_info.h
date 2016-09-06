/*
 * config_info.h
 *
 *  Created on: 2013-5-21
 *      Author: flx
 */

#ifndef __CONFIG_INFO_H__
#define __CONFIG_INFO_H__

typedef struct __SERVER_CONFIG
{
	int   server_type;
	int   port1;
	int   port2;

	char *dst_server_ip;    /* 从机要连的主机IP */
	int   dst_server_port;  /* 从机要连的主机端口 */
}SERVER_CONFIG;


typedef struct __UPDOWN_FILE
{
#define UPDOWN_FILENAME_LEN_MAX    256
#define UPDOWN_FILE_COUNT_MAX      4
	struct
	{
		char    file_type[UPDOWN_FILENAME_LEN_MAX];
		char    file_name[UPDOWN_FILENAME_LEN_MAX];
	}res[UPDOWN_FILE_COUNT_MAX];

	int rec_count;
}UPDOWN_FILE;

#endif /* __CONFIG_INFO_H__ */
