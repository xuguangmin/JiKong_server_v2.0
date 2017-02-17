#include    <stdio.h>      /*标准输入输出定义*/
#include    <stdlib.h>     /*标准函数库定义*/
#include    <unistd.h>     /*Unix 标准函数定义*/
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>      /*文件控制定义*/
#include    <termios.h>    /*PPSIX 终端控制定义*/
#include    <errno.h>      /*错误号定义*/

#include "FLXCommon/flxtypes.h"

#ifdef DEBUG
#define SERIAL_PORT_DEBUG(args)	(printf("[dec] "), printf args)
#else
#define SERIAL_PORT_DEBUG(args)
#endif

FLXInt32 speed_arr[] = {B38400, B19200, B9600, B4800, B2400, B1200, B300,
                   B38400, B19200, B9600, B4800, B2400, B1200, B300,};
FLXInt32 name_arr[] = {38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,
                    19200,  9600, 4800, 2400, 1200,  300,};

//************************************
// 函数名称:  	devc_set_speed
// 功能:			设置串口通信速率
// 参数:		FLXInt32 fd:	打开串口的文件句柄
// 参数:		FLXInt32 speed:	串口速度
// 返回值:   	void:
//************************************
void devc_set_speed(FLXInt32 fd, FLXInt32 speed)
{
     FLXInt32   i;
     FLXInt32   status;
     struct termios   Opt;
     tcgetattr(fd, &Opt);

     for (i = 0; i < sizeof(speed_arr) / sizeof(FLXInt32); i ++)
     {
           if (speed == name_arr[i])
           {
                tcflush(fd, TCIOFLUSH);
                cfsetispeed(&Opt, speed_arr[i]);
                cfsetospeed(&Opt, speed_arr[i]);
                status = tcsetattr(fd, TCSANOW, &Opt);
                if  (status != 0)
                {
					SERIAL_PORT_DEBUG(("tcsetattr fd\n"));
					return;
                }
                tcflush(fd, TCIOFLUSH);
           }
     }
}

//************************************
// 函数名称:  	devc_set_parity
// 功能:		设置串口数据位，停止位和效验位
// 参数:		FLXInt32 fd:		打开的串口文件句柄
// 参数:		FLXInt32 databits:	数据位   取值 为 7 或者8
// 参数:		FLXInt32 stopbits:	取值为 1 或者2
// 参数:		FLXInt32 parity:	效验类型 取值为N,E,O,,S
// 返回值:   	FLXInt32:
//************************************
FLXInt32 devc_set_parity(FLXInt32 fd, FLXInt32 databits, FLXInt32 stopbits, FLXInt32 parity)
{
	struct termios options;

	if (tcgetattr(fd,&options) != 0)
	{
		SERIAL_PORT_DEBUG(("SetupSerial 1\n"));
		return -1;
	}

	options.c_cflag &= ~CSIZE;
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	options.c_oflag  &= ~OPOST;   /*Output*/

	switch (databits) /*设置数据位数*/
	{
		case 7:
			options.c_cflag |= CS7;

			break;
		case 8:
			options.c_cflag |= CS8;

			break;
		default:
			SERIAL_PORT_DEBUG(("Unsupported data size\n")); 
			return -1;
	}

    switch (parity)
    {
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */

			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
			options.c_iflag |= INPCK;             /* Disnable parity checking */

			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     /* Enable parity */
			options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
			options.c_iflag |= INPCK;       /* Disnable parity checking */

			break;
		case 'S':
		case 's':  /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;

			break;
		default:
			SERIAL_PORT_DEBUG(("Unsupported parity\n"));
			return -1;
     }

    /* 设置停止位*/
    switch (stopbits)
    {
		case 1:
			options.c_cflag &= ~CSTOPB;

			break;
		case 2:
			options.c_cflag |= CSTOPB;

			break;
		default:
			SERIAL_PORT_DEBUG(("Unsupported stop bits\n"));
			return -1;
    }

    /* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;

	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 0;		/* 设置超时0 seconds*/
	options.c_cc[VMIN] = 1;			/* define the minimum bytes data to be readed*/
	if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		SERIAL_PORT_DEBUG(("SetupSerial 3"));
		return -1;
	}

	return 0;
}

