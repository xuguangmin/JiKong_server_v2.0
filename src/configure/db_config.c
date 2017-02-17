/*
 * app_config.c
 *
 *  Created on: 2013-5-17
 *      Author: flx
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "sqlite3.h"
#include "util_func.h"
#include "db_utils.h"
#include "sqlite_key.h"
#include "db_table_main.h"
#include "db_table_updown.h"
#include "db_config.h"

static sqlite3 *g_db_config          = NULL;
static char    *g_db_config_filename = NULL;


/*
 * 如果要在数据库中增加表，在这儿增加
 */
static struct __db_config_table_list{
	char *table_name;
	int (*table_create)(sqlite3 *db, const char *tab_name);
}db_config_table_list[] = {
	{SQLITE_TABLE_MAIN,          db_table_main_create},
	{SQLITE_TABLE_UPDOWN,        db_table_updown_create},
	{NULL,			             NULL}
};

static int internal_db_config_open(const char *zFilename)
{
	if(NULL == g_db_config)
	{
		if(!zFilename || !sqlite_db_open(zFilename, &g_db_config))
		{
			g_db_config = NULL;
			return 0;
		}
	}
	return 1;
}

int db_config_open()
{
	return internal_db_config_open(g_db_config_filename);
}
void db_config_close()
{
	if(g_db_config != NULL)
	{
		sqlite3_close(g_db_config);

		g_db_config = NULL;
		if(g_db_config_filename) printf("%s closed\n", g_db_config_filename);
	}
}

int db_config_modify_cfg_info(const char *key_name, const char *new_value)
{
	return db_table_main_update_old(g_db_config, SQLITE_TABLE_MAIN, key_name, new_value);
}
int db_config_main_update(const char *key_name, const char *new_value)
{
	return db_table_main_update(g_db_config, SQLITE_TABLE_MAIN, key_name, new_value);
}
int db_config_updown_update(const char *key_name, const char *new_value)
{
	return db_table_updown_update(g_db_config, SQLITE_TABLE_UPDOWN, key_name, new_value);
}

/*
 * 修改App配置信息
 *
 * 修改数据库中ConfigTab表中的字段
 * 参数：
 *      col_name   字段名
 *      new_value  新值
 *
 * 返回值：0失败，否则成功
 */
static int modify_cfg_info(const char *key_name, const char *new_value)
{
	return db_table_main_update_old(g_db_config, SQLITE_TABLE_MAIN, key_name, new_value);
}

int modify_config_table_system_id  (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_SYSTEM_ID,   new_value);}
int modify_config_table_device_id  (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_DEVICE_ID,   new_value);}
int modify_cfg_host_name  (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_HOST_NAME,   new_value);}
int modify_cfg_ip_type    (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_IP_TYPE,     new_value);}
int modify_cfg_port_1     (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_PORT_1,      new_value);}
int modify_cfg_port_2     (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_PORT_2,      new_value);}
int modify_cfg_ip_address (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_IP_ADDRESS,  new_value);}
int modify_cfg_ip_mask    (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_IP_MASK,     new_value);}
int modify_cfg_gate_way   (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_GATE_WAY,    new_value);}
int modify_cfg_dns_suffix (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_DNS_SUFFIX,  new_value);}
int modify_cfg_domain_1   (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_DOMAIN_1,    new_value);}
int modify_cfg_domain_2   (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_DOMAIN_2,    new_value);}
int modify_cfg_server_type(const char *new_value)  {return modify_cfg_info(SQLITE_KEY_SERVER_TYPE, new_value);}
int modify_cfg_server_ip  (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_SERVER_IP,   new_value);}
int modify_cfg_server_port(const char *new_value)  {return modify_cfg_info(SQLITE_KEY_SERVER_PORT, new_value);}
int modify_cfg_server_dns (const char *new_value)  {return modify_cfg_info(SQLITE_KEY_SERVER_DNS,  new_value);}

/*
 * 修改程序配置信息
 *
 * 该配置信息来自设计器客户端，是一串以分号（;）分割的字符串
 * 参数：
 *      cfg_string  配置字符串
 *      size        长度
 *
 * 返回值：0失败，否则成功
 */
