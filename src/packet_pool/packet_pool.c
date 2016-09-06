/*
 * packet_pool.c
 *
 *  Created on: 2013-3-27
 *      Author: flx
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ccc_packet.h"

typedef struct packet_node *PACKET_NODE;
struct packet_node
{
	CCCPACKET      packet;
	PACKET_NODE    prev;
	PACKET_NODE    next;
};

typedef struct packet_node_list
{
	PACKET_NODE        head;
	PACKET_NODE        tail;
	int                ring_size;
}PACKET_NODE_LIST;

/*
 * 没被使用的包
 * 可以被其他模块取来使用
 */
static PACKET_NODE_LIST  g_packet_list_idle  = {NULL, NULL, 0};
/*
 * 正在使用的包
 * 也包含可能已经不再使用的包
 */
static PACKET_NODE_LIST  g_packet_list_using = {NULL, NULL, 0};
static pthread_mutex_t   g_packet_list_mutex = PTHREAD_MUTEX_INITIALIZER;

static PACKET_NODE internal_create_packet_node()
{
	PACKET_NODE lp_node = (PACKET_NODE)malloc(sizeof(struct packet_node));
	if(!lp_node)
		return NULL;

	lp_node->prev  = NULL;
	lp_node->next  = NULL;

	cccpacket_init(&lp_node->packet);
	return lp_node;
}

static void internal_free_packet_node(PACKET_NODE lp_node)
{
	cccpacket_release(&lp_node->packet);
	free(lp_node);
}

/* 新节点追加到尾部 */
static void internal_insert_packet_node(PACKET_NODE_LIST *lp_db_list, PACKET_NODE lp_node)
{
	lp_node->prev = NULL;
	lp_node->next = NULL;

	if(!lp_db_list->head)
	{
		lp_db_list->head = lp_node;
		lp_db_list->tail = lp_node;
	}
	else
	{
		lp_node->prev          = lp_db_list->tail;
		lp_db_list->tail->next = lp_node;
		lp_db_list->tail       = lp_node;
	}
	lp_db_list->ring_size++;
}

/* 创建节点，并追加到链表尾部 */
static int interval_packet_list_add_node(PACKET_NODE_LIST *lp_db_list)
{
	PACKET_NODE lp_node = internal_create_packet_node();
	if(NULL == lp_node)
		return 0;

	internal_insert_packet_node(lp_db_list, lp_node);
	return 1;
}

static PACKET_NODE interval_get_packet_list_idle_head(PACKET_NODE_LIST *lp_db_list)
{
	if(lp_db_list->head)
	{
		PACKET_NODE node = lp_db_list->head;

		lp_db_list->head = node->next;
		if(lp_db_list->tail == node) lp_db_list->tail = node->next;

		lp_db_list->ring_size--;
		return node;
	}
	return NULL;
}


/*
 * 检查正在使用列表中是否有已经不再使用的包
 * 返回值：
 *      返回包节点，否则为NULL
 */
static PACKET_NODE interval_get_packet_list_using_not_using(PACKET_NODE_LIST *lp_db_list)
{
	if(lp_db_list->head)
	{
		PACKET_NODE node = lp_db_list->head;
		while(node)
		{
			if(!node->packet.using)
			{
				PACKET_NODE result = node;
				if(node->prev) node->prev->next = node->next;
				if(node->next) node->next->prev = node->prev;

				/* 如果是取走的头节点或尾节点，则头节点或尾节点指针需要移动*/
				node = node->next;
				if(lp_db_list->head == result) lp_db_list->head = node;
				if(lp_db_list->tail == result) lp_db_list->tail = node;

				lp_db_list->ring_size--;
				return result;
			}

			node = node->next;
		}
	}
	return NULL;
}

CCCPACKET *interval_get_packet_from_packet_list()
{
	int b_get_node = 0;
	PACKET_NODE node = interval_get_packet_list_idle_head(&g_packet_list_idle);
	if(node)
	{
		b_get_node = 1;
	}

	/* 检查正在使用列表 */
	if(!b_get_node)
	{
		PACKET_NODE temp_node;
		while((temp_node = interval_get_packet_list_using_not_using(&g_packet_list_using)))
		{
			internal_insert_packet_node(&g_packet_list_idle, temp_node);
		}

		if((node = interval_get_packet_list_idle_head(&g_packet_list_idle)))
		{
			b_get_node = 1;
		}
	}

	/* 创建新节点 */
	if(!b_get_node)
	{
		if(interval_packet_list_add_node(&g_packet_list_idle))
		{
			if((node = interval_get_packet_list_idle_head(&g_packet_list_idle)))
			{
				b_get_node = 1;
			}
		}
	}

	if(b_get_node)
	{
		cccpacket_using(&node->packet);
		internal_insert_packet_node(&g_packet_list_using, node);
		return &node->packet;
	}
	return NULL;
}

/* TODO: interval_packet_pool_release */
void interval_packet_pool_release()
{

}

CCCPACKET *get_packet_from_packet_pool()
{
	pthread_mutex_lock(&g_packet_list_mutex);
	CCCPACKET *result = interval_get_packet_from_packet_list();
	pthread_mutex_unlock(&g_packet_list_mutex);

	if(!result) printf("error !!!  get_packet_from_packet_pool NULL\n");
	return result;
}

void packet_pool_release()
{
	pthread_mutex_lock(&g_packet_list_mutex);
	interval_packet_pool_release();
	pthread_mutex_unlock(&g_packet_list_mutex);

}
int packet_pool_init()
{
	return 1;
}
