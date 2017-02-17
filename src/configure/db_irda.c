/*
 * app_config.c
 *
 *  Created on: 2013-5-17
 *      Author: flx
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sqlite3.h"
#include "db_utils.h"

typedef struct __IRDA_RECORD
{
#define IRDA_DATA_LEN_MAX     4096

	char *desc;
	char *value;
}IRDA_RECORD;

#define SQLITE_COL_IRDA_DATA      "data"
#define SQLITE_COL_IRDA_DESC      "desc"

static sqlite3 *g_db_Infrared    = NULL;

static int sqlite_callback_irda_record(void *data, int col_count, char **col_values, char **col_name)
{
	int k, len;
	IRDA_RECORD *irda_rec =(IRDA_RECORD *)data;
	if(!irda_rec)
		return -1;

	for(k = 0; k < col_count; ++k)
	{
		if(!col_values[k] || !col_name[k])
			continue;

		len = strlen(col_values[k]);
		if(!strcasecmp(col_name[k], SQLITE_COL_IRDA_DESC))       /* desc 第 2 列 */
		{
			irda_rec->desc = (char *)malloc(len + 1);
			strcpy(irda_rec->desc, col_values[k]);
			irda_rec->desc[len] = '\0';
		}
		else if(!strcasecmp(col_name[k], SQLITE_COL_IRDA_DATA))  /* data 第 3 列是有效数据 */
		{
			irda_rec->value = (char *)malloc(len + 1);
			strcpy(irda_rec->value, col_values[k]);
			irda_rec->value[len] = '\0';
		}
	}
	return 0;
}

static int open_db_infrared(const char *db_filename)
{
	if(NULL == g_db_Infrared)
	{
		if(!sqlite_db_open(db_filename, &g_db_Infrared))
		{
			g_db_Infrared = NULL;
			return 0;
		}
	}
	return 1;
}

void close_db_infrared()
{
	if(g_db_Infrared != NULL)
	{
		sqlite3_close(g_db_Infrared);
		g_db_Infrared = NULL;
	}
}

/*
 * 从数据库中取红外数据
 *
 * 参数：
 *     key      999
 *     buffer   缓存
 *     size     缓存长度
 *
 * 返回值：数据长度，如果小于等于0，则未取到数据
 */
int read_infrared_table(const char *db_filename, int key, char **irda)
{
	IRDA_RECORD irda_reco;
	char *pErrMsg = NULL;
	char sql_string[256];
	int data_len;
	if(!db_filename || !irda)
		return 0;
	if(!open_db_infrared(db_filename))
		return 0;

	irda_reco.desc  = 0;
	irda_reco.value = 0;
	sprintf(sql_string, "select * from tab_infrared where key=%d", key);
	if(sqlite3_exec(g_db_Infrared, sql_string, sqlite_callback_irda_record, (void*)&irda_reco, &pErrMsg) != SQLITE_OK)
		return 0;

	if(!irda_reco.value) /* || !irda_reco.desc) */
		return 0;

	data_len = strlen(irda_reco.value);
	if(data_len <= 0)
		return 0;

	*irda = irda_reco.value;
	irda_reco.value = NULL;
	if(irda_reco.desc)  free(irda_reco.desc);
	return data_len;
}
