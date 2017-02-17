/*
 * app_console.c
 *
 *  Created on: 2012-10-27
 *      Author: jiayg
 */
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <sys/poll.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include "ccc_cli.h"
#include "cmdedit.h"

static char szPrompt[] = {"->"};


void app_console_start2()
{
	char buffer[8192];
	char szTemp[] = {"\r\nPhilisence ccc console...\r\n\r\n"};

	write(STDOUT_FILENO, szTemp, strlen(szTemp));

	// Çå¿Õ sdtout »º³å
	fflush(stdout);
	fflush(stdin);
	memset(buffer, 0, 8192);

	while(1)
	{
		int len = cmdedit_read_input(szPrompt, buffer);
		if(len > 0 && buffer[len - 1] == '\n')
		{
			buffer[len-1] = '\0';
			ccc_cli(buffer, len);
		}
		else {
			//break;
		}
	}
	printf("*** cmdedit_read_input() detect ^D\n");
}

extern ssize_t safe_read(int fd, void *buf, size_t count);
static int console_command_loop(unsigned char *buffer, int buf_size)
{
	unsigned char nReturn;
	int index = 0;
	while(1)
	{
		if (safe_read(0, &nReturn, 1) < 1)
			continue;

		if(nReturn == 0x0A)
		{
			break;
		}
		else
		{
			if(nReturn != 0xFF)
			{
				// printf("%c\n", nReturn);
				buffer[index] = (unsigned char)nReturn;
				index++;
				if(index >= buf_size)
					break;
			}
		}
	}
	return index;
}

void app_console_start()
{
	int len;
	unsigned char buffer[8192];
	char szTemp[] = {"\r\nPhilisense ccc console...\r\n\r\n"};

	write(STDOUT_FILENO, szTemp, strlen(szTemp));

	// Çå¿Õ sdtout »º³å
	fflush(stdout);
	fflush(stdin);
	memset(buffer, 0, 8192);

	while(1)
	{
		write(STDOUT_FILENO, szPrompt, strlen(szPrompt));
		len = console_command_loop(buffer, 8192);
		if(len > 0)
		{
			buffer[len] = '\0';
			ccc_cli((char *)buffer, len);
		}
	}
}
