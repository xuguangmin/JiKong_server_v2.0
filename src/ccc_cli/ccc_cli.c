/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文件名    ：ccc_cli.c
  创建者    ：贾延刚
  生成日期   ：2012-10-28
  功能描述   : 集控机的命令行接口
             该文件中的函数解析传入的命令字符串，检查合法性，并提取出有效的
             命令元素，提供给下层接口
  函数列表   :
  修改历史   :

******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ccc_command.h"


#define CCC_KEY_HELP                  "help"
#define CCC_KEY_LIST                  "list"
#define CCC_KEY_SHELL                 "shell"
#define CCC_KEY_REBOOT                "reboot"
#define CCC_KEY_EXIT                  "exit"
#define CCC_KEY_PS                    "ps"
#define CCC_KEY_ETHIP                 "ethip"
#define CCC_KEY_ETHIP_1               "ethip1"
#define CCC_KEY_ETHIP_2               "ethip2"
#define CCC_KEY_VERSION               "version"
#define CCC_KEY_SERIAL                "serial"
#define CCC_KEY_WOL                   "wol"
#define CCC_KEY_RELAY                 "relay"
#define CCC_KEY_TEST                  "test"
#define CCC_KEY_DATE                  "date"

static struct command_info {
	char *name;
	int   id;
	char *helpinfo;
}ccc_command_set[] = {
	{CCC_KEY_HELP,	     CCC_ID_HELP,       "show help." },
//	{CCC_KEY_SHELL,      CCC_ID_SHELL,      "open linux shell."},
	{CCC_KEY_REBOOT,     CCC_ID_REBOOT,     "reboot linux."},
//  {CCC_KEY_EXIT,       CCC_ID_EXIT,       "exit from app"},
	{CCC_KEY_LIST,       CCC_ID_LIST,       "show list." },
	{CCC_KEY_PS,         CCC_ID_PS,         "linux ps command." },
	{CCC_KEY_ETHIP,      CCC_ID_ETHIP,      "LAN 1 IP address. format :ethip  [IP] [netmask]" },
	{CCC_KEY_ETHIP_1,    CCC_ID_ETHIP_1,    "LAN 1 IP address. format :ethip1 [IP] [netmask]" },
	{CCC_KEY_ETHIP_2,    CCC_ID_ETHIP_2,    "LAN 2 IP address. format :ethip2 [IP] [netmask]" },
	{CCC_KEY_VERSION,    CCC_ID_VERSION,    "version information." },
	{CCC_KEY_SERIAL,     CCC_ID_SERIAL,     "send data to serial. format :serial no data" },
	{CCC_KEY_WOL,        CCC_ID_WOL,        "wake on pc. format :wol mac. mac: XX:XX:XX:XX:XX:XX" },
	{CCC_KEY_RELAY,      CCC_ID_RELAY,      "send data to relay. format :relay no on/off" },
	{CCC_KEY_TEST,       CCC_ID_TEST,       "test lamp. format :test [timeout] [loop], default timeout = 500ms, loop = 1" },
	{CCC_KEY_DATE,       CCC_ID_DATE,       "system date. format :date [1900-10-20 12:20:20]" },
	{NULL,			     CCC_ID_INVALID,     NULL }
};


static struct command_info* lookup_command_info(const char *command_key)
{
	int k;
	if(!command_key || strlen(command_key) <= 0)
		return NULL;

	for(k = 0; ;k++)
	{
		if(ccc_command_set[k].name == NULL)
			break;

		if(strcasecmp(command_key, ccc_command_set[k].name) == 0)
		{
			return &(ccc_command_set[k]);
		}
	}
	return NULL;
}

static void show_help()
{
	int k;
	for(k = 0; ;k++)
	{
		if(ccc_command_set[k].name == NULL)
			break;

		if(ccc_command_set[k].helpinfo == NULL)
			printf("%-20s : %s\n", ccc_command_set[k].name, "......");
		else
			printf("%-20s : %s\n", ccc_command_set[k].name, ccc_command_set[k].helpinfo);
	}
}

int GetALineNotEndChar(char *srcstr, char *aline, char endChar)
{
	int k;
	char *p;
	if (!srcstr || !aline)
		return 0;
	if (strlen(srcstr) <= 0)
		return 0;

	k = 0;
	*aline = '\0';
	p = srcstr;
	while(*p)
	{
		if (endChar == *p)
		{
			strncpy(aline, srcstr, k);            // 截取
			aline[k] = '\0';

			p++;                                  // 跳过endChar
			strcpy(srcstr, p);
			return 1;
		}
		k++;
		p++;
	}
	return 0;
}
//
static int trim_string_left(char *buffer)
{
	char *p;
	if(!buffer || strlen(buffer) <= 0)
		return 0;

	p = buffer;
	while(*p)
	{
		if(0x20 == *p || 0x0D == *p || 0x0A == *p)
			p++;
		else
			break;
	}
	if(p != buffer)
	{
		strcpy(buffer, p);
		return 1;
	}
	return 0;
}

static int skip_char(char ch)
{
	if(0x20 == ch || 0x0D == ch || 0x0A == ch)
		return 1;

	return 0;
}

static int jikong_cli_2(int argc, char *argv[])
{
	struct command_info *cmd_info;
	if(argc < 1)
		return 0;

	cmd_info = lookup_command_info(argv[0]);
	if(!cmd_info)
	{
		printf("invalid command.\n");
		return 0;
	}

	switch(cmd_info->id)
	{
	case CCC_ID_HELP:
		show_help();
		break;

	default:
		ccc_command(cmd_info->id, argc, (argc >= 1) ? argv:NULL);
		break;
	}
	return 1;
}

int ccc_cli(char *buffer, int size)
{
	int argc;
	char *p;
	char *argv[16];
	if(!buffer || size <= 0)
		return 0;

	trim_string_left(buffer);
	size = strlen(buffer);
	if(size <= 0)
		return 0;

	argc = 0;
	p = buffer;
	argv[0] = p;
	while(*p)
	{
		if(skip_char(*p))
		{
			*p = '\0';
			p++;
			while(*p)
			{
				if(!skip_char(*p)) break;
				p++;
			}

			if(strlen(p) <= 0) break;

			argc++;
			argv[argc] = p;
			continue;
		}
		p++;
	}
	argc++;

	if(!jikong_cli_2(argc, argv))
	{
		printf("command not success.\n");
		return 0;
	}
	return 1;
}
