/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : cccdp_send.c
  作者    : 贾延刚
  生成日期 : 2013-04-01

  版本    : 1.0
  功能描述 : 发送缓存链表
            保存所有的接收数据，通过回调函数输出数据：pdu或普通数据


  修改历史 :
******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "FLXCommon/flxthread.h"
#include "packet_pool/packet_pool.h"

#include "cccdp_send_queue.h"
#include "cccdp_send.h"
#include "tcp_server/tcp_share_type.h"


typedef struct cccdp_send_node *CCCDP_SEND_NODE;
struct cccdp_send_node
{
	int                connect_type;      /* 缓存中数据的来源，目前有telnet, serial, client等等 */
	int                connect_no;        /* 节点对应的通讯连接的编号 */
	CCCDP_SEND_QUEUE   send_queue;

	CCCDP_SEND_NODE    prev;
	CCCDP_SEND_NODE    next;
};

typedef struct cccdp_send_list
{
	CCCDP_SEND_NODE    head;
	CCCDP_SEND_NODE    tail;
	pthread_mutex_t    csl_mutex;

	int                ring_size;
}CCCDP_SEND_LIST;

static CCCDP_SEND_LIST      g_cccdp_send_list     = {NULL, NULL, PTHREAD_MUTEX_INITIALIZER, 0};
static cccdp_send_callback  g_cccdp_send_callback = NULL;

static void invoke_cccdp_send_callback(int connect_type, int connect_no, CCCPACKET *cccpacket)
{
	if(g_cccdp_send_callback) g_cccdp_send_callback(connect_type, connect_no, cccpacket);
}

static CCCDP_SEND_NODE internal_create_cccdp_send_node()
{
	CCCDP_SEND_NODE lp_node = (CCCDP_SEND_NODE)malloc(sizeof(struct cccdp_send_node));
	if(!lp_node)
		return NULL;

	lp_node->connect_type     = -1;
	lp_node->connect_no       = -1;
	cccdp_send_queue_init(&lp_node->send_queue);

	lp_node->prev = NULL;
	lp_node->next = NULL;
	return lp_node;
}

static void internal_free_cccdp_send_node(CCCDP_SEND_NODE lp_node)
{
	cccdp_send_queue_release(&lp_node->send_queue);
	free(lp_node);
}

/* 新节点追加到尾部 */
static void internal_insert_cccdp_send_node(CCCDP_SEND_LIST *lp_send_list, CCCDP_SEND_NODE lp_node)
{
	lp_node->prev = NULL;
	lp_node->next = NULL;
	if(lp_send_list->ring_size <= 0)
	{
		lp_send_list->head = lp_node;
		lp_send_list->tail = lp_node;
	}
	else
	{
		lp_send_list->tail->next = lp_node;
		lp_node->prev            = lp_send_list->tail;
		lp_send_list->tail       = lp_node;
	}
	lp_send_list->ring_size++;
}

/*
 * 删掉指定的节点
 * 返回值：
 *      0 不存在这个节点， 否则返回1
 */
static int internal_cccdp_send_delete_node(CCCDP_SEND_LIST *lp_send_list, int connect_type, int connect_no)
{
	CCCDP_SEND_NODE node;
	if(!lp_send_list->head)
		return 0;

	node = lp_send_list->head;
	while(node)
	{
		if(node->connect_type == connect_type && node->connect_no == connect_no)
		{
			CCCDP_SEND_NODE temp_node;
			if(node->prev) node->prev->next = node->next;
			if(node->next) node->next->prev = node->prev;

			temp_node = node;
			node = node->next;

			/* 如果是删除的头节点或尾节点，则头节点或尾节点指针需要移动*/
			if(lp_send_list->head == temp_node) lp_send_list->head = node;
			if(lp_send_list->tail == temp_node) lp_send_list->tail = node;

			internal_free_cccdp_send_node(temp_node);
			lp_send_list->ring_size--;
			return 1;
		}

		node = node->next;
	}
	return 0;
}

static CCCDP_SEND_NODE search_cccdp_send_node(CCCDP_SEND_LIST *lp_send_list, int connect_type, int connect_no)
{
	CCCDP_SEND_NODE node;
	if(!lp_send_list)
		return NULL;

	node = lp_send_list->head;
	while(node)
	{
		if(node->connect_type == connect_type && node->connect_no == connect_no)
		{
			return node;
		}
		node = node->next;
	}

	return NULL;
}

