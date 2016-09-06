#include <stdio.h>
#include <stdlib.h>

#include "share_type.h"
//#include "flx_methord.h"
#include "FLXCommon/flxthread.h"
#include "FLXCommon/tcpserver.h"
#include "util_log.h"

#include "tcp_share_type.h"
#include "tps_tcp_slave_client.h"
#include "connect_manager.h"
//#include "data_pool/ccc_data_pool.h"
#include "tps_tcp_server.h"
#include "connect_http.h"
#include "wol.h"

static network_recv_data g_network_recv_data = NULL;

#define TPS_TCP_SERVER_INFO(args) (printf args)
#ifdef DEBUG
	#define TPS_TCP_SERVER_DEBUG(args)	(printf("[tps] "), printf args)
#else
	#define TPS_TCP_SERVER_DEBUG(args)
#endif

static FLXSocket myServerSocket         = -1;/* server socket */
static FLXSocket ToSlowServerSocket     = -1;/* server socket for slowServer */


FLXInt32 tps_tcp_server_stop()
{
	tps_release_all_client();
	if (tcp_server_stop(myServerSocket) == 0)
	{
		myServerSocket = -1;
		return 0;
	}
	else
	{
		return -1;
	}
}


//FLXInt32 tps_server_receive_data(FLXSocket sock, char *pcData, FLXInt32 iLen)
//{
//	return tcp_server_readData(sock, pcData, iLen);
//}

int network_manager_send_onvif(const char *http_uri, const char *http_body)
{
	return connect_http_send_onvif(http_uri, http_body);
}
/*
 * ������
 *     des_mac_addr : Ŀ��PC��MAC��ַ
 *
 * format
 *     00:11:22:33:44:55
 */
int network_manager_wol(const char *des_mac_addr)
{
	return wake_on_lan(des_mac_addr);
}
int network_manager_send_url(const char *url)
{
	return connect_http_send_url(url);
}
int network_manager_send_data(FLXSocket sock, unsigned char *data, unsigned int data_len)
{
	return (0 == tcp_server_sendData(sock, data, data_len)) ? 1:0;
}

/* ���ϲ㴫������ */
static void output_network_recv_data(int data_source, unsigned char *buffer, int *data_len, CLIENT_INFO_STRU *clientInfo)
{
	if(g_network_recv_data)
		g_network_recv_data(data_source, buffer, data_len, clientInfo);
}
/*
 * �Ӵӻ��Ŀͻ��˻�ȡ���ݵĻص�����
 *
 *������
 *   *data      ���ݻ���
 *   *data_len  ���ݵĳ��ȣ��ֽ�Ϊ��λ�����ϲ㴦�������ʣ�����ݵĳ���
 *
 */
int callback_recv_data_from_slave_client(unsigned char *data, int *data_len)
{
	output_network_recv_data(DATA_SOURCE_SLAVE_CLIENT, data, data_len, NULL);
	return 1;
}
/*
 * ��ȡ�ͻ������ݵĻص�����
 *      ��ƽ����������
 *      ������������Ĵӻ�
 *
 *������
 *   data_source ����������Դ
 *   *data       ���ݻ���
 *   *data_len   ���ݵĳ��ȣ��ֽ�Ϊ��λ�����ϲ㴦�������ʣ�����ݵĳ���
 *   clientInfo  �ͻ�����Ϣ
 */
int callback_recv_data_from_client(int data_source, unsigned char *data, int *data_len, CLIENT_INFO_STRU *clientInfo)
{
	output_network_recv_data(data_source, data, data_len, clientInfo);
	return 1;
}

int slave_relay_data_to_master(FLXByte *strName, FLXInt32 iLen)
{
	return tps_tcp_slave_client_send(strName, iLen);
}

