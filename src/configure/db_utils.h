/*
 * db_utils.h
 *
 *  Created on: 2013-5-20
 *      Author: flx
 */

#ifndef __DB_UTILS_H__
#define __DB_UTILS_H__

#include "sqlite3.h"

extern int sqlite_db_open(const char *zFilename, sqlite3 **ppDb);
extern int sqlite_table_is_exist(sqlite3 *db, const char *tab_name);

#endif /* __DB_UTILS_H__ */
