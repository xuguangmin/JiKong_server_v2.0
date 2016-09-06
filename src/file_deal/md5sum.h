/*
 * md5sum.h
 *
 *  Created on: 2013-7-8
 *      Author: flx
 *      md5sum ./irda.db >irda.md5
 */

#ifndef __MD5SUM_H__
#define __MD5SUM_H__

extern int calcu_md5_checksum(const char *filename, char *sum_buffer, int buf_size);

#endif /* __MD5SUM_H__ */
