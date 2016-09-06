/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : connecting_list.c
  ����    : ���Ӹ�
  �������� : 2012-11-19

  �汾    : 1.0
  �������� : ������Ҫ���ӵ�socket�Ķ����б���һ��������У�
            �½ڵ���ӵ�ͷ
           �̻߳����ʱ�����һ�£����ӳɹ���
           ���Զ�ɾ���ڵ�

  �޸���ʷ :

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <semaphore.h>

#include "FLXCommon/flxnettypes.h"
#include "FLXCommon/flxthread.h"
#include "FLXCommon/tcpserver.h"
#include "connecting_list.h"
#include "util_func.h"
#include "util_log.h"
#include "tcp_share_type.h"

#define CONNECTING_DELAY_INTERVAL   2000                 /* ���� */

typedef struct connecting_node *CONNECTING_NODE;
struct connecting_node
{
	int                connect_type;
	int                connect_no;
	FLXSocket          sock;					/* �׽��� */
	struct sockaddr_in remote_address;
	int                b_connected;
	unsigned int       start_time;              /* ִ����connectʱ��ʱ�� */

	CONNECTING_NODE    prev;
	CONNECTING_NODE    next;
};

static CONNECTING_NODE      g_connecting_node_head    = NULL;
static pthread_mutex_t      g_connecting_list_mutex_t = PTHREAD_MUTEX_INITIALIZER;
static sem_t                g_connecting_list_sem;
static connecting_callback  g_connecting_callback     = NULL;

static CONNECTING_NODE connecting_node_create()
{
	CONNECTING_NODE lp_node = (CONNECTING_NODE)malloc(sizeof(struct connecting_node));
	if(!lp_node)
		return NULL;

	lp_node->connect_type   = CONNECT_TYPE_UNKNOWN;
	lp_node->connect_no     = -1;
	lp_node->sock           = INVALID_SOCKET;
	lp_node->b_connected    = 0;
	lp_node->start_time     = 0;

	lp_node->prev           = NULL;
	lp_node->next           = NULL;
	return lp_node;
}

static void free_connecting_node(CONNECTING_NODE lp_node)
{
	free(lp_node);
}

static void insert_connecting_node(CONNECTING_NODE *head, CONNECTING_NODE lp_node)
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


static void internal_connecting_node_delete_all(CONNECTING_NODE *head)
{
	CONNECTING_NODE temp_node, node;
	if(!(*head))
		return;

	node = *head;
	while(node)
	{
		temp_node = node;
		node = node->next;

		free_connecting_node(temp_node);
	}
	*head = NULL;
	return;
}
/*
 * ɾ�����������ӳɹ��Ľڵ�
 */
static void internal_connecting_node_delete_connected(CONNECTING_NODE *head)
{
	CONNECTING_NODE node;
	if(!(*head))
		return;

	node = *head;
	while(node)
	{
		if(node->b_connected)
		{
			CONNECTING_NODE temp_node;
			if(node->prev) node->prev->next = node->next;
			if(node->next) node->next->prev = node->prev;

			temp_node = node;
			node = node->next;

			/* �����ɾ����ͷ�ڵ㣬��ͷ�ڵ�ָ����Ҫ�ƶ�*/
			if(*head == temp_node) *head = node;
			free_connecting_node(temp_node);
			continue;
		}

		node = node->next;
	}
}
/*
 * ɾ��һ��ָ���ڵ�
 * ����ֵ��
 *      1 �ýڵ����
 *      0 �ýڵ㲻����
 */
static int internal_connecting_node_delete(CONNECTING_NODE *head, int connect_type, int connect_no)
{
	int result = 0;
	CONNECTING_NODE node;
	if(!(*head))
		return 0;

	node = *head;
	while(node)
	{
		if(node->connect_type == connect_type && node->connect_no == connect_no)
		{
			CONNECTING_NODE temp_node;
			if(node->prev) node->prev->next = node->next;
			if(node->next) node->next->prev = node->prev;

			temp_node = node;
			node = node->next;

			/* �����ɾ����ͷ�ڵ㣬��ͷ�ڵ�ָ����Ҫ�ƶ�*/
			if(*head == temp_node) *head = node;
			free_connecting_node(temp_node);
			result = 1;
			continue;             // TODO: �Ƿ���Ҫ�����ɾ��
		}

		node = node->next;
	}
	return result;
}

static CONNECTING_NODE internal_connecting_node_search(CONNECTING_NODE *head, int connect_type, int connect_no)
{
	CONNECTING_NODE node;
	if(!(*head))
		return NULL;

	node = *head;
	while(node)
	{
		if(connect_type == node->connect_type &&
		   connect_no   == node->connect_no)
			return node;

		node = node->next;
	}
	return NULL;
}
/*
 * ���ӽڵ�
 * ����Ѿ�����ͬ�Ľڵ㣬��ֱ�ӷ��سɹ�
 * ����ֵ��
 *      1�ɹ���0ʧ��
 */
static int internal_connecting_node_add(CONNECTING_NODE *head, int connect_type, int connect_no, const char *ip_address, int port)
{
	CONNECTING_NODE lp_node;
	if(!ip_address)   // !(*head) ||
		return 0;

	lp_node = internal_connecting_node_search(head, connect_type, connect_no);
	if(lp_node != NULL)
		return 1;

	lp_node = connecting_node_create();
	if(NULL == lp_node)
		return 0;

	memset(&lp_node->remote_address, 0, sizeof(struct sockaddr_in));
	lp_node->remote_address.sin_family = AF_INET;
	lp_node->remote_address.sin_port   = htons(port);
	inet_pton(AF_INET, ip_address, (void *)&(lp_node->remote_address.sin_addr));

	lp_node->connect_type = connect_type;
	lp_node->connect_no   = connect_no;
	insert_connecting_node(head, lp_node);
	return 1;
}

