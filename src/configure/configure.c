/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : configure.c
  作者    : 贾延刚
  生成日期 : 2012-10

  版本    : 1.0
  功能描述 : 封装了程序配置

  修改历史 :

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "configure.h"
#include "db_config.h"
#include "db_irda.h"
#include "sqlite_key.h"

#include "util_queue.h"
#include "sysconfig.h"
#include "protocol/protocol.h"


static SERVER_CONFIG    g_server_config;
static UPDOWN_FILE      g_updown_file;

static UTIL_QUEUE       g_util_queue;
struct update_file_info
{
	unsigned char  protocol_file_type;      // 当前正在传输的文件类型
	char          *filename_tx;             // 实际存在的文件名
	char          *filename;                // 将要修改为的文件名
};

int modify_cfg_info_all(char *cfg_string, int size)
{
	char buffer[512];
	if(!db_config_modify_cfg_info_all(cfg_string, size))
	{
		printf("update cfg info is failed\n");
		return 0;
	}

	// TODO: check if(!modify_ip_address_script(cfgInfo->ipAddr, cfgInfo->mask)) return 0;
	if(!db_config_get_ifconfig_script(buffer, 512))
		return 0;

	return modify_ifconfig_eth_script(buffer);
}

int get_cfg_info_string(char *cfg_string, int size)
{
	return db_config_get_cfg_info_string(cfg_string, size);
}

int modify_cfg_ip_address_ex(int lan_no, const char *ip_addr, const char *net_mask)
{
	char buffer[512];
	if(1 == lan_no)
	{
		modify_cfg_ip_address(ip_addr);
		modify_cfg_ip_mask(net_mask);
	}
	else if(2 == lan_no)
	{
		db_config_main_update(SQLITE_KEY_IP_ADDRESS2, ip_addr);
		db_config_main_update(SQLITE_KEY_IP_MASK2, net_mask);
	}
	else return 0;

	db_config_load_main();
	if(!db_config_get_ifconfig_script(buffer, 512))
		return 0;

	//modify_ip_address_script(ip_addr, net_mask);
	return modify_ifconfig_eth_script(buffer);
}

static const char *get_irda_db_filename()
{
	return get_res_filename(PROTOCOL_FILE_TYPE_IRDA);
}
int get_irda_data(int key, char **irda)
{
	return read_infrared_table(get_irda_db_filename(), key, irda);
}

/*TODO: check */
static const char *file_type_to_key(unsigned char protocol_file_type)
{
	switch(protocol_file_type)
	{
	case PROTOCOL_FILE_TYPE_ZIP  :return SQLITE_KEY_FILE_ZIP;
	case PROTOCOL_FILE_TYPE_SO   :return SQLITE_KEY_FILE_SO;
	case PROTOCOL_FILE_TYPE_LUA  :return SQLITE_KEY_FILE_LUA;
	case PROTOCOL_FILE_TYPE_IRDA :return SQLITE_KEY_FILE_DB;
	case PROTOCOL_FILE_TYPE_ZIP_IOS  :return SQLITE_KEY_FILE_ZIP; // TODO:  只是临时情况
	}
	return NULL;
}

const char *get_res_filename(unsigned char protocol_file_type)
{
	const char *key_name = file_type_to_key(protocol_file_type);
	if(!key_name)
		return NULL;

	return db_config_res_filename(key_name);
}

const char *get_ui_event_so_filename()
{
	return get_res_filename(PROTOCOL_FILE_TYPE_SO);
}


void close_infrared_file()
{
	close_db_infrared();
}


static int modify_filename_table_file_name(int id, const char *filename)
{
	const char *updown_key = file_type_to_key(id);
	return db_config_updown_update(updown_key, filename);
}
static void interval_print_res_file_info(UPDOWN_FILE *lp_updown_file)
{
	int k;
	if(!lp_updown_file)
		return;

	for(k = 0; k < lp_updown_file->rec_count; ++k)
	{
		printf("resource file[%s] = %s\n", lp_updown_file->res[k].file_type, lp_updown_file->res[k].file_name);
	}
	printf("\n");
}
static int update_file_from_upload(const char *filename_tx, const char *filename)
{
	if (rename(filename_tx, filename) != 0)
	{
		printf("update_file_from_upload :rename %s error :%s\n", filename_tx, strerror(errno));
		return 0;
	}

	printf("rename %s to %s success\n", filename_tx, filename);
	return 1;
}

