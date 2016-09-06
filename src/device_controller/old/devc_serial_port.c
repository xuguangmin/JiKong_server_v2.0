#include    <stdio.h>      /*��׼�����������*/
#include    <stdlib.h>     /*��׼�����ⶨ��*/
#include    <unistd.h>     /*Unix ��׼��������*/
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>      /*�ļ����ƶ���*/
#include    <termios.h>    /*PPSIX �ն˿��ƶ���*/
#include    <errno.h>      /*����Ŷ���*/

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
// ��������:  	devc_set_speed
// ����:			���ô���ͨ������
// ����:		FLXInt32 fd:	�򿪴��ڵ��ļ����
// ����:		FLXInt32 speed:	�����ٶ�
// ����ֵ:   	void:
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
// ��������:  	devc_set_parity
// ����:		���ô�������λ��ֹͣλ��Ч��λ
// ����:		FLXInt32 fd:		�򿪵Ĵ����ļ����
// ����:		FLXInt32 databits:	����λ   ȡֵ Ϊ 7 ����8
// ����:		FLXInt32 stopbits:	ȡֵΪ 1 ����2
// ����:		FLXInt32 parity:	Ч������ ȡֵΪN,E,O,,S
// ����ֵ:   	FLXInt32:
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

	switch (databits) /*��������λ��*/
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
			options.c_cflag |= (PARODD | PARENB); /* ����Ϊ��Ч��*/
			options.c_iflag |= INPCK;             /* Disnable parity checking */

			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     /* Enable parity */
			options.c_cflag &= ~PARODD;   /* ת��ΪżЧ��*/
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

    /* ����ֹͣλ*/
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
	options.c_cc[VTIME] = 0;		/* ���ó�ʱ0 seconds*/
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
// ��������:  	devc_init_serial_port
// ����:							��ʼ������
// ����:		FLXChar *cDev:		�����豸�ļ�
// ����:		FLXInt32 iRate:		������
// ����:		FLXInt32 * iHandle:	���ھ��
// ����ֵ:   	FLXInt32:
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
// ��������:  dec_serial_port_write_data
// ����:		�򴮿�д������
// ����:		FLXInt32  iHandle:	���ھ��
// ����:		FLXChar * pcData:	Ҫд�������
// ����:		FLXInt32 iLen:		д������ݳ���
// ����ֵ:   	FLXInt32:			-1��д��ʧ�ܣ����򷵻ص���ʵ��д������ݳ���
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
// ��������:  	dec_serial_port_read_data
// ����:							�Ӵ��ڶ�����
// ����:		FLXInt32 iHandle:	���ھ��
// ����:		FLXChar * pcData:	����buffer
// ����:		FLXInt32 iLen:		Ҫ�������ݳ���
// ����ֵ:   	FLXInt32:
//************************************
FLXInt32 devc_serial_port_read_data(FLXInt32 iHandle, FLXChar *pcData, FLXInt32 iLen)
{
	FLXInt32 iReadLen = 0;
	FLXChar cBuff[10] = {0};
	FLXInt32 i;

	for (i = 0; i < iLen; i ++)
	{
		if ((iReadLen = read(iHandle, cBuff, 1)) > 0)  /* 20121010-16:08  jia �������� */
		{
			pcData[i] = cBuff[0];
		}
	}

	return 0;
}

//************************************
// ��������:  	dec_serial_port_close
// ����:							�رմ���
// ����:		FLXInt32 iHandle:	���ھ��
// ����ֵ:   	FLXInt32:
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
//	FLXChar *dev  = "/dev/ttyS0"; //���ڶ�
//
//	fd = devc_open_dev(dev);
//	devc_set_speed(fd, 19200);
//	if (devc_set_parity(fd, 8, 1, 'N') == FALSE)
//	{
//		printf("Set Parity Error\n");
//		exit (0);
//	}
//
//	while (1) //ѭ����ȡ����
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