/* ���� ������pad��������� */
FLXInt32 callback_server_accept_client_pad_or_0(FLXSocket *sock, FLXIpEndPoint* ipEndpoint)
{
	CCC_LOG_OUT("enter %s.\n", __FUNCTION__);
	if(!tps_reg_client_pad_or_0(*sock))
	{
		/* ���ע�᲻�ɹ���Ӧ��close���sock ... */
		return -1;
	}

	CCC_LOG_OUT("pad or designer connected(%d) : %s, %d\n", *sock, ipEndpoint->ipAddress, ipEndpoint->port);
	return 0;
}

/* ���� �����Դӻ� */
FLXInt32 callback_server_accept_client_slave(FLXSocket *sock, FLXIpEndPoint* ipEndpoint)
{
	CCC_LOG_OUT("enter %s.\n", __FUNCTION__);
	if(!tps_reg_client_slave_server(*sock))
		return -1;

	CCC_LOG_OUT("slave connected(%d) : %s, %d\n", *sock, ipEndpoint->ipAddress, ipEndpoint->port);
	return 0;
}

//************************************
// ��������:  tps_tcp_server_start
// ����:		����server
// ����ֵ:   	FLXInt32:	0���ɹ���
//************************************
/*���տͻ��˷���ʼ*/
FLXInt32 tps_tcp_server_start(FLXInt32 port)
{
	myServerSocket = tcp_server_initialize();//����socket
	if (myServerSocket == -1)
	{
		printf("1tcp server initialize error\n");
		return -1;
	}

	if (tcp_server_start(myServerSocket, port, callback_server_accept_client_pad_or_0) != 0)
	{
		return -1;
	}
	return 0;
}
/*�������������մӻ���������ʼ*/
FLXInt32 server_to_slow_start(FLXInt32 port)
{
	ToSlowServerSocket = tcp_server_initialize();//�������մӻ����ӵ�socket
	if (ToSlowServerSocket == -1)
	{
		//MSG_OUT("tcp server initialize error\n");
		return -1;
	}

	if (tcp_server_start(ToSlowServerSocket, port, callback_server_accept_client_slave) != 0)
	{
		//MSG_OUT("tcp server start error\n");
		return -1;
	}
	return 0;
}

int network_manager_init(SERVER_CONFIG *server_config, network_recv_data callback)
{
	int iRet = 0;
	FLXThread pid;
	int server_type;
	if(!server_config)
		return 0;

	server_type = server_config->server_type;
	switch(server_type)
	{
	case SERVER_TYPE_SINGLE:
	case SERVER_TYPE_MASTER:
	case SERVER_TYPE_SLAVE:
		break;
	default:
		printf("server type error\n");
		return 0;
	}

	iRet = tps_tcp_server_start(server_config->port1);  /* ������ */
	if (iRet != 0)
	{
		CCC_LOG_OUT("error :main server not start.\n");
		return 0;
	}
	CCC_LOG_OUT("main server start.\n");

	if(SERVER_TYPE_MASTER == server_type)
	{
		/*
		 * �����Ĵӷ���
		 * ��������Ҫ������pad�ȿͻ��ˣ�
		 * �������ڴӻ�
		 */
		iRet = thread_create(&pid, NULL, (void *)server_to_slow_start,(void*)(server_config->port2));
		if (iRet != 0)
		{
			CCC_LOG_OUT("thread create error\n");
			return 0;
		}
		CCC_LOG_OUT("master second server start.\n");
	}
	else if(SERVER_TYPE_SLAVE == server_type)
	{
		/*
		 * �ӻ��Ŀͻ���
		 * �ÿͻ��˽���������
		 */
		if(!tps_tcp_slave_client_start(server_config->dst_server_ip, server_config->dst_server_port, callback_recv_data_from_slave_client))//proa_analyse_data_from_master))
		{
			CCC_LOG_OUT("tps_tcp_slave_client_start\n");
			return 0;
		}
	}

	g_network_recv_data = callback;
	tps_client_manage_init(callback_recv_data_from_client);

	if(!connect_http_init())
		return 0;

	return 1;
}

int network_manager_close()
{
	connect_http_close();
	tps_tcp_server_stop();
	return 1;
}



