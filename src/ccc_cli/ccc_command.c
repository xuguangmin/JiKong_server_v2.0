/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文件名    ：ccc_command.c
  创建者    ：贾延刚
  生成日期   ：2012-10-28
  功能描述   : 实现集控机的命令行接口
             将会使用程序中其他功能模块提供的相应功能
  函数列表   :
  修改历史   :

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/reboot.h>
#include <errno.h>
#include <time.h>

#include "ccc_command.h"
#include "sysconfig.h"
#include "configure.h"
#include "device_controller/device_manager.h"
#include "tcp_server/wol.h"


/*
 * 1.3.4.399  （使用了新的svn，数字重新计算）
 * curl库更换为libghttp，为onvif提供了接口（比较粗略）
 * libhmui_event.so的生成集成到了项目中，一起生成
 * 修改了设置控件属性的协议的一个bug：一个值错了。
 * 动态库中提供了对控件设置属性的接口
 *
 * 1.3.3.1071
 * 配置部分代码做了大幅度修改
 * 并增加了命令date, test, relay，第二个网口IP的设置也完全正常了
 *
 * 1.3.2.1026
 * 修改了继电只能打开一个的bug
 *
 * 1.3.2.999
 * 继电功能生效
 *
 *
 * 1.3.1.971
 * telnet连接已经可以正常使用
 * 通过这个功能实现了远程关机
 *
 *1.3.0.799
 *    增加wol功能
 *1.3.0.796
 */
const char *ccc_version_info = "1.3.4.400";

static const char *ccc_get_version()
{
    return ccc_version_info;
}

static void version(void)
{
    printf("\nPhilisense CCC version:  %s\n", ccc_get_version());
    printf("Web:           http://www.philisense.com/\n");
    printf("Email:         support@philisense.com\n\n");
}

static int ccc_command_shell(int argc, char *argv[])
{
	pid_t pid = fork();

	printf("command : %s, pid :%d\n", "shell", pid);
	if(pid < 0)
	{
		printf("command shell error\n");
		return 0;
	}
	else if(0 == pid)
	{
		int ret = execvp("/bin/sh", NULL); //system函数
		printf("execvp ret :%d\n", ret);
		if(ret < 0)
		{
			//printf("execvp error, \n");
			printf("execvp error, errno:%d error:%s\n", errno, strerror(errno));
			return 0;
		}
		exit(0);
	}
	else
	{
		int status;
		wait(&status);
	}
	return 1;
}

static int ccc_command_reboot(int argc, char *argv[])
{
	printf("command : %s", "reboot");
	system("reboot");
	//reboot(LINUX_REBOOT_CMD_RESTART);
	return 1;
}

static int ccc_command_exit(int argc, char *argv[])
{
	//exit(0);
	return 1;
}
static int ccc_command_ps(int argc, char *argv[])
{
	system("ps");
	return 1;
}

static int ccc_command_modify_ip_address(int lan_no, const char *ip_addr, const char *net_mask)
{
	if(!set_ipaddr_netmask2(lan_no, ip_addr, net_mask))
		return 0;

	return modify_cfg_ip_address_ex(lan_no, ip_addr, net_mask);
}

static int ccc_command_ethip_modify(int lan_no, int argc, char *argv[])
{
	unsigned val, netmsk;

	if(argc < 1 || lan_no < 1 || lan_no > 2)
		return 0;

	//printf("%s\n", __FUNCTION__);
	if(1 == argc)
	{
		char ethname[64];
		//unsigned int cnt;

		sprintf(ethname, "eth%d", lan_no-1);
		val = get_ipaddr(ethname);
		netmsk = get_netaddr(ethname);

		/*for(cnt = 0; cnt < 32; cnt++)
		{
			if(!(netmsk & (0x1 << (31-cnt))))
				break;
		}*/
		printf("%s : %03d.%03d.%03d.%03d/", argv[0], (val >> 24) & 0xff, (val >> 16) & 0xff, (val >> 8) & 0xff, val & 0xff);
		val = netmsk;
		printf("%03d.%03d.%03d.%03d\n", (val >> 24) & 0xff, (val >> 16) & 0xff, (val >> 8) & 0xff, val & 0xff);
		return 1;
	}

	if(2 == argc)
	{
		if(!str_to_ip_address(argv[1]))
		{
			printf("%s parameter error\n", argv[0]);
			return 0;
		}
		return ccc_command_modify_ip_address(lan_no, argv[1], "255.255.255.0");
	}
	else if(3 == argc)
	{
		if(!str_to_ip_address(argv[1]) || !str_to_ip_address(argv[2]))
		{
			printf("%s parameter error\n", argv[0]);
			return 0;
		}
		return ccc_command_modify_ip_address(lan_no, argv[1], argv[2]);
	}
	return 0;
}

static int ccc_command_ethip_2(int argc, char *argv[])
{
	return ccc_command_ethip_modify(2, argc, argv);
}
static int ccc_command_ethip_1(int argc, char *argv[])
{
	return ccc_command_ethip_modify(1, argc, argv);
}
static int ccc_command_ethip(int argc, char *argv[])
{
	return ccc_command_ethip_1(argc, argv);
}


