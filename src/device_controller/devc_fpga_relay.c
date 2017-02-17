/******************************************************************************

  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : devc_fpga_relay.c
  版 本 号   : 初稿
  作    者   : chen zhi tao
  生成日期   : 2011年4月2日星期六
  最近修改   :
  功能描述   : 继电器设备接口
  函数列表   :
  修改历史   :
  1.日    期   : 2011年4月2日星期六
    作    者   : chen zhi tao
    修改内容   : 创建文件
    1111 1111
    从右向左，依次为1-8继电口，1为开，0为关

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include    <stdio.h>      /*标准输入输出定义*/
#include    <stdlib.h>     /*标准函数库定义*/
#include    <unistd.h>     /*Unix 标准函数定义*/
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>      /*文件控制定义*/
#include    <termios.h>    /*PPSIX 终端控制定义*/
#include    <errno.h>      /*错误号定义*/
#include 	"FLXCommon/flxtypes.h"

static int g_fd_relay = -1;
static unsigned char g_relay_mask = 0x0;

static int devc_fpga_relay_open()
{
	int fd = open("/dev/FPGA_Jikong19", O_RDWR);
	if(fd < 0)
	{
		perror("Cannot Open relay!\n");
		return -1;
	}
	printf("FPGA relay opened!\n");
	return fd;
}

static void devc_fpga_relay_close()
{
	close(g_fd_relay);
	g_fd_relay = -1;
}

int devc_fpga_relay_write_data(int relay_no, int b_status)
{
	unsigned char value[1];
	unsigned char current;
	if(relay_no < 1 || relay_no > 8)
		return 0;

	if(g_fd_relay < 0) g_fd_relay = devc_fpga_relay_open();
	if(g_fd_relay < 0)
		return 0;

	current = 0x1 << (relay_no-1);
	if (b_status)
	{
		g_relay_mask |= current;
	}
	else
	{
		g_relay_mask &= ~current;
	}

	value[0] = g_relay_mask;
	if(write(g_fd_relay, value, 1) < 0)
	{
		printf("%s write fail.\n", __FUNCTION__);
		devc_fpga_relay_close();
		return 0;
	}
	printf(">R%d = 0x%X\n", relay_no, value[0]);
	return 1;
}
