#include <stdio.h>
#include <stdlib.h>

#if 0

#define MAX_SERIAL_NUM	256
#define MAX_SOCKET_NUM	256

typedef struct SOCKET_INFO
{
	FLXInt16 reg;/* ���SOCKET�Ƿ�ע��,0��ʾע�� */
    FLXSocket sock;
}SOCKET_INFO_STRU;

/* ���ڿͻ��˹����ṹ�� */
typedef struct PORT_INFO
{
	FLXInt16 reg;/* ��Ǵ����Ƿ���й���ʼ��,0��ʾע�� */
	SOCKET_INFO_STRU sockInfo[MAX_SOCKET_NUM];	/* ������socket */
}PORT_INFO_STRU;

/* ����id�������� */
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

	if (port[iPort].reg != 0)/* δע�� */
	{
		if (dec_init_serial_port("/dev/s3c2410_serial1", iRate, &iHandle) != 0)
			return -1;

		/* ��������id�;�� */
		sSerialInfo[iSerialCount].iSerialId = iSerialId;
		sSerialInfo[iSerialCount].iHandle = iHandle;
		iSerialCount ++;

		/* ע�ᴮ�� */
		port[iPort].reg = 0;
		port[iPort].sockInfo[0].sock = *((FLXSocket *)param);
		port[iPort].sockInfo[0].reg = 0;

		/* ��ʼ���ڼ��� */
		FLXThread pid;
		if (thread_create(&pid, NULL, (void *)hmui_serial_port_listen_thread, *iPort) != 0)
		{
			MSG_OUT("serial port listen thread create error");
			return -1;
		}
	}
	else
	{
		/* �򴮿�ע���µ�socket */
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

	/* ���ݿؼ�idȷ���ؼ���� */
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

	/* ���ݿؼ�idȷ���ؼ���� */
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
