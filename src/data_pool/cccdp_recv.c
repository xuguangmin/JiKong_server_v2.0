/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : cccdp_recv.c
  作者    : 贾延刚
  生成日期 : 2013-03-26

  版本    : 1.0
  功能描述 : 接收缓存链表
            保存所有的接收数据，通过回调函数输出数据：pdu或普通数据


  修改历史 :
******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "FLXCommon/flxthread.h"
#include "packet_pool/packet_pool.h"
#include "cccdp_recv_buffer.h"
#include "cccdp_recv.h"
#include "tcp_server/tcp_share_type.h"


typedef struct cccdp_recv_node *CCCDP_RECV_NODE;
struct cccdp_recv_node
{
	int                connect_type;      /* 缓存中数据的来源，目前有telnet, serial, client等等 */
	int                connect_no;        /* 节点对应的通讯连接的编号 */
	CCCDP_RECV_BUFFER  recv_buf;

	CCCDP_RECV_NODE    prev;
	CCCDP_RECV_NODE    next;
};

typedef struct cccdp_recv_list
{
	CCCDP_RECV_NODE    head;
	CCCDP_RECV_NODE    tail;
	int                ring_size;
	int                using_size;
	pthread_mutex_t    dbl_mutex;
}CCCDP_RECV_LIST;

static CCCDP_RECV_LIST  g_cccdp_recv_list = {NULL, NULL, 0, 0, PTHREAD_MUTEX_INITIALIZER};
static cccdp_recv_callback g_cccdp_recv_callback = NULL;

static void invoke_cccdp_recv_callback(int connect_type, int connect_no, CCCPACKET *dp_packet)
{
	if(g_cccdp_recv_callback) g_cccdp_recv_callback(connect_type, connect_no, dp_packet);
}

static CCCDP_RECV_NODE internal_create_cccdp_recv_node()
{
	CCCDP_RECV_NODE lp_node = (CCCDP_RECV_NODE)malloc(sizeof(struct cccdp_recv_node));
	if(!lp_node)
		return NULL;

	lp_node->connect_type     = -1;
	lp_node->connect_no       = -1;
	cccdp_recv_buffer_init(&lp_node->recv_buf);

	lp_node->prev = NULL;
	lp_node->next = NULL;
	return lp_node;
}
/*TODO: 需要检查是否释放完成 */
static void internal_free_cccdp_recv_node(CCCDP_RECV_NODE lp_node)
{
	cccdp_recv_buffer_release(&lp_node->recv_buf);
	free(lp_node);
}

/* 新节点追加到尾部 */
static void internal_insert_cccdp_recv_node(CCCDP_RECV_LIST *lp_recv_list, CCCDP_RECV_NODE lp_node)
{
	if(lp_recv_list->ring_size <= 0)
	{
		lp_recv_list->head = lp_node;
		lp_recv_list->tail = lp_node;

		lp_node->prev = NULL;
		lp_node->next = NULL;
	}
	else
	{
		lp_node->next          = NULL;
		lp_recv_list->tail->next = lp_node;
		lp_node->prev          = lp_recv_list->tail;
		lp_recv_list->tail       = lp_node;
	}
	lp_recv_list->ring_size++;
}

/*
 * 删掉指定的节点
 * 返回值：
 *      0 不存在这个节点， 否则返回1
 */
static int internal_cccdp_recv_delete_node(CCCDP_RECV_LIST *lp_recv_list, int connect_type, int connect_no)
{
	CCCDP_RECV_NODE node;
	if(!lp_recv_list->head)
		return 0;

	node = lp_recv_list->head;
	while(node)
	{
		if(node->connect_type == connect_type && node->connect_no == connect_no)
		{
			CCCDP_RECV_NODE temp_node;
			if(node->prev) node->prev->next = node->next;
			if(node->next) node->next->prev = node->prev;

			temp_node = node;
			node = node->next;

			/* 如果是删除的头节点或尾节点，则头节点或尾节点指针需要移动*/
			if(lp_recv_list->head == temp_node) lp_recv_list->head = node;
			if(lp_recv_list->tail == temp_node) lp_recv_list->tail = node;

			lp_recv_list->ring_size--;
			lp_recv_list->using_size--;

			internal_free_cccdp_recv_node(temp_node);
			return 1;
		}

		node = node->next;
	}
	return 0;
}

/*
 * 搜索节点未使用的编号
 * 节点编号从1开始，0编号会分配给一个特殊节点
 * 返回值：
 *       0  错误
 *       >0 有效编号
 */