static int ccc_command_list(int argc, char *argv[])
{
	char buffer[4096];

	print_server_info();
	printf("\n");

	buffer[0] = '\0';
	if(get_device_stat_info(buffer, 4096))
	{
		printf("%s\n", buffer);
	}
	return 1;
}

static int ccc_command_version(int argc, char *argv[])
{
	if(argc < 1)
		return 0;

	version();
	return 1;
}

static int ccc_command_serial(int argc, char *argv[])
{
	int serial_no;
	if(argc < 3)
		return 0;

	serial_no = atoi(argv[1]);
	if (strlen(argv[2]) > 0)
	{
		if(send_data_to_comm_serial(serial_no, (unsigned char *)argv[2], strlen(argv[2])))
			return 1;
		else
			printf("send_data_to_comm_serial error\n");
	}
	return 0;
}

static int ccc_command_relay(int argc, char *argv[])
{
	int relay_no;
	char *onoff;
	if(argc < 3)
		return 0;

	relay_no = atoi(argv[1]);
	onoff = argv[2];
	if (strlen(onoff) > 0)
	{
		int b_status = 0;
		if(0 == strcasecmp(onoff, "on"))
			b_status = 1;
		else if(0 == strcasecmp(onoff, "off"))
			b_status = 0;
		else {
			return 0;
		}

		if(send_data_to_comm_relay(relay_no, b_status))
			return 1;
		else
			printf("send_data_to_comm_relay error\n");
	}
	return 0;
}

static int ccc_command_wol(int argc, char *argv[])
{
	char *mac_addr;
	if(argc < 2)
		return 0;

	mac_addr = argv[1];
	if(wake_on_lan(mac_addr))
	{
		printf("wake_on_lan %s\n", mac_addr);
		return 1;
	}
	else{
		printf("wake_on_lan error\n");
	}

	return 0;
}

const unsigned char g_test_infrared_data[] =
{4,87,85,2,35,30,0,72,119,0,208,230,0,68,142,0,207,143,0,71,85,0,64,89,0,73,157,0,64,18,0,75,63,0,63,217,0,
 74,109,0,206,16,0,71,121,0,64,175,0,74,117,0,203,213,0,70,249,0,64,194,0,75,36,0,64,225,0,75,155,0,203,119,
0,71,54,0,207,12,0,67,164,0,206,102,0,71,226,0,64,48,0,74,168,0,207,159,0,67,216,0,64,48,0,75,236,0,205,56,
0,71,220,0,62,108,0,76,99,0,62,164,0,76,7,0,206,210,0,68,205,0,206,149,0,70,177,0,62,172,0,71,66,0,68,30,0,
75,102,0,205,86,0,71,131,0,62,122,0,73,110,0,207,239,0,71,71,0,204,204,0,71,87,0,64,131,0,74,143,0,63,71,0,
75,22,0,205,51,0,71,204,0,207,151,0,67,255,0,64,164,0,74,128,12,203,25,4,87,240,2,36,51,0,71,54,0,211,85,0,
66,132,0,209,46,0,69,17,0,204,246,0,71,4,0,205,228,0,71,23,0,65,144,0,72,227,0,207,198,0,68,115,0,63,190,0,
75,194,0,206,27,0,69,228,0,63,210,0,75,219,0,62,175,0,76,38,0,63,187,0,74,109,0,64,3,0,75,125,0,207,137,0,
68,96,0,63,138,0,75,235,0,206,249,0,69,37,0,63,182,0,75,111,0,63,18,0,75,83,0,207,242,0,68,91,0,208,80,0,68,
107,0,63,107,0,75,216,0,207,131,0,67,114,0,209,38,0,68,174,0,63,41,0,75,8,0,208,47,0,68,185,0,207,98,0,68,
218,0,63,24,0,76,46,0,63,52,0,75,149,0,207,71,0,68,232,0,62,244,0,76,104,0,63,12,0,75,147,0,205,78,0,70,127,
0,63,157,0,75,116,12,206,33,4,84,80,2,38,194,0,70,204,0,205,136,0,72,109,0,204,54,0,71,87,0,64,117,0,75,19,0,
63,149,0,75,8,0,63,130,0,75,161,0,202,125,0,73,108,0,63,254,0,75,116,0,204,21,0,72,120,0,63,107,0,75,166,0,
63,99,0,75,39,0,204,37,0,71,162,0,205,83,0,70,219,0,205,14,0,72,103,0,63,38,0,76,32,0,202,165,0,72,255,0,63,
152,0,76,79,0,203,208,0,73,5,0,62,83,0,74,231,0,63,223,0,75,25,0,204,220,0,71,68,0,205,17,0,71,194,0,63,101,
0,75,31,0,64,89,0,75,14,0,204,32,0,71,220,0,63,232,0,75,149,0,204,162,0,71,162,0,204,171,0,71,26,0,64,29,0,
75,224,0,63,146,0,74,239,0,204,179,0,71,131,0,204,223,0,70,17,0,65,133,0,75,61,12,204,194,4,85,197,2,36,183,
0,73,104,0,205,20,0,68,126,0,207,73,0,71,137,0,205,67,0,71,198,0,204,90,0,72,91,0,63,196,0,74,26,0,205,86,0,
71,126,0,64,23,0,75,109,0,204,43,0,71,159,0,64,78,0,74,101,0,65,13,0,73,113,0,65,163,0,74,40,0,65,99,0,71,7,
0,206,218,0,69,231,0,66,148,0,73,11,0,205,239,0,71,237,0,65,17,0,73,143,0,65,147,0,73,99,0,203,53,0,72,213,
0,205,6,0,70,216,0,66,76,0,71,176,0,204,170,0,72,175,0,205,75,0,72,50,0,66,137,0,71,43,0,204,10,0,72,158,0,
203,252,0,72,250,0,67,9,0,70,185,0,68,138,0,70,3,0,203,105,0,72,253,0,68,121,0,70,116,0,68,232,0,70,69,0,
203,116,0,72,106,0,68,221,0,69,137
};
unsigned char g_test_serial_data[] = {" :serial test data .............................................\n"};

