/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : devc_fpga_serial_port.c
  ����    : chen zhi tao
  �������� : 2011��3��30��������

  �汾    : 1.0
  �������� : fpga���ڽӿ�


  �޸���ʷ :

******************************************************************************/

#include <stdio.h>      /*��׼�����������*/
#include <stdlib.h>     /*��׼�����ⶨ��*/
#include <unistd.h>     /*Unix ��׼��������*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /*�ļ����ƶ���*/
#include <termios.h>    /*PPSIX �ն˿��ƶ���*/
#include <errno.h>      /*����Ŷ���*/
#include <sys/ioctl.h>
#include <string.h>
#include "FLXCommon/flxtypes.h"

typedef struct FPGA_UART_INIT
{
	int           band_rate;
	unsigned char data_bits;       //5,6,7,8
	unsigned char parity;          // 0--no, 1--odd, 2--even, 3--force parity "1", 4--force parity "0"
	unsigned char stop_bits;       // 1,2
}FPGA_UART_INIT_STRU;

#define FPGA_SERIAL_NAME_SUFFIX    "/dev/FPGA_Jikong"


FLXInt32 devc_fpga_serial_write_data(FLXInt32 handle, FLXByte *data, FLXInt32 len)
{
	if (write(handle, data, len) < 0)
		return -1;
	else
		return 0;
}  

FLXInt32 devc_fpga_serial_read_data(FLXInt32 handle, FLXByte *data, FLXInt32 len)
{
	return read(handle, data, len);
	/*
	FLXInt32 iReadLen = 0;
	FLXChar cBuff[10] = {0};
	FLXInt32 i;

	for (i = 0; i < len; i ++)
	{
		if (iReadLen = read(handle, cBuff, 1) > 0)
			data[i] = cBuff[0];
	}
	
	return 0;
	*/
}

FLXInt32 devc_fpga_serial_modify(FLXInt32 fd, int baud_rate, unsigned char data_bits,
		                              unsigned char parity, unsigned char stop_bits)
{
	FPGA_UART_INIT_STRU uartParam;

	uartParam.band_rate = baud_rate;
	uartParam.data_bits = data_bits;
	uartParam.parity    = parity;
	uartParam.stop_bits = stop_bits;

	if (ioctl(fd, 1, &uartParam) < 0)
	{
		perror("Call cmd IOCPRINT fail\n");
	 	return -1;
	}
	return 0;
}

FLXInt32 devc_fpga_serial_close(FLXInt32 handle)
{
	if (close(handle) == 0)
		return 0;
	else
		return -1;
}

/*****************************************************************************
 �� �� ��  : devc_init_fpga_serial_port
 ��������  : ��ʼ��fpga����
 �������  : FLXInt32 port     ���ں�
             FLXInt32 rate     ������
             FLXByte dataBits  ����λ
             FLXByte parity    У��λ
             FLXByte stopBits  ֹͣλ
             FLXInt32 *handle  ���ھ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��3��30��������
    ��    ��   : chen zhi tao
    �޸�����   : �����ɺ���

*****************************************************************************/

FLXInt32 devc_fpga_serial_open(int serial_no, FLXInt32 *handle)
{
	//FLXChar file[50] = "/dev/FPGA_Jikong";
	//FLXChar fileNo[10] = {0};
	char serial_name[256];

	sprintf(serial_name, "%s%02d", FPGA_SERIAL_NAME_SUFFIX, serial_no - 1);

	//sprintf(fileNo, "%02d", serial_no - 1);
	//strcat(file, fileNo);
	FLXInt32 fd = open(serial_name, O_RDWR);
	if (fd < 0)
	{
		perror("Cannot Open Serial Port !\n");
		return -1;
	}

	*handle = fd;
	return 0;
}
