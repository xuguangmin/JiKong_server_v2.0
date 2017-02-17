/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : indicator_lamp.c
  作者    : 贾延刚
  生成日期 : 2012-10

  版本    : 1.0
  功能描述 : 前面板的指示灯接口

  修改历史 :

******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define PANEL_LED_ON     0x05
#define PANEL_LED_OFF    0x03

static int g_led_fd = -1;
static int open_led()
{
	g_led_fd = open("/dev/Jikong_Led", O_RDWR); //270
	if(g_led_fd < 0)
	{
		perror("open_led() : cannot open the front panel led !\n");
		return 0;
	}
	return 1;
}

void panel_led_off()
{
	if(g_led_fd >= 0)
	{
		unsigned char ledTemp[1] = {PANEL_LED_OFF};
		write(g_led_fd, ledTemp, 1);

		close(g_led_fd);
		g_led_fd = -1;
	}
}
int panel_led_on()
{
	unsigned char ledTemp[1] = {PANEL_LED_ON};
	if(g_led_fd < 0)
	{
		if(!open_led())
		{
			g_led_fd = -1;
			return 0;
		}
	}
	return (write(g_led_fd,ledTemp, 1) >= 0) ? 1:0;
}
