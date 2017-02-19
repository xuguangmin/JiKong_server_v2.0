#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "FLXCommon/flxthread.h"
#include "FLXCommon/tcpserver.h"
#include "util_log.h"
#include "tcp_share_type.h"
#include "tps_client_manage.h"

#define FLX_CLIENT_TYPE_UNKNOWN        -1
#define FLX_CLIENT_TYPE_DESIGNER        0
#define FLX_CLIENT_TYPE_PAD             1
#define FLX_CLIENT_TYPE_PAD_IOS         2

typedef struct flx_client_node *FLX_CLIENT_NODE;
struct flx_client_node
{
	int              clientType;            /* 客户端类型：pad还是设计器 */
	FLXSocket        sock;					/* 客户端套接字 */

	FLX_CLIENT_NODE  prev;
	FLX_CLIENT_NODE  next;
};

static FLX_CLIENT_NODE     g_flx_client_node_head = NULL;
static pthread_mutex_t     g_flx_client_mutex     = PTHREAD_MUTEX_INITIALIZER;
static tps_client_callback g_tps_client_callback  = NULL;


static int output_client_recv_data(int data_source, unsigned char *buffer, int *data_len, CLIENT_INFO_STRU *clientInfo)
{
	if(g_tps_client_callback)
	{
		int old_data_len = *data_len;
		//最终调用callback_recv_data_from_network
		g_tps_client_callback(data_source, buffer, data_len, clientInfo);

		/* 如果数据长度没变，说明剩下的数据中已经没有完整的数据包了 */
		return (*data_len == old_data_len) ? 0:1;
	}
	return 0;
}

static FLX_CLIENT_NODE make_flx_client_node()
{
	FLX_CLIENT_NODE lp_node = (FLX_CLIENT_NODE)malloc(sizeof(struct flx_client_node));
	if(!lp_node)
		return NULL;

	lp_node->sock           = INVALID_SOCKET;
	lp_node->clientType     = FLX_CLIENT_TYPE_UNKNOWN;

	lp_node->prev           = NULL;
	lp_node->next           = NULL;
	return lp_node;
}

static void free_flx_client_node(FLX_CLIENT_NODE lp_node)
{
	tcp_server_stop(lp_node->sock);
	free(lp_node);
}

static void insert_flx_client_node(FLX_CLIENT_NODE *head, FLX_CLIENT_NODE lp_node)
{
	if(*head == NULL)
	{
		lp_node->prev = NULL;
		lp_node->next = NULL;
		*head = lp_node;
	}
	else
	{
		lp_node->next = *head;
		(*head)->prev = lp_node;

		*head = lp_node;
		lp_node->prev = NULL;
	}
}
/*
 * 删除一个节点
 * 返回值：
 *      1 该节点存在
 *      0 该节点不存在
 */
static int interval_delete_flx_client_node(FLX_CLIENT_NODE *head, FLXSocket sock)
{
	FLX_CLIENT_NODE node;
	if(!(*head))
		return 0;

	node = *head;
	while(node)
	{
		if(node->sock == sock)
		{
			FLX_CLIENT_NODE temp_node;
			if(node->prev) node->prev->next = node->next;
			if(node->next) node->next->prev = node->prev;

			temp_node = node;
			node = node->next;

			/* 如果是删除的头节点，则头节点指针需要移动*/
			if(*head == temp_node) *head = node;
			free_flx_client_node(temp_node);
			return 1;
		}

		node = node->next;
	}
	return 0;
}

static void interval_delete_all_flx_client_node(FLX_CLIENT_NODE *head)
{
	FLX_CLIENT_NODE temp_node, node;
	if(!(*head))
		return;

	node = *head;
	while(node)
	{
		temp_node = node;
		node = node->next;

		free_flx_client_node(temp_node);
	}
	*head = NULL;
	return;
}

