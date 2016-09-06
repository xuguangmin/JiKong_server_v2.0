/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : connect_list.c
  ����    : ���Ӹ�
  �������� : 2012-11-16

  �汾    : 1.0
  �������� : �����������ӵ���Ϣ


  �޸���ʷ :

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "connect_list.h"
#include "connecting_list.h"
#include "util_func.h"
#include "util_log.h"
#include "FLXCommon/flxnettypes.h"
#include "FLXCommon/tcpserver.h"
#include "tcp_share_type.h"

static CONNECT_NODE           g_connect_node_head     = NULL;
static pthread_mutex_t        g_connect_mutex         = PTHREAD_MUTEX_INITIALIZER;
static connect_list_callback  g_connect_list_callback = NULL;

static CONNECT_NODE make_connect_node()
{
	CONNECT_NODE lp_node = (CONNECT_NODE)malloc(sizeof(struct connect_node));
	if(!lp_node)
		return NULL;


	lp_node->connect_type   = CONNECT_TYPE_UNKNOWN;
	lp_node->connect_no     = -1;
	lp_node->sock           = INVALID_SOCKET;
	lp_node->ip_address     = NULL;

	lp_node->prev           = NULL;
	lp_node->next           = NULL;
	return lp_node;
}

static void free_connect_node(CONNECT_NODE lp_node)
{
	close(lp_node->sock);
	if(lp_node->ip_address) free(lp_node->ip_address);
	free(lp_node);
}

static void insert_connect_node(CONNECT_NODE *head, CONNECT_NODE lp_node)
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
 * ɾ��һ���ڵ�
 * ����ֵ��
 *      1 �ýڵ����
 *      0 �ýڵ㲻����
 */
static int internal_delete_connect_node(CONNECT_NODE *head, int connect_type, int connect_no)
{
	CONNECT_NODE node;
	if(!(*head))
		return 0;

	node = *head;
	while(node)
	{
		if(node->connect_type == connect_type && node->connect_no == connect_no)
		{
			CONNECT_NODE temp_node;
			if(node->prev) node->prev->next = node->next;
			if(node->next) node->next->prev = node->prev;

			temp_node = node;
			node = node->next;

			/* �����ɾ����ͷ�ڵ㣬��ͷ�ڵ�ָ����Ҫ�ƶ�*/
			if(*head == temp_node) *head = node;
			free_connect_node(temp_node);
			return 1;
		}

		node = node->next;
	}
	return 0;
}

static void internal_delete_all_connect_node(CONNECT_NODE *head)
{
	CONNECT_NODE temp_node, node;
	if(!(*head))
		return;

	node = *head;
	while(node)
	{
		temp_node = node;
		node = node->next;

		free_connect_node(temp_node);
	}
	*head = NULL;
	return;
}

/* ����   �����ڵ� */
static CONNECT_NODE internal_search_connect_node(CONNECT_NODE *head, int connect_type, int connect_no)
{
	CONNECT_NODE node;
	if(!(*head))
		return NULL;

	node = *head;
	while(node)
	{
		if(node->connect_type == connect_type && node->connect_no == connect_no) return node;
		node = node->next;
	}
	return NULL;
}

/* ����sock�����ڵ� */
static CONNECT_NODE internal_search_connect_node_by_sock(CONNECT_NODE *head, FLXSocket sock)
{
	CONNECT_NODE node;
	if(!(*head))
		return NULL;

	node = *head;
	while(node)
	{
		if(node->sock == sock) return node;
		node = node->next;
	}
	return NULL;
}

static CONNECT_NODE internal_modify_connect_node_value_sock(CONNECT_NODE *head, int connect_type, int connect_no, FLXSocket sock)
{
	CONNECT_NODE lp_node = internal_search_connect_node(head, connect_type, connect_no);
	if(NULL == lp_node)
		return NULL;

	lp_node->sock = sock;
	return lp_node;
}

static CONNECT_NODE internal_connect_list_close_connect(CONNECT_NODE *head, FLXSocket sock)
{
	CONNECT_NODE lp_node = internal_search_connect_node_by_sock(head, sock);
	if(NULL == lp_node)
		return NULL;

	close(lp_node->sock);
	lp_node->sock = INVALID_SOCKET;
	return lp_node;
}

