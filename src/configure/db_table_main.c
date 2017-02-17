/******************************************************************************

                  版权所有 (C), 2001-2020

 ******************************************************************************
  文件名    ：db_table_main.c
  作者      ：贾延刚
  生成日期   ：2013-5-17
  功能描述   : 在一个指定数据库上创建一个保存配置信息的表

  函数列表   :
  修改历史   :

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sqlite3.h"
#include "sqlite_key.h"
#include "util_func.h"
#include "config_info.h"

static char *g_table_name_config = NULL;
/*
 * 表的初始值
 * 纵向排列，以关键字识别每一行
 * 如果要在表中增加行，在这儿增加
 */
static struct __db_table_main_init_value {
	char *key_name;
	char  value[256];
}db_table_main_init_value[] = {
	{SQLITE_KEY_SYSTEM_ID,	     "1" },
	{SQLITE_KEY_DEVICE_ID,       "2" },
	{SQLITE_KEY_HOST_NAME,       "hostName1" },
	{SQLITE_KEY_IP_TYPE,         "0" },
	{SQLITE_KEY_PORT_1,          "9000" },
	{SQLITE_KEY_PORT_2,          "9001" },
	{SQLITE_KEY_IP_ADDRESS,      "192.168.1.100" },
	{SQLITE_KEY_IP_MASK,         "255.255.255.0" },
	{SQLITE_KEY_GATE_WAY,        "192.168.1.1" },
	{SQLITE_KEY_DNS_SUFFIX,      "202.168.1.228" },
	{SQLITE_KEY_DOMAIN_1,        "202.168.1.227" },
	{SQLITE_KEY_DOMAIN_2,        "202.168.1.226" },
	{SQLITE_KEY_SERVER_TYPE,     "-1" },
	{SQLITE_KEY_SERVER_IP,       "192.168.1.101" },
	{SQLITE_KEY_SERVER_PORT,     "9001" },
	{SQLITE_KEY_SERVER_DNS,      "202.168.1.228" },
	{SQLITE_KEY_IP_ADDRESS2,     "192.168.2.100" },
	{SQLITE_KEY_IP_MASK2,        "255.255.255.0" },
	{SQLITE_KEY_GATE_WAY2,       "192.168.2.1" },
	{NULL,			             "" }
};

static struct __db_table_main_rec
{
	char key_name[100];
	char value[100];
}db_table_main_rec;

/* 修改记录的值 */
static int db_table_main_load_set(const char *key_name, const char *new_value)
{
	int k;
	if(!key_name || !new_value || strlen(key_name) <= 0 || strlen(new_value) <= 0)
		return 0;

	for(k = 0; ;k++)
	{
		if(db_table_main_init_value[k].key_name == NULL)
			break;

		if(!strcasecmp(db_table_main_init_value[k].key_name, key_name))
		{
			strcpy(db_table_main_init_value[k].value, new_value);
			return 1;
		}
	}
	return 0;
}
/* 获取记录的值 */
static char *db_table_main_get_value(const char *key_name)
{
	int k;
	if(!key_name || strlen(key_name) <= 0)
		return NULL;

	for(k = 0; ;k++)
	{
		if(db_table_main_init_value[k].key_name == NULL)
			break;

		if(!strcasecmp(db_table_main_init_value[k].key_name, key_name))
		{
			return db_table_main_init_value[k].value;
		}
	}
	return NULL;
}

static int db_table_main_insert_init_value(sqlite3 *db, const char *tab_name)
{
	int k, sqlResult;
	char sqlstr[512];
	if(!db || !tab_name)
		return 0;

	for(k = 0; ;k++)
	{
		if(db_table_main_init_value[k].key_name == NULL)
			break;

		sprintf(sqlstr, "insert into %s values ('%s','%s')", tab_name, db_table_main_init_value[k].key_name, db_table_main_init_value[k].value);
		sqlResult = sqlite3_exec(db, sqlstr, NULL, NULL, NULL);
		if(sqlResult != SQLITE_OK )
		{
			printf("%s SQLITE SQL failed :%s\n", __FUNCTION__, sqlstr);
			return 0;
		}
	}
	return 1;
}

