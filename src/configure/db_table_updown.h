/******************************************************************************

                  版权所有 (C), 2001-2020

 ******************************************************************************
  文件名    ：db_table_updown.h
  作者      ：贾延刚
  生成日期   ：2013-5-17
  功能描述   : 在一个指定数据库上创建一个保存配置信息的表

  函数列表   :
  修改历史   :

******************************************************************************/
#ifndef __DB_TABLE_UPDOWN_H__
#define __DB_TABLE_UPDOWN_H__

#include "sqlite3.h"

extern int db_table_updown_create(sqlite3 *db, const char *tab_name);
extern int db_table_updown_load(sqlite3 *db, const char *tab_name);
extern int db_table_updown_update(sqlite3 *db, const char *tab_name, const char *key_name, const char *value);

extern const char *db_table_updown_res_filename(const char *key_name);
extern int db_table_updown_file_all(UPDOWN_FILE *lp_updown_file);

#endif /* __DB_TABLE_UPDOWN_H__ */