static FLX_CLIENT_NODE interval_add_flx_client_node(FLX_CLIENT_NODE *head, FLXSocket sock)
{
	FLX_CLIENT_NODE lp_node = make_flx_client_node();
	if(NULL == lp_node)
		return NULL;

	lp_node->sock = sock;
	insert_flx_client_node(head, lp_node);
	return lp_node;
}

/* 根据sock搜索节点 */
static FLX_CLIENT_NODE interval_search_flx_client_node(FLX_CLIENT_NODE *head, FLXSocket sock)
{
	FLX_CLIENT_NODE node;
	if(!(*head))
		return NULL;

	node = *head;
	while(node)
	{
		if(node->sock == sock)
		{
			return node;
		}

		node = node->next;
	}
	return NULL;
}

static int interval_modify_flx_client_node_value_type(FLX_CLIENT_NODE *head, FLXSocket sock, unsigned char client_type)
{
	FLX_CLIENT_NODE lp_node = interval_search_flx_client_node(head, sock);
	if(NULL == lp_node)
		return 0;

	switch(client_type)
	{
	case 0x01:   /* 设计器 */
		lp_node->clientType = FLX_CLIENT_TYPE_DESIGNER; //0x01;
		break;
	case 0x02:   /* pad */
		lp_node->clientType = FLX_CLIENT_TYPE_PAD;     //0x02;
		break;
	case 0x03:   /* ios */
		lp_node->clientType = FLX_CLIENT_TYPE_PAD_IOS;
		break;

	default:
		return 0;
	}
	return 1;
}

static void interval_send_data_to_all_pad(FLX_CLIENT_NODE *head, unsigned char *data, unsigned int data_len)
{
	FLX_CLIENT_NODE node;
	if(!(*head) || !data || data_len <= 0)
		return;

	node = *head;
	while(node)
	{
		if(FLX_CLIENT_TYPE_PAD     == node->clientType ||
		   FLX_CLIENT_TYPE_PAD_IOS == node->clientType)
		{
			tcp_server_sendData(node->sock, data, data_len);
		}
		node = node->next;
	}
}

static int interval_send_data_to_pad(FLX_CLIENT_NODE *head, FLXSocket sock, unsigned char *data, unsigned int data_len)
{
	FLX_CLIENT_NODE node;
	if(!(*head) || !data || data_len <= 0)
		return 0;

	node = *head;
	while(node)
	{
		if(sock == node->sock &&
		   (FLX_CLIENT_TYPE_PAD     == node->clientType ||
		    FLX_CLIENT_TYPE_PAD_IOS == node->clientType))
		{
			return (0 == tcp_server_sendData(sock, data, data_len)) ? 1:0;
		}
		node = node->next;
	}
	return 0;
}

int delete_flx_client_node(FLXSocket sock)
{
	int result;
	pthread_mutex_lock(&g_flx_client_mutex);
	result = interval_delete_flx_client_node(&g_flx_client_node_head, sock);
	pthread_mutex_unlock(&g_flx_client_mutex);
	return result;
}

FLX_CLIENT_NODE add_flx_client_node(FLXSocket sock)
{
	FLX_CLIENT_NODE result;
	pthread_mutex_lock(&g_flx_client_mutex);
	result = interval_add_flx_client_node(&g_flx_client_node_head, sock);
	pthread_mutex_unlock(&g_flx_client_mutex);
	return result;
}


int tps_set_client_type(FLXSocket sock, unsigned char client_type)
{
	int result;
	pthread_mutex_lock(&g_flx_client_mutex);
	result = interval_modify_flx_client_node_value_type(&g_flx_client_node_head, sock, client_type);
	pthread_mutex_unlock(&g_flx_client_mutex);
	return result;
}
void send_data_to_all_pad(unsigned char *data, unsigned int data_len)
{
	if(!data || data_len <= 0)
		return;

	pthread_mutex_lock(&g_flx_client_mutex);
	interval_send_data_to_all_pad(&g_flx_client_node_head, data, data_len);
	pthread_mutex_unlock(&g_flx_client_mutex);
}
int send_data_to_pad(FLXSocket sock, unsigned char *data, unsigned int data_len)
{
	int result = 0;
	if(!data || data_len <= 0)
		return 0;

	pthread_mutex_lock(&g_flx_client_mutex);
	result = interval_send_data_to_pad(&g_flx_client_node_head, sock, data, data_len);
	pthread_mutex_unlock(&g_flx_client_mutex);
	return result;
}