int db_config_modify_cfg_info_all(char *cfg_string, int size)
{
	char *p;
	int flag = 0;
	char *delim = ";";

	p = strtok(cfg_string, delim);
	if(NULL == p)
		return 0;
	if(!modify_config_table_system_id(p))
		return 0;

	while((p = strtok(NULL, delim)))
	{
		flag++;

		if     (flag ==1)  {if(!modify_config_table_device_id(p))   return 0;}
		else if(flag ==2)  {if(!modify_cfg_host_name(p))   return 0;}
		else if(flag ==3)  {if(!modify_cfg_ip_type(p))     return 0;}
		else if(flag ==4)  {if(!modify_cfg_ip_address(p))  return 0;}
		else if(flag ==5)  {if(!modify_cfg_port_1(p))      return 0;}
		else if(flag ==6)  {if(!modify_cfg_port_2(p))      return 0;}
		else if(flag ==7)  {if(!modify_cfg_ip_mask(p))     return 0;}
		else if(flag ==8)  {if(!modify_cfg_gate_way(p))    return 0;}
		else if(flag ==9)  {if(!modify_cfg_dns_suffix(p))  return 0;}
		else if(flag ==10) {if(!modify_cfg_domain_1(p))    return 0;}
		else if(flag ==11) {if(!modify_cfg_domain_2(p))    return 0;}
		else if(flag ==12) {if(!modify_cfg_server_type(p)) return 0;}
		else if(flag ==13) {if(!modify_cfg_server_ip(p))   return 0;}
		else if(flag ==14) {if(!modify_cfg_server_port(p)) return 0;}
		else if(flag ==15) {if(!modify_cfg_server_dns(p))  return 0;}
	}

	if(flag < 15)
		return 0;

	if(!db_table_main_load(g_db_config, SQLITE_TABLE_MAIN))
	{
		printf("update cfg info is failed\n");
		return 0;
	}

	printf("update cfg info success\n");
	return 1;
}

int db_config_server_config(SERVER_CONFIG *lp_server_config)
{
	return db_table_main_server_config(lp_server_config);
}
int db_config_get_cfg_info_string(char *cfg_string, int size)
{
	return db_table_main_get_cfg_info_string(cfg_string, size);
}
int db_config_get_ifconfig_script(char *buffer, int size)
{
	int len;
	if(!buffer || size <= 256)
		return 0;

	memset(buffer, 0, size);
	sprintf(buffer,"ifconfig lo 127.0.0.1\nifconfig eth0 %s netmask %s\nifconfig eth1 %s netmask %s", db_table_main_get(SQLITE_KEY_IP_ADDRESS),\
			                                db_table_main_get(SQLITE_KEY_IP_MASK),\
			                                db_table_main_get(SQLITE_KEY_IP_ADDRESS2),\
			                                db_table_main_get(SQLITE_KEY_IP_MASK2));
	len = strlen(buffer);
	buffer[len] = '\0';
	return 1;

}
const char *db_config_res_filename(const char *key_name)
{
	return db_table_updown_res_filename(key_name);
}

const char *db_config_get_config(const char *key_name)
{
	return db_table_main_get(key_name);
}

/*
 * 从数据库中取配置信息，数据库中有两张表fileNameTab和main表
 *
 * 返回值：1成功，否则失败
 */
int db_config_load()
{
	if(!db_config_open())
		return 0;
	//获取上传的文件名
	if(!db_table_updown_load(g_db_config, SQLITE_TABLE_UPDOWN))
		return 0;
	//获取程序配置数据
	return db_table_main_load(g_db_config, SQLITE_TABLE_MAIN);
}
int db_config_load_main()
{
	if(!db_config_open())
		return 0;

	return db_table_main_load(g_db_config, SQLITE_TABLE_MAIN);
}
int db_config_load_updown()
{
	if(!db_config_open())
		return 0;

	return db_table_updown_load(g_db_config, SQLITE_TABLE_UPDOWN);
}

int db_config_check(const char *db_name)
{
	int k;
	if(!db_name || strlen(db_name) <= 0)
		return 0;

	//if (0 == access(db_name, F_OK)){}
	//保存数据库文件名，malloc
	g_db_config_filename = util_strcpy(db_name);
	if(!db_config_open())
		return 0;
	//检查数据库表是否存在，否则创建数据库表
	for(k = 0; ;k++)
	{
		if(db_config_table_list[k].table_name == NULL)
			break;

		if(!sqlite_table_is_exist(g_db_config, db_config_table_list[k].table_name))
		{
			if(!db_config_table_list[k].table_create(g_db_config, db_config_table_list[k].table_name))
				return 0;
		}
	}

	db_config_close();
	return 1;
}
