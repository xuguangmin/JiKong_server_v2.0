/******************************************************************************

                  版权所有 (C), 2001-2020

 ******************************************************************************
  文件名    ：db_table_main.h
  作者      ：贾延刚
  生成日期   ：2013-5-17
  功能描述   : 在一个指定数据库上创建一个保存配置信息的表

  函数列表   :
  修改历史   :

******************************************************************************/
#ifndef __DB_TABLE_MAIN_H__
#define __DB_TABLE_MAIN_H__

#include "sqlite3.h"
#include "config_info.h"

extern int db_table_main_create(sqlite3 *db, const char *tab_name);
extern int db_table_main_load(sqlite3 *db, const char *tab_name);
extern int db_table_main_update(sqlite3 *db, const char *tab_name, const char *key_name, const char *value);
extern int db_table_main_update_old(sqlite3 *db, const char *tab_name, const char *key_name, const char *value);

extern const char *db_table_main_get(const char *key_name);

extern int db_table_main_server_config(SERVER_CONFIG *lp_server_config);
extern int db_table_main_get_cfg_info_string(char *cfg_string, int size);
extern int db_table_main_modify_cfg_info_all(char *cfg_string, int size);

#endif /* __DB_TABLE_MAIN_H__ */