/*
 * 功能:		监听接收客户端数据
 * 参数:		void * param:	传入参数
 * 返回值:   	void:
 */
static void thread_func_client_pad_or_0(void *param)
{
	CLIENT_INFO_STRU clientInfo;
	FLXByte recBuffer[RECV_DATA_BUFFER];
	FLXByte mainRecBuffer[RECV_DATA_BUFFER * 2];
	FLXInt32 iRetrun, recvLen = 0;
	FLX_CLIENT_NODE flx_client_node = (FLX_CLIENT_NODE)param;
	if(!flx_client_node)
	{
		CCC_LOG_OUT("error :%s return\n", __FUNCTION__);
		return;
	}

	clientInfo.sock = flx_client_node->sock;
	clientInfo.clientType = flx_client_node->clientType;

    while(1)
    {
		//TODO: printf("waiting data ...\n");
		iRetrun = recv(clientInfo.sock, recBuffer, RECV_DATA_BUFFER, 0);

		//TODO:  printf("= %d\n", iRetrun);
		if (iRetrun > 0)
		{
		 	memcpy(&mainRecBuffer[recvLen], recBuffer, iRetrun);
			recvLen += iRetrun;

			while(recvLen >= 9)
			{	//最终由callback_recv_data_from_network完成数据协议的处理
				if(!output_client_recv_data(DATA_SOURCE_PAD_OR_0, mainRecBuffer, &recvLen, &clientInfo))
					break;
			}
		}
		else // iRetrun <= 0
		{
			delete_flx_client_node(clientInfo.sock);
			printf("%s closed, delete socket %d. %s\n", DESCRIBE_PAD_OR_0, clientInfo.sock, strerror(errno));
			return;
		}
    }
}

/* 注册客户端 */
int tps_reg_client_pad_or_0(FLXSocket sock)
{
	FLXThread pid;
	FLX_CLIENT_NODE flx_client_node;
	if(!(flx_client_node = add_flx_client_node(sock)))//将socket加入g_flx_client_node_head链表
		return 0;
	//监听接收客户端数据
	if(thread_create(&pid, NULL, (void *)thread_func_client_pad_or_0, flx_client_node) != 0)
	{
		delete_flx_client_node(sock);
		CCC_LOG_OUT("tps_reg_client_pad_or_0 create thread error\n");
		return 0;
	}
	return 1;
}

/* -----------------------------------------
 * 以下为保存从机客户端的代码
 * 使用另一个队列
 * ----------------------------------------- */

static FLX_CLIENT_NODE     g_flx_client_node_slave_head = NULL;
static pthread_mutex_t     g_flx_client_slave_mutex = PTHREAD_MUTEX_INITIALIZER;


int delete_flx_client_node_slave(FLXSocket sock)
{
	int result;
	pthread_mutex_lock(&g_flx_client_slave_mutex);
	result = interval_delete_flx_client_node(&g_flx_client_node_slave_head, sock);
	pthread_mutex_unlock(&g_flx_client_slave_mutex);
	return result;
}

FLX_CLIENT_NODE add_flx_client_node_slave(FLXSocket sock)
{
	FLX_CLIENT_NODE result;
	pthread_mutex_lock(&g_flx_client_slave_mutex);
	result = interval_add_flx_client_node(&g_flx_client_node_slave_head, sock);
	pthread_mutex_unlock(&g_flx_client_slave_mutex);
	return result;
}

