/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ���    ��tps_tcp_slave_client.c
  ������    �����Ӹ�
  ��������   ��2012-11-6
  ��������   : tcp�ͻ��ˣ����ڷ�������Ϊ�ӻ�ʱ������һ��������������
  �����б�   :
  �޸���ʷ   :

******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "FLXCommon/flxtypes.h"
#include "FLXCommon/flxnettypes.h"
#include "FLXCommon/flxthread.h"
#include "FLXCommon/tcpserver.h"
#include "tps_tcp_slave_client.h"
#include "util_log.h"

static FLXSocket g_slave_client_socket = INVALID_SOCKET;           /* TODO ��Ҫ��ͬ�� */

static tps_tcp_slave_client_callback g_slave_client_callback = NULL;
static char *g_master_server_ip   = NULL;
static int   g_master_server_port = 0;
static int   g_connetionOrNotFlag = 0;


static FLXSocket tps_tcp_slave_client_connect(const char *server_ip, int server_port)
{
	FLXSocket clSocket = INVALID_SOCKET;
	struct sockaddr_in remote_ipv4_address;
	if(!server_ip)
		return INVALID_SOCKET;

	if ((clSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		CCC_LOG_OUT("tps_tcp_slave_client_connect create socket failed: %s\n", strerror(errno));
		return INVALID_SOCKET;
	}

	CCC_LOG_OUT("slave create socket success sock= %d\n", (int)clSocket);
	memset(&remote_ipv4_address, 0, sizeof(remote_ipv4_address));
	remote_ipv4_address.sin_family = AF_INET;
	remote_ipv4_address.sin_port = htons(server_port);
	inet_pton(AF_INET, server_ip, (void *)&(remote_ipv4_address.sin_addr));

	while(1)
	{
		g_connetionOrNotFlag = 0;
		CCC_LOG_OUT("\nslave connecting to : %s (%d) ... \n", server_ip, server_port);
		if(connect(clSocket, (struct sockaddr *)&remote_ipv4_address, sizeof(remote_ipv4_address)) == 0)
		{
			g_connetionOrNotFlag = 1;
			CCC_LOG_OUT("slave connect OK\n");
			break;
		}
		CCC_LOG_OUT("slave connect failed, error : %s\n", strerror(errno));
		CCC_LOG_OUT("wait to continue ...\n");
		sleep(2);
	}
	return clSocket;
}

/* �ӻ�����������������������Ϣ */
static int tps_tcp_slave_client_recv(FLXSocket sock, FLXByte *recBuffer, int buf_size)
{
	int iRetrun;

	CCC_DEBUG_OUT("tps_tcp_slave_client_recv data ...\n");
	memset(recBuffer, 0, buf_size);
	iRetrun = recv(sock, recBuffer, buf_size, 0);
	CCC_DEBUG_OUT("iRetrun = %d\n",iRetrun);
	return iRetrun;
}

static void close_slave_client_socket()
{
	CCC_LOG_OUT("sock = %d error, will to close.\n", (int)g_slave_client_socket);
	close(g_slave_client_socket);
	g_slave_client_socket = INVALID_SOCKET;
}

void *tps_slave_client_thread_func(void *param)
{
	FLXByte recBuffer[RECV_DATA_BUFFER];
	FLXByte mainRecBuffer[RECV_DATA_BUFFER * 2];  /* ��������С������Ҫ���� */
	FLXInt32 iRetrun, recvLen = 0;

	while(1)
	{
		if(INVALID_SOCKET == g_slave_client_socket)
		{
			g_slave_client_socket = tps_tcp_slave_client_connect(g_master_server_ip, g_master_server_port);
		}
		else
		{
			iRetrun = tps_tcp_slave_client_recv(g_slave_client_socket, recBuffer, RECV_DATA_BUFFER);
			if (iRetrun > 0)
			{
				memcpy(&mainRecBuffer[recvLen],recBuffer,iRetrun);
				recvLen += iRetrun;

				while(recvLen >= 9)  /* 1 + 2 + 4 + 2*/
				{
					if(g_slave_client_callback)
						g_slave_client_callback(mainRecBuffer, &recvLen);
				}
			}
			else //if (iRetrun <= 0 )
			{
				close_slave_client_socket();
			}
		}
	}
	return NULL;
}

/*
 * �ӻ��������ӵ�������������
 */
int tps_tcp_slave_client_send(unsigned char *data, int data_len)
{
	if(g_slave_client_socket != INVALID_SOCKET && g_connetionOrNotFlag ==1)
	{
		if(0 == tcp_server_sendData(g_slave_client_socket, data, data_len))
			return 1;
	}

	CCC_LOG_OUT("tps_tcp_slave_client_send error.\n");
	return 0;
}

/*
 * �����ӻ��ͻ�����������������
 */
int tps_tcp_slave_client_start(const char *server_ip, int server_port, tps_tcp_slave_client_callback callback)
{
	FLXThread pid;
	int len = strlen(server_ip);
	if(!server_ip || len <= 0 || !callback)
		return 0;

	// IP
	if(g_master_server_ip)
	{
		free(g_master_server_ip);
		g_master_server_ip = NULL;
	}
	g_master_server_ip = (char *)malloc(len + 1);
	strcpy(g_master_server_ip, server_ip);
	g_master_server_ip[len] = '\0';
	// port
	g_master_server_port = server_port;
	// call back function
	g_slave_client_callback = callback;

	if(thread_create(&pid, NULL, (void *)tps_slave_client_thread_func, NULL) != 0)
	{
		CCC_LOG_OUT("%s thread_create error\n", __FUNCTION__);
		return 0;
	}
	return 1;
}