FLXInt32 devc_open_dev(FLXChar *cDev)
{
	FLXInt32 fd = open(cDev, O_RDWR);    //| O_NOCTTY | O_NDELAY

	if (-1 == fd)
	{
		SERIAL_PORT_DEBUG(("Can't Open Serial Port %s", cDev));
		return -1;
	}
	else
		return fd;
}

//************************************
// 函数名称:  	devc_init_serial_port
// 功能:							初始化串口
// 参数:		FLXChar *cDev:		串口设备文件
// 参数:		FLXInt32 iRate:		波特率
// 参数:		FLXInt32 * iHandle:	串口句柄
// 返回值:   	FLXInt32:
//************************************
FLXInt32 devc_init_serial_port(FLXChar *cDev, FLXInt32 iRate, FLXInt32 *iHandle)
{
	FLXInt32 fd;
	// jia FLXChar buff[512] = {0};

	fd = devc_open_dev(cDev);
	devc_set_speed(fd, iRate);
	if (devc_set_parity(fd, 8, 1, 'N') == -1)
	{
		return -1;
	}

	*iHandle = fd;
	return 0;
}

//************************************
// 函数名称:  dec_serial_port_write_data
// 功能:		向串口写入数据
// 参数:		FLXInt32  iHandle:	串口句柄
// 参数:		FLXChar * pcData:	要写入的数据
// 参数:		FLXInt32 iLen:		写入的数据长度
// 返回值:   	FLXInt32:			-1，写入失败；否则返回的是实际写入的数据长度
//************************************
FLXInt32 devc_serial_port_write_data(FLXInt32 iHandle, FLXChar *pcData, FLXInt32 iLen)
{
	FLXInt32 iWriteLen = 0;

	iWriteLen = write(iHandle, pcData, iLen);
	if (iWriteLen  == -1)
	{
		return -1;
	}
	else
	{
		return iWriteLen;
	}
}

//************************************
// 函数名称:  	dec_serial_port_read_data
// 功能:							从串口读数据
// 参数:		FLXInt32 iHandle:	串口句柄
// 参数:		FLXChar * pcData:	数据buffer
// 参数:		FLXInt32 iLen:		要读的数据长度
// 返回值:   	FLXInt32:
//************************************
FLXInt32 devc_serial_port_read_data(FLXInt32 iHandle, FLXChar *pcData, FLXInt32 iLen)
{
	FLXInt32 iReadLen = 0;
	FLXChar cBuff[10] = {0};
	FLXInt32 i;

	for (i = 0; i < iLen; i ++)
	{
		if ((iReadLen = read(iHandle, cBuff, 1)) > 0)  /* 20121010-16:08  jia 增加括号 */
		{
			pcData[i] = cBuff[0];
		}
	}

	return 0;
}

//************************************
// 函数名称:  	dec_serial_port_close
// 功能:							关闭串口
// 参数:		FLXInt32 iHandle:	串口句柄
// 返回值:   	FLXInt32:
//************************************
FLXInt32 devc_serial_port_close(FLXInt32 iHandle)
{
	close(iHandle);
	return 0;
}

//FLXInt32 main(FLXInt32 argc, FLXChar **argv)
//{
//	FLXInt32 fd;
//	FLXInt32 nread;
//	FLXChar buff[512] = {0};
//	FLXChar *dev  = "/dev/ttyS0"; //串口二
//
//	fd = devc_open_dev(dev);
//	devc_set_speed(fd, 19200);
//	if (devc_set_parity(fd, 8, 1, 'N') == FALSE)
//	{
//		printf("Set Parity Error\n");
//		exit (0);
//	}
//
//	while (1) //循环读取数据
//	{
//		while ((nread = read(fd, buff, 1)) > 0)
//		{
//			printf("Len %d\n", nread);
//			buff[nread + 1] = '\0';
//			printf("%s\n", buff);
//
//			buff[0] = 'v';
//			write(fd, buff, 1);
//		}
//	}
//
//	close(fd);
//}