CONNECT_NODE modify_connect_node_value_sock(int connect_type, int connect_no, FLXSocket sock)
{
	CONNECT_NODE result;
	pthread_mutex_lock(&g_connect_mutex);
	result = internal_modify_connect_node_value_sock(&g_connect_node_head, connect_type, connect_no, sock);
	pthread_mutex_unlock(&g_connect_mutex);
	return result;
}

CONNECT_NODE search_connect_node_by_sock(FLXSocket sock)
{
	CONNECT_NODE result;
	pthread_mutex_lock(&g_connect_mutex);
	result = internal_search_connect_node_by_sock(&g_connect_node_head, sock);
	pthread_mutex_unlock(&g_connect_mutex);
	return result;
}
static CONNECT_NODE connect_list_search(int connect_type, int connect_no)
{
	CONNECT_NODE result;
	pthread_mutex_lock(&g_connect_mutex);
	result = internal_search_connect_node(&g_connect_node_head, connect_type, connect_no);
	pthread_mutex_unlock(&g_connect_mutex);
	return result;
}

int delete_connect_node(int connect_type, int connect_no)
{
	int result;
	pthread_mutex_lock(&g_connect_mutex);
	result = internal_delete_connect_node(&g_connect_node_head, connect_type, connect_no);
	pthread_mutex_unlock(&g_connect_mutex);
	return result;
}

/*
 * ������socket����Ƿ���Ч
 */
int connect_send_data(int connect_type, int connect_no, unsigned char *buffer, int data_len)
{
	CONNECT_NODE connect_node;
	if(!buffer || data_len <= 0)
		return 0;

	connect_node = connect_list_search(connect_type, connect_no);
	if(!connect_node || INVALID_SOCKET == connect_node->sock)
		return 0;

	return (0 == tcp_server_sendData(connect_node->sock, buffer, data_len)) ? 1:0;
}

/*
 * �رյ�ǰ���ӵľ��
 * ��������Ϣ�������������б�ȥ���½�������
 */
int connect_list_reconnect(FLXSocket sock)
{
	CONNECT_NODE connect_node;
	pthread_mutex_lock(&g_connect_mutex);
	connect_node = internal_connect_list_close_connect(&g_connect_node_head, sock);
	pthread_mutex_unlock(&g_connect_mutex);

	if(!connect_node)
	{
		printf("%s connect_node NULL.\n", __FUNCTION__);
		return 0;
	}
	printf("%s(%d) closed\n", connect_node->ip_address, sock);

	if(!connecting_list_add(connect_node->connect_type, connect_node->connect_no, connect_node->ip_address, connect_node->ip_port))
	{
		printf("%s add_connecting_node error.\n", __FUNCTION__);
		return 0;
	}
	return 1;
}

/*
 * �������ӽڵ�
 * ����ڵ��Ѿ����ڣ���ֱ�ӷ��ؽڵ�
 * ������
 *     connect_type  �������ͣ�ȡֵ��CONNECT_TYPE_TELNET, CONNECT_TYPE_TELNET_FOR_HTTP, CONNECT_TYPE_CLIENT
 *     ip_address
 *     port,
 *     user_data     ���Դ���һ���û��Զ��������
 *     new_node      �����ӵĽڵ㣬����Ѵ��ڣ���Ϊ�Ѵ��ڵĽڵ�
 *
 * ����ֵ��
 *     1 ������0 ����-1 �ڵ��Ѵ��ڡ�����ֵ������
 */
static int internal_add_connect_node(CONNECT_NODE *head, int connect_type, int connect_no, const char *ip_address, int port)
{
	char *ip;
	CONNECT_NODE lp_node;
	if(!ip_address)
		return 0;

	ip = util_strcpy(ip_address);
	if(!ip)
		return 0;

	lp_node = make_connect_node();
	if(NULL == lp_node)
		return 0;

	lp_node->ip_address   = ip;
	lp_node->ip_port      = port;
	lp_node->connect_type = connect_type;
	lp_node->connect_no   = connect_no;

	insert_connect_node(head, lp_node);
	return 1;
}

static int internal_add_connect_node_telnet(CONNECT_NODE *head, int connect_type, int connect_no, const char *ip_address, int port)
{
	CONNECT_NODE lp_node;
	if(!ip_address)
		return 0;

	/* ���ͬ��IP�Ͷ˿ڵĽڵ��Ƿ��Ѿ�����	 */
	lp_node = internal_search_connect_node(head, connect_type, connect_no);
	if(lp_node)
		return 1;

	return internal_add_connect_node(head, connect_type, connect_no, ip_address, port);
}