static int ccc_command_test(int argc, char *argv[])
{
	int k, timeout = 1000, loop = 1;
	if(argc < 1)
		return 0;

	if(argc >= 2)
	{
		timeout = atoi(argv[1]);
		if(timeout < 1)    timeout = 1;
		if(timeout > 5000) timeout = 5000;
	}
	timeout = timeout * 1000;

	if(argc >= 3)
	{
		loop = atoi(argv[2]);
		if(loop < 1)   loop = 1;
		if(loop > 100) loop = 1000;
	}

	while(loop > 0)
	{
		for(k = 0; k < 8; ++k)
		{
			send_data_to_comm_relay(k+1, 1);
			usleep(timeout);
			send_data_to_comm_relay(k+1, 0);
			usleep(timeout);
		}

		for(k = 0; k < 8; ++k)
		{
			g_test_serial_data[0] = (unsigned char)('1' + k);
			send_data_to_comm_serial(k+1, g_test_serial_data, sizeof(g_test_serial_data)/sizeof(unsigned char));
			usleep(timeout);
		}

		for(k = 0; k < 8; ++k)
		{
			send_data_to_comm_infrared(k+1, g_test_infrared_data, sizeof(g_test_serial_data)/sizeof(unsigned char));
			usleep(timeout);
		}
		loop--;
	}

	return 1;
}

static int ccc_command_date(int argc, char *argv[])
{
	if(argc < 1)
		return 0;

	if(1 == argc)
	{
		time_t seconds = time(NULL);
		struct tm *tm = gmtime(&seconds);
		printf("%s %04d-%02d-%02d %02d:%02d:%02d\n", argv[0], tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour+8, tm->tm_min, tm->tm_sec);
		return 1;
	}

	if(3 == argc)
	{
		int year, month, day;
		int hour, minute, second;
		char datestr[512];
		sprintf(datestr, "%s %s", argv[1], argv[2]);
		if(sscanf(datestr, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6)
		{
			printf("date set error.\n");
			return 0;
		}

		return set_system_time(year, month, day, hour, minute, second);
	}
	return 0;

}
/*
 * command_id  : 命令ID
 * argc        ：参数个数（不包括命令关键字）
 * argv[]      ：参数数组
 */
int ccc_command(int command_id, int argc, char *argv[])
{
	int k;
	/*printf("command_id %d : argc %d\n", command_id, argc);*/
	for(k = 0; k < argc; ++k)
	{
		if(!argv[k] || strlen(argv[k]) <= 0) return 0;
	}

	switch(command_id)
	{
	case CCC_ID_REBOOT    :return ccc_command_reboot(argc, argv);
	case CCC_ID_SHELL     :return ccc_command_shell(argc, argv);
	case CCC_ID_EXIT      :return ccc_command_exit(argc, argv);
	case CCC_ID_PS        :return ccc_command_ps(argc, argv);
	case CCC_ID_ETHIP     :return ccc_command_ethip(argc, argv);
	case CCC_ID_ETHIP_1   :return ccc_command_ethip_1(argc, argv);
	case CCC_ID_ETHIP_2   :return ccc_command_ethip_2(argc, argv);
	case CCC_ID_LIST      :return ccc_command_list(argc, argv);
	case CCC_ID_VERSION   :return ccc_command_version(argc, argv);
	case CCC_ID_SERIAL    :return ccc_command_serial(argc, argv);
	case CCC_ID_WOL       :return ccc_command_wol(argc, argv);
	case CCC_ID_RELAY     :return ccc_command_relay(argc, argv);
	case CCC_ID_TEST      :return ccc_command_test(argc, argv);
	case CCC_ID_DATE      :return ccc_command_date(argc, argv);
	}
	return 0;
}