static void connecting_node_delete_connected()
{
	pthread_mutex_lock(&g_connecting_list_mutex_t);
	internal_connecting_node_delete_connected(&g_connecting_node_head);
	pthread_mutex_unlock(&g_connecting_list_mutex_t);
}

static int set_nonblocking(FLXSocket sock)
{
	int opts = fcntl(sock, F_GETFL);
	if(opts < 0)
	{
		CCC_LOG_OUT("%s fcntl(sock, F_GETFL) failed: %s\n", __FUNCTION__, strerror(errno));
		return 0;
	}

	opts |= O_NONBLOCK;
	if(fcntl(sock, F_SETFL, opts) < 0)
	{
		CCC_LOG_OUT("%s fcntl(sock, F_SETFL) failed: %s\n", __FUNCTION__, strerror(errno));
		return 0;
	}
	return 1;
}

static FLXSocket create_connect_handle()
{
	FLXSocket sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		CCC_LOG_OUT("%s create socket failed: %s\n", __FUNCTION__, strerror(errno));
		return INVALID_SOCKET;
	}

	if(!set_nonblocking(sock))
	{
		close(sock);
		return INVALID_SOCKET;
	}
	return sock;
}

static int connect_to_server(FLXSocket sock, struct sockaddr_in *remote_address)
{
	int bconnected = connect(sock, (struct sockaddr *)remote_address, sizeof(struct sockaddr_in));

	//if(0 == bconnected) printf("connect(%d) success.\n", (int)sock);
	//else                printf("connect(%d): %s\n", (int)sock, strerror(errno));
	return (0 == bconnected) ? 1:0;
}

static void notify_sokcet_connected(int connect_type, int connect_no, FLXSocket sock)
{
	if(g_connecting_callback)
		g_connecting_callback(connect_type, connect_no, sock);
}

static void execute_connect(CONNECTING_NODE node)
{
	if(INVALID_SOCKET == node->sock || !node)
		return;

	if(connect_to_server(node->sock, &node->remote_address))
	{
		notify_sokcet_connected(node->connect_type, node->connect_no, node->sock);
		node->b_connected = 1;
	}
	else
	{
		node->start_time = util_get_m_second();
	}
}

/*
 * ��ѯ���нڵ�
 * ����socket�����������
 */
static int internal_polling_connecting_node(CONNECTING_NODE *head)
{
	int result = 0;
	CONNECTING_NODE node;
	if(!(*head))
		return result;

	node = *head;
	while(node)
	{
		if(INVALID_SOCKET == node->sock)
		{
			node->sock = create_connect_handle();
			if(node->sock != INVALID_SOCKET) execute_connect(node);
		}
		else
		{
			if(!node->b_connected)
			{
				unsigned int current_time = util_get_m_second();
				if(labs(current_time - node->start_time) > CONNECTING_DELAY_INTERVAL)
				{
					execute_connect(node);
				}
			}
		}
		node = node->next;
		result += 1;
	}
	return result;
}

static int polling_connecting_node()
{
	int result;
	pthread_mutex_lock(&g_connecting_list_mutex_t);
	result = internal_polling_connecting_node(&g_connecting_node_head);
	pthread_mutex_unlock(&g_connecting_list_mutex_t);
	return result;
}

/*
 * �����߳�
 */
void thread_func_connecting(void *param)
{
	while(1)
	{
		sem_wait(&g_connecting_list_sem);
		while(1)
		{
			/* ֻҪ�������нڵ㣬��һֱѭ�� */
			if(polling_connecting_node() <= 0)
				break;
			connecting_node_delete_connected();
		}
	}
}
int connecting_list_delete(int connect_type, int connect_no)
{
	int result;
	pthread_mutex_lock(&g_connecting_list_mutex_t);
	result = internal_connecting_node_delete(&g_connecting_node_head, connect_type, connect_no);
	pthread_mutex_unlock(&g_connecting_list_mutex_t);
	return result;
}

int connecting_list_add(int connect_type, int connect_no, const char *ip_address, int port)
{
	int result;
	pthread_mutex_lock(&g_connecting_list_mutex_t);
	result = internal_connecting_node_add(&g_connecting_node_head, connect_type, connect_no, ip_address, port);
	pthread_mutex_unlock(&g_connecting_list_mutex_t);
	if(result) sem_post(&g_connecting_list_sem);
	return result;
}

/*
 * ɾ�����еĽڵ�
 */
void connecting_list_release(void)
{
	pthread_mutex_lock(&g_connecting_list_mutex_t);
	internal_connecting_node_delete_all(&g_connecting_node_head);
	pthread_mutex_unlock(&g_connecting_list_mutex_t);
}

int connecting_list_init(connecting_callback callback)
{
	FLXThread pid;
	sem_init(&g_connecting_list_sem, 0, 0);
	if(thread_create(&pid, NULL, (void *)thread_func_connecting, NULL) != 0)
		return 0;

	g_connecting_callback = callback;
	return 1;
}
