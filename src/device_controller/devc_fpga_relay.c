/******************************************************************************

  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : devc_fpga_relay.c
  �� �� ��   : ����
  ��    ��   : chen zhi tao
  ��������   : 2011��4��2��������
  ����޸�   :
  ��������   : �̵����豸�ӿ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2011��4��2��������
    ��    ��   : chen zhi tao
    �޸�����   : �����ļ�
    1111 1111
    ������������Ϊ1-8�̵�ڣ�1Ϊ����0Ϊ��

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include    <stdio.h>      /*��׼�����������*/
#include    <stdlib.h>     /*��׼�����ⶨ��*/
#include    <unistd.h>     /*Unix ��׼��������*/
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>      /*�ļ����ƶ���*/
#include    <termios.h>    /*PPSIX �ն˿��ƶ���*/
#include    <errno.h>      /*����Ŷ���*/
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
