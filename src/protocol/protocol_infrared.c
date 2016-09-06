/*
 * protocol_infrared.c
 *
 *  Created on: 2013-1-29
 *      Author: flx
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IRDA_DELIM      ";"

static int calcu_count_of_char(const char *srcstr, char ch)
{
	int k, result = 0;
	if(!srcstr || strlen(srcstr) <= 0)
		return 0;

	for(k = 0; k < strlen(srcstr); ++k)
	{
		if(ch == srcstr[k]) result++;
	}
	return result;
}

int protocol_infrared_transfer(char *srcstr, unsigned char **irda_data)
{
	int len = 0;
	char *p;
	unsigned char *buffer;
	if(!srcstr || strlen(srcstr) <= 0 || !irda_data)
		return 0;

	len = calcu_count_of_char(srcstr, ';') + 10;
	buffer = (unsigned char *)malloc(len);
	if (!buffer)
		return 0;

	p = strtok(srcstr, IRDA_DELIM);
	if(!p)
		return 0;

	len = 0;
	buffer[len] = (unsigned char)(atoi(p));
	len++;
	while((p = strtok(NULL, IRDA_DELIM)))
	{
		buffer[len] = (unsigned char)(atoi(p));
		len++;
	}
	*irda_data = buffer;
	return len;
}