static CCCDP_RECV_NODE internal_search_data_pool_node(CCCDP_RECV_LIST *lp_recv_list, int connect_type, int connect_no)
{
	CCCDP_RECV_NODE node;
	if(!lp_recv_list)
		return NULL;

	node = lp_recv_list->head;
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
static int interval_cccdp_recv_add_node(CCCDP_RECV_LIST *lp_recv_list, int connect_type, int connect_no)
{
	CCCDP_RECV_NODE lp_node = internal_search_data_pool_node(lp_recv_list, connect_type, connect_no);
	if(NULL == lp_node)
	{
		lp_node = internal_create_cccdp_recv_node();
		if(NULL == lp_node)
			return 0;
	}

	lp_node->connect_type = connect_type;
	lp_node->connect_no   = connect_no;

	internal_insert_cccdp_recv_node(lp_recv_list, lp_node);
	return 1;
}

static int internal_cccdp_recv_save_data(CCCDP_RECV_LIST *lp_recv_list, int connect_type, int connect_no, const unsigned char *buffer, int data_len)
{
	CCCDP_RECV_NODE db_node = internal_search_data_pool_node(lp_recv_list, connect_type, connect_no);
	if(!db_node)
		return 0;

	return cccdp_recv_buffer_save_data(&db_node->recv_buf, buffer, data_len);
}

static void internal_output_recv_data_protocol(int connect_type, int connect_no, CCCPACKET *cccpacket, unsigned char *buffer, int *data_len)
{
	//
}
//
static void internal_output_recv_data_normal(int connect_type, int connect_no, unsigned char *buffer, int *data_len)
{
	CCCPACKET *cccpacket = get_packet_from_packet_pool();
	if(!cccpacket)
		return;

	if(!cccpacket_save_normal_data(cccpacket, buffer, *data_len))
		return;

	*data_len = 0;
	invoke_cccdp_recv_callback(connect_type, connect_no, cccpacket);
}

/*
 * 返回值：
 */
static int internal_data_pool_poll_recv_data(CCCDP_RECV_LIST *lp_recv_list)
{
	CCCDP_RECV_NODE  node;
	CCCPACKET *cccpacket = NULL;
	if(!lp_recv_list || !lp_recv_list->tail)
		return 0;

	node = lp_recv_list->tail;
	while(node)
	{
		if(node->recv_buf.data_len > 0)
		{
			switch(node->connect_type)
			{
			case CONNECT_TYPE_CLIENT:
				internal_output_recv_data_protocol(node->connect_type, node->connect_no, cccpacket, node->recv_buf.buf, &node->recv_buf.data_len);
				break;

			default:
				internal_output_recv_data_normal(node->connect_type, node->connect_no, node->recv_buf.buf, &node->recv_buf.data_len);
				break;
			}
		}

		node = node->prev;
	}
	return 0;
}

/*
 * 线程函数
 * 轮询接收链表，根据接收数据的类型，分别输出已接收的数据
 */
void thread_func_cccdp_recv_poll(void *param)
{
	while(1)
	{
		pthread_mutex_lock(&g_cccdp_recv_list.dbl_mutex);
		internal_data_pool_poll_recv_data(&g_cccdp_recv_list);
		pthread_mutex_unlock(&g_cccdp_recv_list.dbl_mutex);
	}
}

/* 追加接收数据 */
int cccdp_recv_save_data(int connect_type, int connect_no, const unsigned char *buffer, int data_len)
{
	int result;
	pthread_mutex_lock(&g_cccdp_recv_list.dbl_mutex);
	result = internal_cccdp_recv_save_data(&g_cccdp_recv_list, connect_type, connect_no, buffer, data_len);
	pthread_mutex_unlock(&g_cccdp_recv_list.dbl_mutex);
	return result;
}
/* 删除节点 */
int cccdp_recv_delete_node(int connect_type, int connect_no)
{
	int result;
	pthread_mutex_lock(&g_cccdp_recv_list.dbl_mutex);
	result = internal_cccdp_recv_delete_node(&g_cccdp_recv_list, connect_type, connect_no);
	pthread_mutex_unlock(&g_cccdp_recv_list.dbl_mutex);
	return result;
}
/* 增加节点 */
int cccdp_recv_add_node(int connect_type, int connect_no)
{
	int result;
	pthread_mutex_lock(&g_cccdp_recv_list.dbl_mutex);
	result = interval_cccdp_recv_add_node(&g_cccdp_recv_list, connect_type, connect_no);
	pthread_mutex_unlock(&g_cccdp_recv_list.dbl_mutex);
	return result;
}

int cccdp_recv_init(cccdp_recv_callback callback)
{
	FLXThread pid;
	g_cccdp_recv_callback = callback;
	if(thread_create(&pid, NULL, (void *)thread_func_cccdp_recv_poll, NULL) != 0)
		return 0;

	return 1;
}