/*
 * TODO:还需确认通过那个元素来唯一的识别这个节点
 *
 * 创建节点，并追加到链表尾部
 */
static int interval_cccdp_send_add_node(CCCDP_SEND_LIST *lp_send_list, int connect_type, int connect_no)
{
	CCCDP_SEND_NODE lp_node = search_cccdp_send_node(lp_send_list, connect_type, connect_no);
	if(NULL == lp_node)
	{
		lp_node = internal_create_cccdp_send_node();
		if(NULL == lp_node)
			return 0;
	}

	lp_node->connect_type = connect_type;
	lp_node->connect_no   = connect_no;

	internal_insert_cccdp_send_node(lp_send_list, lp_node);
	return 1;
}

static int internal_cccdp_send_save_data(CCCDP_SEND_LIST *lp_send_list, int connect_type, int connect_no, const unsigned char *buffer, int data_len)
{
	CCCDP_SEND_NODE db_node = search_cccdp_send_node(lp_send_list, connect_type, connect_no);
	if(db_node)
	{
		CCCPACKET *cccpacket = get_packet_from_packet_pool();
		if(!cccpacket)
			return 0;

		if(!cccpacket_save_normal_data(cccpacket, buffer, data_len))
			return 0;

		return cccdp_send_queue_add_packet(&db_node->send_queue, cccpacket);
	}
	return 0;
}


/*
 * 轮询所有节点的发送列表
 * 每个节点都是一个连接的发送列表
 */
static int internal_data_pool_poll_send_data(CCCDP_SEND_LIST *lp_send_list)
{
	CCCDP_SEND_NODE  node;
	CCCPACKET *cccpacket = NULL;
	if(!lp_send_list || !lp_send_list->tail)
		return 0;

	node = lp_send_list->tail;
	while(node)
	{

		while((cccpacket = cccdp_send_queue_get_head(&node->send_queue)) != NULL)
		{
			invoke_cccdp_send_callback(node->connect_type, node->connect_no, cccpacket);
		}

		node = node->prev;
	}
	return 0;
}

/*
 * 线程函数
 * 轮询接收链表，根据接收数据的类型，分别输出已接收的数据
 */
void thread_func_cccdp_send_poll(void *param)
{
	while(1)
	{
		pthread_mutex_lock(&g_cccdp_send_list.csl_mutex);
		internal_data_pool_poll_send_data(&g_cccdp_send_list);
		pthread_mutex_unlock(&g_cccdp_send_list.csl_mutex);
	}
}

/* 追加接收数据 */
int cccdp_send_save_data(int connect_type, int connect_no, const unsigned char *buffer, int data_len)
{
	int result;
	pthread_mutex_lock(&g_cccdp_send_list.csl_mutex);
	result = internal_cccdp_send_save_data(&g_cccdp_send_list, connect_type, connect_no, buffer, data_len);
	pthread_mutex_unlock(&g_cccdp_send_list.csl_mutex);
	return result;
}
/* 删除节点 */
int cccdp_send_delete_node(int connect_type, int connect_no)
{
	int result;
	pthread_mutex_lock(&g_cccdp_send_list.csl_mutex);
	result = internal_cccdp_send_delete_node(&g_cccdp_send_list, connect_type, connect_no);
	pthread_mutex_unlock(&g_cccdp_send_list.csl_mutex);
	return result;
}
/* 增加节点 */
int cccdp_send_add_node(int connect_type, int connect_no)
{
	int result;
	pthread_mutex_lock(&g_cccdp_send_list.csl_mutex);
	result = interval_cccdp_send_add_node(&g_cccdp_send_list, connect_type, connect_no);
	pthread_mutex_unlock(&g_cccdp_send_list.csl_mutex);
	return result;
}

int cccdp_send_init(cccdp_send_callback callback)
{
	FLXThread pid;
	g_cccdp_send_callback = callback;
	if(thread_create(&pid, NULL, (void *)thread_func_cccdp_send_poll, NULL) != 0)
		return 0;

	return 1;
}