static int internal_add_connect_node_http(CONNECT_NODE *head, int connect_type, int connect_no, const char *ip_address, int port)
{
	CONNECT_NODE lp_node;
	if(!ip_address)
		return 0;

	/* ���ͬ��IP�Ͷ˿ڵĽڵ��Ƿ��Ѿ�����	 */
	lp_node = internal_search_connect_node(head, connect_type, connect_no);
	if(lp_node)
	{
		connecting_list_delete(connect_type, connect_no);                               /* �������Ƕ�� */
		internal_delete_connect_node(head, connect_type, connect_no);
	}

	return internal_add_connect_node(head, connect_type, connect_no, ip_address, port);
}

static int add_connect_node_telnet(int connect_type, int connect_no, const char *ip_address, int port)
{
	int result;
	pthread_mutex_lock(&g_connect_mutex);
	result = internal_add_connect_node_telnet(&g_connect_node_head, connect_type, connect_no, ip_address, port);
	pthread_mutex_unlock(&g_connect_mutex);
	return result;
}

static int add_connect_node_http(int connect_type, int connect_no, const char *ip_address, int port)
{
	int result;
	pthread_mutex_lock(&g_connect_mutex);
	result = internal_add_connect_node_http(&g_connect_node_head, connect_type, connect_no, ip_address, port);
	pthread_mutex_unlock(&g_connect_mutex);
	return result;
}

/*
 * �������ӽڵ�
 * ���������ӷ����������Ӷ��У��������������������
 * ����ڵ��Ѿ����ڣ���ֱ�ӷ����Ѵ��ڽڵ�
 * ������
 *     connect_type  �������ͣ�ȡֵ��CONNECT_TYPE_TELNET, CONNECT_TYPE_TELNET_FOR_HTTP, CONNECT_TYPE_CLIENT
 *     ip_address
 *     port,
 *     user_data     ���Դ���һ���û��Զ��������
 *
 * ����ֵ��
 *
 */
int connect_list_add_telnet(int connect_type, int connect_no, const char *ip_address, int port)
{
	if(!add_connect_node_telnet(connect_type, connect_no, ip_address, port))
	{
		printf("%s %s error\n", __FUNCTION__, "add_connect_node_telnet()");
		return 0;
	}

	if(!connecting_list_add(connect_type, connect_no, ip_address, port))
	{
		printf("%s %s error\n", __FUNCTION__, "connecting_list_add()");
		return 0;
	}

	return 1;
}

int connect_list_add_http(int connect_type, int connect_no, const char *ip_address, int port)
{
	if(!add_connect_node_http(connect_type, connect_no, ip_address, port))
		return 0;

	if(!connecting_list_add(connect_type, connect_no, ip_address, port))
		return 0;

	return 1;
}

int connect_list_delete(int connect_type, int connect_no)
{
	int result = 1;
	if(!connecting_list_delete(connect_type, connect_no))
	{
		CCC_LOG_OUT("%s connecting_list_delete failed.\n", __FUNCTION__);
		result = 0;
	}

	result = delete_connect_node(connect_type, connect_no);
	return result;
}
/*
 * �ص�����
 * ��connecting_list����
 * ��ȡ�Ѿ����ӳɹ��ľ��
 */
int callback_by_connecting_list(int connect_type, int connect_no, FLXSocket sock)
{
	CONNECT_NODE conn_node = modify_connect_node_value_sock(connect_type, connect_no, sock);
	if(!conn_node)
		return 0;

	if(g_connect_list_callback) g_connect_list_callback(conn_node);
	return 1;
}
/*
 * ɾ�����еĽڵ�
 */
void connect_list_release(void)
{
	connecting_list_release();

	pthread_mutex_lock(&g_connect_mutex);
	internal_delete_all_connect_node(&g_connect_node_head);
	pthread_mutex_unlock(&g_connect_mutex);
}

int connect_list_init(connect_list_callback callback)
{
	if(!connecting_list_init(callback_by_connecting_list))
		return 0;

	g_connect_list_callback = callback;
	return 1;
}

