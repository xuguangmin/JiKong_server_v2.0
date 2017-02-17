/*
 * util_log.h
 *
 *  Created on: 2012-11-1
 *      Author: flx
 */

#ifndef __UTIL_LOG_H__
#define __UTIL_LOG_H__

#include <stdio.h>

#define EXTERNAL_TEST
#ifdef EXTERNAL_TEST
	#define CCC_DEBUG_OUT(fmt, args...) printf(fmt, ##args);
#else
	#define CCC_DEBUG_OUT(fmt, args...)
#endif

#define CCC_LOG_OUT(fmt, args...) \
		printf(fmt, ##args);

extern void ccc_log(const char *format, ...);


#endif /* __UTIL_LOG_H__ */