int update_config_file()
{
	void *data;
	int user_data;

	printf("update file ......\n");

	while(util_queue_get_head_data(&g_util_queue, &data, &user_data))
	{
		struct update_file_info *file_info = (struct update_file_info *)data;
		if(!file_info)
			continue;

		if(update_file_from_upload(file_info->filename_tx, file_info->filename))
		{
			if(!modify_filename_table_file_name(file_info->protocol_file_type, file_info->filename))
			{
				printf("update %s to sqlite error.\n", file_info->filename);
			}
		}
	}

	db_config_load_updown();
	if(!db_table_updown_file_all(&g_updown_file))
		return 0;
	interval_print_res_file_info(&g_updown_file);
	return 1;
}

int delete_upload_file()
{
	void *data;
	int user_data;

	printf("delete file ......\n");

	while(util_queue_get_head_data(&g_util_queue, &data, &user_data))
	{
		struct update_file_info *file_info = (struct update_file_info *)data;
		if(!file_info)
			continue;

		remove(file_info->filename_tx);
		printf("delete file %s\n", file_info->filename_tx);
	}
	return 1;
}

int new_config_file(int protocol_file_type, const char *filename_tx, const char *filename)
{
	int len;
	struct update_file_info *file_info;
	if(!filename_tx || !filename)
		return 0;
	file_info = (struct update_file_info *)malloc(sizeof(struct update_file_info));
	if(!file_info)
		return 0;

	file_info->protocol_file_type = protocol_file_type;

	len = strlen(filename_tx);
	file_info->filename_tx = (char *)malloc(len + 1);
	strcpy(file_info->filename_tx, filename_tx);

	len = strlen(filename);
	file_info->filename = (char *)malloc(len + 1);
	strcpy(file_info->filename, filename);

	if(!util_queue_append_data(&g_util_queue, (void *)file_info, 0))
		return 0;

	return 1;
}

static int init_config()
{
	util_queue_init(&g_util_queue);
	return 1;
}

int get_cfg_server(SERVER_CONFIG *lp_server_config)
{
	return db_config_server_config(lp_server_config);
}

static void internal_print_server_info(SERVER_CONFIG *cfgInfo)
{
	int server_type = 0;
	if(!cfgInfo)
		return;

	printf("LAN(1) IP = %s\n", db_config_get_config(SQLITE_KEY_IP_ADDRESS));
	printf("LAN(2) IP = %s\n", db_config_get_config(SQLITE_KEY_IP_ADDRESS2));
	server_type = cfgInfo->server_type;
	switch(server_type)
	{
	case -1:
		printf("server type : single(%d)\n", server_type);
		printf("server port for client : %d\n", cfgInfo->port1);
		break;
	case 0:
		printf("server type : master(%d)\n", server_type);
		printf("server port for client : %d\n", cfgInfo->port1);
		printf("server port for slave : %d\n",  cfgInfo->port2);
		break;
	case 1:
		printf("server type : slave(%d)\n", server_type);
		printf("server port for client : %d\n", cfgInfo->port1);
		printf("connect to %s : %d\n", cfgInfo->dst_server_ip, cfgInfo->dst_server_port);
		break;
	}
	printf("\n");
}

void print_server_info()
{
	internal_print_server_info(&g_server_config);
}
static void print_updown_file_info()
{
	interval_print_res_file_info(&g_updown_file);
}

void close_configure()
{
	db_config_close();
	close_db_infrared();
}

static int load_external_configure(const char *config_name)
{
	if(!db_config_check(config_name))
	{
		printf("%s db_config_check() error.\n", __FUNCTION__);
		return 0;
	}

	if(!db_config_load())
	{
		printf("%s db_config_load() error.\n", __FUNCTION__);
		return 0;
	}

	if(!db_table_main_server_config(&g_server_config)) return 0;
	print_server_info();

	if(!db_table_updown_file_all(&g_updown_file)) return 0;
	print_updown_file_info();
	return 1;
}

int load_configure(const char *config_name)
{
	init_config();
	if(!load_external_configure(config_name))
		return 0;

	if(!set_ipaddr_netmask2(1, db_config_get_config(SQLITE_KEY_IP_ADDRESS), db_config_get_config(SQLITE_KEY_IP_MASK)))
	{
		printf("%s set LAN(1) IP error.\n", __FUNCTION__);
		return 0;
	}
	if(!set_ipaddr_netmask2(2, db_config_get_config(SQLITE_KEY_IP_ADDRESS2), db_config_get_config(SQLITE_KEY_IP_MASK2)))
	{
		printf("%s set LAN(2) IP error.\n", __FUNCTION__);
	}
	return 1;
}


