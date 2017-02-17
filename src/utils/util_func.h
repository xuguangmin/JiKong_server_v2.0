/*
 * util_func.h
 *
 *  Created on: 2012-11-20
 *      Author: flx
 *
 *      一些工具类函数
 */

#ifndef __UTIL_FUNC_H__
#define __UTIL_FUNC_H__

extern char *util_strcpy(const char *srcstr);
extern const char *util_strcasestr(const char *srcstr, const char *substr);
extern unsigned int util_get_m_second();

extern void trim_left_space(char *srcstr);
extern void output_byte_array(const unsigned char *buffer, int size, int max_output);
extern int four_bytes_to_int(unsigned char *data, int size);
extern int int_to_four_bytes(int value, unsigned char *buffer, int size);

#endif /* __UTIL_FUNC_H__ */
