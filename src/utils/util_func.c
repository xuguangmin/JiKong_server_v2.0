/*
 * util_func.c
 *
 *  Created on: 2012-11-20
 *      Author: flx
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/*
 * 复制srcstr，
 * 内部会分配内存，返回值的内存需要使用者释放
 */
char *util_strcpy(const char *srcstr)
{
	char *result = NULL;
	if(srcstr)
	{
		int len = strlen(srcstr);
		if(len > 0)
		{
			result = (char *)malloc(len + 1);
			if(result)
			{
				strcpy(result, srcstr);
				result[len] = '\0';
			}
		}
	}
	return result;
}

const char *util_strcasestr(const char *srcstr, const char *substr)
{
	int len;
	if(!srcstr || !substr)
		return NULL;

    len = strlen(substr);
    if(len <= 0 || strlen(srcstr) <= 0)
        return NULL;                                   /* 这里我并未照strstr一样返回str，而是返回NULL */

    while(*srcstr)
    {
        if(strncasecmp(srcstr, substr, len) == 0)       /* 这里使用了可限定比较长度的strncasecmp */
            return srcstr;

        srcstr++;
    }
    return NULL;
}

/* 得到毫秒时间 */
unsigned int util_get_m_second()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec/1000);
}

void trim_left_space(char *srcstr)
{
	char *p;
	if(!srcstr || strlen(srcstr) <= 0)
		return;

	p = srcstr;
	while(*p)
	{
		if(*p != 0x20) break;
		p++;
	}

	if(p != srcstr) strcpy(srcstr, p);
}

/*
 * 参数：
 *
 *      max_output  最多输出多少数据
 */
void output_byte_array(const unsigned char *buffer, int size, int max_output)
{
	const unsigned char *p = buffer;
	int out_len = (size > max_output) ? max_output:size;
	while(out_len > 0)
	{
		printf("%02X", *p);
		out_len--;
		p++;
	}

	printf("\n");
}

int four_bytes_to_int(unsigned char *data, int size)
{
	if(!data || size < 4)
		return 0;

	return ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
}
int int_to_four_bytes(int value, unsigned char *buffer, int size)
{
	if(!buffer || size < 4)
		return 0;

	buffer[0] = (value >> 24) & 0xFF;
	buffer[1] = (value >> 16) & 0xFF;
	buffer[2] = (value >> 8) & 0xFF;
	buffer[3] =  value & 0xFF;
	return 1;
}

