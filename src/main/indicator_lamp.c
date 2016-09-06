/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : indicator_lamp.c
  ����    : ���Ӹ�
  �������� : 2012-10

  �汾    : 1.0
  �������� : ǰ����ָʾ�ƽӿ�

  �޸���ʷ :

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