/* 检查关键字key_name指定的记录是否存在 */
static int internal_db_table_main_row_query(sqlite3 *db, const char *tabname, const char *key_name)
{
	char sqlstr[512];
	char *pErrMsg = NULL;
	char **db_result;
	int nrow, ncol;

	sprintf(sqlstr, "select * from %s where key='%s'", tabname, key_name);
	if(sqlite3_get_table(db, sqlstr, &db_result, &nrow, &ncol, &pErrMsg) != SQLITE_OK)
	{
		sqlite3_free_table(db_result);
		printf("%s SQLITE SQL failed :%s, %s\n", __FUNCTION__, sqlstr, pErrMsg);
		return 0;
	}

	sqlite3_free_table(db_result);
	return nrow;
}

/* 插入一条记录 */
static int internal_db_table_main_insert(sqlite3 *db, const char *tab_name, const char *key_name, const char *value)
{
	char sqlstr[512];
	if(!db || !tab_name || !key_name || strlen(key_name) <= 0)
		return 0;
	if(!value || strlen(value) <= 0)
		return 0;

	sprintf(sqlstr, "insert into %s values ('%s','%s')", tab_name, key_name, value);
	if(sqlite3_exec(db, sqlstr, NULL, NULL, NULL) != SQLITE_OK )
	{
		printf("%s SQLITE SQL failed :%s\n", __FUNCTION__, sqlstr);
		return 0;
	}
	return 1;
}
/* 根据关键字修改记录的值 */
static int internal_db_table_main_update(sqlite3 *db, const char *tabname, const char *key_name, const char *value)
{
	char sqlstr[512];
	char *pErrMsg = NULL;

	sprintf(sqlstr, "update %s set %s='%s' where %s='%s'", tabname, TABLE_MAIN_COL_VALUE, value, TABLE_MAIN_COL_KEY, key_name);
	if(sqlite3_exec(db, sqlstr, NULL, NULL, &pErrMsg) != SQLITE_OK )
	{
		printf("%s SQLITE SQL failed :%s, %s\n", __FUNCTION__, sqlstr, pErrMsg);
		sqlite3_free(pErrMsg);
		return 0;
	}
	return 1;
}

static int sqlite_callback_cfg_info(void *data,int col_count,char **col_values,char **col_name)
{
	int k;
	db_table_main_rec.key_name[0] = '\0';
	db_table_main_rec.value[0] = '\0';
	for(k = 0; k < col_count; ++k)
	{
		if     (!strcasecmp(col_name[k], TABLE_MAIN_COL_KEY))   strncpy(db_table_main_rec.key_name,   col_values[k], 100-1);
		else if(!strcasecmp(col_name[k], TABLE_MAIN_COL_VALUE)) strncpy(db_table_main_rec.value,      col_values[k], 100-1);
	}

	//printf("%s = %s\n", db_table_main_rec.key_name, db_table_main_rec.value);
	db_table_main_load_set(db_table_main_rec.key_name, db_table_main_rec.value);
	return 0;
}
/*
 * 从数据库中取配置信息，从数据库获取程序配置信息
 *
 * 返回值：1成功，否则失败
 */
int db_table_main_load(sqlite3 *db, const char *tab_name)
{
	char *pErrMsg = NULL;
	char sqlstr[512];

	sprintf(sqlstr, "select *from %s", tab_name);
	if(sqlite3_exec(db, sqlstr, sqlite_callback_cfg_info, NULL, &pErrMsg) != SQLITE_OK)
		return 0;

	return 1;
}

/* 根据关键字修改记录的值 */
int db_table_main_update(sqlite3 *db, const char *tab_name, const char *key_name, const char *value)
{
	if(internal_db_table_main_row_query(db, tab_name, key_name) <= 0)
		return internal_db_table_main_insert(db, tab_name, key_name, value);

	return internal_db_table_main_update(db, tab_name, key_name, value);
}
int db_table_main_update_old(sqlite3 *db, const char *tab_name, const char *key_name, const char *value)
{
	return internal_db_table_main_update(db, tab_name, key_name, value);
}

