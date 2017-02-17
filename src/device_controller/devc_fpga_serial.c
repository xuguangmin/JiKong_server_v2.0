/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : devc_fpga_serial_port.c
  作者    : chen zhi tao
  生成日期 : 2011年3月30日星期三

  版本    : 1.0
  功能描述 : fpga串口接口


  修改历史 :

******************************************************************************/

#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>     /*标准函数库定义*/
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX 终端控制定义*/
#include <errno.h>      /*错误号定义*/
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
 函 数 名  : devc_init_fpga_serial_port
 功能描述  : 初始化fpga串口
 输入参数  : FLXInt32 port     串口号
             FLXInt32 rate     波特率
             FLXByte dataBits  数据位
             FLXByte parity    校验位
             FLXByte stopBits  停止位
             FLXInt32 *handle  串口句柄
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年3月30日星期三
    作    者   : chen zhi tao
    修改内容   : 新生成函数

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
