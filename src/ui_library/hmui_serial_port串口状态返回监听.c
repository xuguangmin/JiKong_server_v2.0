#include <stdio.h>
#include <stdlib.h>

#if 0

#define MAX_SERIAL_NUM	256
#define MAX_SOCKET_NUM	256

typedef struct SOCKET_INFO
{
	FLXInt16 reg;/* 标记SOCKET是否注册,0表示注册 */
    FLXSocket sock;
}SOCKET_INFO_STRU;

/* 串口客户端关联结构体 */
typedef struct PORT_INFO
{
	FLXInt16 reg;/* 标记串口是否进行过初始化,0表示注册 */
	SOCKET_INFO_STRU sockInfo[MAX_SOCKET_NUM];	/* 关联的socket */
}PORT_INFO_STRU;

/* 串口id与句柄关联 */
struct tag_SerialInfo
{
	FLXInt32 iSerialId;
	FLXInt32 iHandle;
};

static struct tag_SerialInfo sSerialInfo[MAX_SERIAL_NUM];
static PORT_INFO_STRU port[MAX_SERIAL_NUM] = {-1};
static FLXInt32 iSerialCount = 0;

void hmui_serial_port_listen_thread(void *param)
{
	FLXInt32 port = param;	
}

FLXInt32 hmui_init_serial_port(FLXInt32 iSerialId, FLXInt32 iPort, FLXInt32 iRate)
{
	FLXInt32 iHandle;

	if (port[iPort].reg != 0)/* 未注册 */
	{
		if (dec_init_serial_port("/dev/s3c2410_serial1", iRate, &iHandle) != 0)
			return -1;

		/* 关联串口id和句柄 */
		sSerialInfo[iSerialCount].iSerialId = iSerialId;
		sSerialInfo[iSerialCount].iHandle = iHandle;
		iSerialCount ++;

		/* 注册串口 */
		port[iPort].reg = 0;
		port[iPort].sockInfo[0].sock = *((FLXSocket *)param);
		port[iPort].sockInfo[0].reg = 0;

		/* 开始串口监听 */
		FLXThread pid;
		if (thread_create(&pid, NULL, (void *)hmui_serial_port_listen_thread, *iPort) != 0)
		{
			MSG_OUT("serial port listen thread create error");
			return -1;
		}
	}
	else
	{
		/* 向串口注册新的socket */
		for (FLXInt32 i = 0; i < MAX_SOCKET_NUM; i ++)
		{
			if (port[iPort].sockInfo[i].reg != 0)
			{
				port[iPort].sockInfo[i].sock = *((FLXSocket *)param);
				port[iPort].sockInfo[i].reg = 0;
				break;
			}
		}
	}

	return 0;
}

FLXInt32 hmui_serial_port_write_data(void *param, FLXInt32 iSerialId, FLXChar *pcData, FLXInt32 iLen)
{
	FLXInt32 i;

	/* 根据控件id确定控件句柄 */
	for (i = 0; i < iSerialCount; i ++)
	{
		if (sSerialInfo[i].iSerialId == iSerialId)
		{
			break;
		}
	}

	if (i == iSerialCount)
	{
		return -1;
	}
	else
	{
		if (dec_serial_port_write_data(sSerialInfo[i].iHandle, pcData, iLen) == -1)
		{
			return -1;
		}

		return 0;
	}
}

FLXInt32 hmui_release_serial_port(void *param, FLXInt32 iSerialId)
{
	FLXInt32 i;

	/* 根据控件id确定控件句柄 */
	for (i = 0; i < iSerialCount; i ++)
	{
		if (sSerialInfo[i].iSerialId == iSerialId)
		{
			break;
		}
	}

	if (i == iSerialCount)
	{
		return -1;
	}
	else
	{
		if (dec_serial_port_close(sSerialInfo[i].iHandle) != 0)
		{
			return -1;
		}
		return 0;
	}
}
#endif
