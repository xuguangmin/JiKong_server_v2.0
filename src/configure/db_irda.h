/*
 * app_config.h
 *
 *  Created on: 2013-5-17
 *      Author: flx
 */

#ifndef __DB_IRDA_H__
#define __DB_IRDA_H__

extern int read_infrared_table(const char *db_filename, int key, char **irda);
extern void close_db_infrared();

#endif /* __DB_IRDA_H__ */
