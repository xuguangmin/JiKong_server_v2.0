/*
 * util_log.c
 *
 *  Created on: 2012-11-29
 *      Author: flx
 */
#include <stdio.h>
#include <stdarg.h>
#include "ipc_msg.h"

/*
void ccc_log(const char *format, ...)
{
	va_list args;
	char buffer[1000];

	va_start(args, format);
	vsnprintf(buffer, 1000, format, args);
	va_end(args);
	//printf("%s\n", buffer);
}
 */
void ccc_log(const char *format, ...)
{
	va_list args;
	//char buffer[1000];
	struct ccc_msg msg;

	va_start(args, format);
	vsnprintf(msg.msg_data, BUFSIZ, format, args);
	va_end(args);

	//printf("msg.msg_data %s\n", msg.msg_data);
	msg.msg_type = 1;
	send_msg_to_console(&msg);
	//printf("%s\n", buffer);
}