/* 接收从机发送过来的数据 */
static void thread_func_client_slave(void *param)
{
	CLIENT_INFO_STRU clientInfo;
	FLXByte recBuffer[RECV_DATA_BUFFER];
	FLXByte mainRecBuffer[RECV_DATA_BUFFER * 2];
	FLXInt32 iRetrun, recvLen = 0;
	FLX_CLIENT_NODE flx_client_node = (FLX_CLIENT_NODE)param;
	if(!flx_client_node)
	{
		CCC_LOG_OUT("error :%s return\n", __FUNCTION__);
		return;
	}

	clientInfo.sock = flx_client_node->sock;
    while(1)
    {
		printf("waiting data ...\n");
		iRetrun = recv(clientInfo.sock, recBuffer, RECV_DATA_BUFFER, 0);

		printf("= %d\n", iRetrun);
		if (iRetrun > 0)
		{
		 	memcpy(&mainRecBuffer[recvLen],recBuffer,iRetrun);
			recvLen += iRetrun;

			while(recvLen >= 9)
			{
				if(!output_client_recv_data(DATA_SOURCE_SLAVE_SERVER, mainRecBuffer, &recvLen, &clientInfo))
					break;
			}
		}
		else // if (iRetrun <= 0 )
		{
			delete_flx_client_node_slave(clientInfo.sock);
			printf("%s closed, delete socket %d.\n", "slave", clientInfo.sock);
			return;
		}
    }
}

int tps_reg_client_slave_server(FLXSocket sock)
{
	FLXThread pid;
	FLX_CLIENT_NODE flx_client_node;
	if(!(flx_client_node = add_flx_client_node_slave(sock)))
		return 0;

	if(thread_create(&pid, NULL, (void *)thread_func_client_slave, flx_client_node) != 0)
	{
		delete_flx_client_node_slave(sock);
		CCC_LOG_OUT("tps_reg_client_pad_or_0 create thread error\n");
		return 0;
	}
	return 1;
}

void interval_send_data_to_all_slave(FLX_CLIENT_NODE *head, unsigned char *data, unsigned int data_len)
{
	FLX_CLIENT_NODE node;
	if(!(*head) || !data || data_len <= 0)
		return;

	node = *head;
	while(node)
	{
		tcp_server_sendData(node->sock, data, data_len);
		node = node->next;
	}
}

void send_data_to_all_slave(unsigned char *data, unsigned int data_len)
{
	pthread_mutex_lock(&g_flx_client_slave_mutex);
	interval_send_data_to_all_slave(&g_flx_client_node_slave_head, data, data_len);
	pthread_mutex_unlock(&g_flx_client_slave_mutex);
}

void interval_send_data_to_other_slave(FLX_CLIENT_NODE *head, unsigned char *data, unsigned int data_len, FLXSocket sock)
{
	FLX_CLIENT_NODE node;
	if(!(*head) || !data || data_len <= 0)
		return;

	node = *head;
	while(node)
	{
		if(node->sock != sock) tcp_server_sendData(node->sock, data, data_len);
		node = node->next;
	}
}

/* 主机把来自一台从机的数据转发到其他从机 */
void send_data_to_other_slave(FLXSocket sock, unsigned char *data, unsigned int data_len)
{
	pthread_mutex_lock(&g_flx_client_slave_mutex);
	interval_send_data_to_other_slave(&g_flx_client_node_slave_head, data, data_len, sock);
	pthread_mutex_unlock(&g_flx_client_slave_mutex);
}

/*
 * 删除所有的客户端
 * pad
 * 从机
 */
void tps_release_all_client(void)
{
	pthread_mutex_lock(&g_flx_client_slave_mutex);
	interval_delete_all_flx_client_node(&g_flx_client_node_slave_head);
	pthread_mutex_unlock(&g_flx_client_slave_mutex);

	pthread_mutex_lock(&g_flx_client_mutex);
	interval_delete_all_flx_client_node(&g_flx_client_node_head);
	pthread_mutex_unlock(&g_flx_client_mutex);
}

int tps_client_manage_init(tps_client_callback callback)
{
	g_tps_client_call                                   