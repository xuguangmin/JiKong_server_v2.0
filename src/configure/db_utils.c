/*
 * db_utils.c
 *
 *  Created on: 2013-5-20
 *      Author: flx
 */
#include <stdio.h>
#include <string.h>
#include "sqlite3.h"

int sqlite_db_open(const char *zFilename, sqlite3 **ppDb)
{
	if(!zFilename)
		return 0;

	if(sqlite3_open(zFilename, ppDb) != SQLITE_OK)
	{
		fprintf(stderr,"open %s fail: %s\n", zFilename, sqlite3_errmsg(*ppDb));
		return 0;
	}
	printf("open %s success\n", zFilename);
	return 1;
}

int sqlite_table_is_exist(sqlite3 *db, const char *tab_name)
{
	int result = 0;
	int k, s, index;
	int nrow, ncol;
	char **db_result;
	char *sqlstr = "select name from sqlite_master where type = \"table\" order by \"name\"";
	if(!db || !tab_name || strlen(tab_name) <= 0)
		return 0;

	result = sqlite3_get_table(db, sqlstr, &db_result, &nrow, &ncol, NULL);
	if(result != SQLITE_OK)
	{
		sqlite3_free_table(db_result);
		return 0;
	}

	//printf("nrow=%d ncol = %d\n", nrow, ncol);

	index = ncol;
	for(k = 0; k < nrow; ++k)
	{
		for(s = 0; s < ncol; ++s)
		{
			//printf("name %s: value %s\n", db_result[s], db_result[index]);
			//printf("--------- table : %s ---------\n", db_result[index]);
			if(!strcasecmp(db_result[index], tab_name))
			{
				result = 1;
				break;
			}
			index++;
		}
	}
	sqlite3_free_table(db_result);
	return result;
}