/* 在数据库中创建表 */
int db_table_main_create(sqlite3 *db, const char *tab_name)
{
	int result = 1;
	char sqlstr[512];
	if(!db || !tab_name)
		return 0;

	sprintf(sqlstr, "create table %s(%s nvarchar(100),%s nvarchar(100))", tab_name, TABLE_MAIN_COL_KEY, TABLE_MAIN_COL_VALUE);
	if(sqlite3_exec(db, sqlstr, NULL, NULL, NULL) != SQLITE_OK )
	{
		result = 0;
	}
	if(result)
	{
		db_table_main_insert_init_value(db, tab_name);
		g_table_name_config = util_strcpy(tab_name);
	}

	printf("SQLITE create table %s %s\n", tab_name, result ? "success":"failed");
	return result;
}

/*
 * 把程序配置信息转换为一串字符
 *
 * 字符串以分号（;）分割
 * 参数：
 *      cfg_string  保存配置字符串的缓存
 *      size        长度
 *
 * 返回值：0失败，否则成功
 */

/*sprintf(cfg_string,"%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s",cfgInfo->systemId,cfgInfo->deviceId,cfgInfo->hostName,cfgInfo->ipType,cfgInfo->ipAddr,cfgInfo->port1,\
               cfgInfo->port2,cfgInfo->mask,cfgInfo->gateWay,cfgInfo->dnsSuffix,cfgInfo->domain1,cfgInfo->domain2,cfgInfo->serverType,cfgInfo->serverIp,cfgInfo->serverPort,cfgInfo->severDns);
 */
int db_table_main_get_cfg_info_string(char *cfg_string, int size)
{
	int len;
	if(!cfg_string || size < 64)
		return 0;

	memset(cfg_string, 0, size);
	sprintf(cfg_string,"%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s",db_table_main_get_value(SQLITE_KEY_SYSTEM_ID),\
			                                                             db_table_main_get_value(SQLITE_KEY_DEVICE_ID),\
			                                                             db_table_main_get_value(SQLITE_KEY_HOST_NAME),\
			                                                             db_table_main_get_value(SQLITE_KEY_IP_TYPE),\
			                                                             db_table_main_get_value(SQLITE_KEY_IP_ADDRESS),\
			                                                             db_table_main_get_value(SQLITE_KEY_PORT_1),\
			                                                             db_table_main_get_value(SQLITE_KEY_PORT_2),\
			                                                             db_table_main_get_value(SQLITE_KEY_IP_MASK),\
			                                                             db_table_main_get_value(SQLITE_KEY_GATE_WAY),\
			                                                             db_table_main_get_value(SQLITE_KEY_DNS_SUFFIX),\
			                                                             db_table_main_get_value(SQLITE_KEY_DOMAIN_1),\
			                                                             db_table_main_get_value(SQLITE_KEY_DOMAIN_2),\
			                                                             db_table_main_get_value(SQLITE_KEY_SERVER_TYPE),\
			                                                             db_table_main_get_value(SQLITE_KEY_SERVER_IP),\
			                                                             db_table_main_get_value(SQLITE_KEY_SERVER_PORT),\
			                                                             db_table_main_get_value(SQLITE_KEY_SERVER_DNS));
	len = strlen(cfg_string);
	cfg_string[len] = '\0';
	return 1;
}

int db_table_main_server_config(SERVER_CONFIG *lp_server_config)
{
	if(!lp_server_config)
		return 0;

	lp_server_config->server_type     = atoi(db_table_main_get_value(SQLITE_KEY_SERVER_TYPE));
	lp_server_config->port1           = atoi(db_table_main_get_value(SQLITE_KEY_PORT_1));
	lp_server_config->port2           = atoi(db_table_main_get_value(SQLITE_KEY_PORT_2));
	lp_server_config->dst_server_port = atoi(db_table_main_get_value(SQLITE_KEY_SERVER_PORT));

	lp_server_config->dst_server_ip   = util_strcpy(db_table_main_get_value(SQLITE_KEY_SERVER_IP));
	return 1;
}

const char *db_table_main_get(const char *key_name)
{
	return db_table_main_get_value(key_name);
}
