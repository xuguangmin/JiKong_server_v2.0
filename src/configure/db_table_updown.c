/******************************************************************************

                  版权所有 (C), 2001-2020

 ******************************************************************************
  文件名    ：db_table_updown.c
  作者      ：贾延刚
  生成日期   ：2013-5-17
  功能描述   : 在一个指定数据库上创建一个保存配置信息的表

  函数列表   :
  修改历史   :

******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "sqlite3.h"
#include "sqlite_key.h"
#include "util_func.h"
#include "config_info.h"

static char *g_table_name_updown = NULL;

/*
 * 表的初始值
 * 纵向排列，以关键字识别每一行
 * 如果要在表中增加行，在这儿增加
 */
static struct _db_table_updown_init_value {
	char *key_name;
	char  value[UPDOWN_FILENAME_LEN_MAX];
}db_table_updown_init_value[] = {
	{SQLITE_KEY_FILE_ZIP,	     "LT-7000C.zip" },
	{SQLITE_KEY_FILE_SO,         "libhmui_event.so" },
	{SQLITE_KEY_FILE_LUA,        "editevent.lua" },
	{SQLITE_KEY_FILE_DB,         "irda.db" },
	{NULL,                       "" }
};

static struct __db_table_updown_rec
{
	char key_name[100];
	char value[100];
}db_table_updown_rec;

/* 修改记录的值 */
static int db_table_updown_load_set(const char *key_name, const char *new_value)
{
	int k;
	if(!key_name || !new_value || strlen(key_name) <= 0 || strlen(new_value) <= 0)
		return 0;

	for(k = 0; ;k++)
	{
		if(db_table_updown_init_value[k].key_name == NULL)
			break;

		if(!strcasecmp(db_table_updown_init_value[k].key_name, key_name))
		{
			strcpy(db_table_updown_init_value[k].value, new_value);
			return 1;
		}
	}
	return 0;
}
/* 获取记录的值 */
static char *db_table_updown_get_value(const char *key_name)
{
	int k;
	if(!key_name || strlen(key_name) <= 0)
		return NULL;

	for(k = 0; ;k++)
	{
		if(db_table_updown_init_value[k].key_name == NULL)
			break;

		if(!strcasecmp(db_table_updown_init_value[k].key_name, key_name))
		{
			return db_table_updown_init_value[k].value;
		}
	}
	return NULL;
}

static int db_table_updown_insert_init_value(sqlite3 *db, const char *tab_name)
{
	int k, sqlResult;
	char sqlstr[512];
	if(!db || !tab_name)
		return 0;

	for(k = 0; ;k++)
	{
		if(db_table_updown_init_value[k].key_name == NULL)
			break;

		sprintf(sqlstr, "insert into %s values ('%s','%s')", tab_name, db_table_updown_init_value[k].key_name, db_table_updown_init_value[k].value);
		sqlResult = sqlite3_exec(db, sqlstr, NULL, NULL, NULL);
		if(sqlResult != SQLITE_OK )
		{
			printf("%s SQLITE SQL failed :%s\n", __FUNCTION__, sqlstr);
			return 0;
		}
	}
	return 1;
}

/* 根据关键字修改记录的值 */
static int internal_db_table_updown_update(sqlite3 *db, const char *tabname, const char *key_name, const char *value)
{
	char sqlstr[512];
	char *pErrMsg = NULL;

	sprintf(sqlstr, "update %s set %s='%s' where %s='%s'", tabname, TABLE_UPDOWN_COL_VALUE, value, TABLE_UPDOWN_COL_NAME, key_name);
	if(sqlite3_exec(db, sqlstr, NULL, NULL, &pErrMsg) != SQLITE_OK )
	{
		printf("%s SQLITE SQL failed :%s, %s\n", __FUNCTION__, sqlstr, pErrMsg);
		sqlite3_free(pErrMsg);
		return 0;
	}
	return 1;
}

static int sqlite_callback_res_file(void *data,int col_count,char **col_values,char **col_name)
{
	int k;
	db_table_updown_rec.key_name[0] = '\0';
	db_table_updown_rec.value[0] = '\0';
	for(k = 0; k < col_count; ++k)
	{
		if     (!strcasecmp(col_name[k], TABLE_UPDOWN_COL_NAME))  strncpy(db_table_updown_rec.key_name,   col_values[k], 100-1);
		else if(!strcasecmp(col_name[k], TABLE_UPDOWN_COL_VALUE)) strncpy(db_table_updown_rec.value,      col_values[k], 100-1);
	}

	//printf("%s = %s\n", db_table_updown_rec.key_name, db_table_updown_rec.value);
	db_table_updown_load_set(db_table_updown_rec.key_name, db_table_updown_rec.value);
	return 0;
}
/*
 * 从数据库中取配置信息
 *
 * 返回值：1成功，否则失败
 */
int db_table_updown_load(sqlite3 *db, const char *tab_name)
{
	char *pErrMsg = NULL;
	char sqlstr[512];

	//g_ix = 0;
	sprintf(sqlstr, "select *from %s", tab_name);
	if(sqlite3_exec(db, sqlstr, sqlite_callback_res_file, NULL, &pErrMsg) != SQLITE_OK)
		return 0;

	return 1;
}

/* 根据关键字修改记录的值 */
int db_table_updown_update(sqlite3 *db, const char *tab_name, const char *key_name, const char *value)
{
	return internal_db_table_updown_update(db, tab_name, key_name, value);
}

/* 在数据库中创建表 */
int db_table_updown_create(sqlite3 *db, const char *tab_name)
{
	int result = 1;
	char sqlstr[512];
	if(!db || !tab_name)
		return 0;

	sprintf(sqlstr, "create table %s(%s nvarchar(100),%s nvarchar(100))", tab_name, TABLE_UPDOWN_COL_NAME, TABLE_UPDOWN_COL_VALUE);
	if(sqlite3_exec(db, sqlstr, NULL, NULL, NULL) != SQLITE_OK )
	{
		result = 0;
	}
	if(result)
	{
		db_table_updown_insert_init_value(db, tab_name);
		g_table_name_updown = util_strcpy(tab_name);
	}

	printf("SQLITE create table %s %s\n", tab_name, result ? "success":"failed");
	return result;
}

const char *db_table_updown_res_filename(const char *key_name)
{
	return db_table_updown_get_value(key_name);
}

int db_table_updown_file_all(UPDOWN_FILE *lp_updown_file)
{
	int k,ix;
	if(!lp_updown_file)
		return 0;

	ix = 0;
	for(k = 0; ;k++)
	{
		if(db_table_updown_init_value[k].key_name == NULL)
			break;
		if(ix >= UPDOWN_FILE_COUNT_MAX)
			break;

		strcpy(lp_updown_file->res[ix].file_type, db_table_updown_init_value[k].key_name);
		strcpy(lp_updown_file->res[ix].file_name, db_table_updown_init_value[k].value);
		ix++;
	}
	lp_updown_file->rec_count = ix;
	return 1;
}
