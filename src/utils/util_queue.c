/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : util_queue.c
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-10-29
  最近修改   :
  功能描述   : 一个通用的队列，数据成员只有一个void指针。
             在尾部追加，从头部取
             使用时，先定义一个UTIL_QUEUE类型的变量，然后调用函数：util_queue_init，
             对队列初始化后，就可以使用了。
             如果使用util_queue_release清空了队列，需要重新初始化以后才可以使用

  函数列表   :
  修改历史   :

******************************************************************************/

#include <stdlib.h>
#include "util_queue.h"

static QUEUE_NODE make_queue_node()
{
	QUEUE_NODE lp_node = (QUEUE_NODE)malloc(sizeof(struct queue_node));
	if(!lp_node)
		return NULL;

	lp_node->data = 0;
	lp_node->next = NULL;
	return lp_node;
}

static void free_queue_node(QUEUE_NODE lp_node)
{
	free(lp_node);
}

static void insert_queue_node(UTIL_QUEUE *lp_util_queue, QUEUE_NODE lp_node)
{
	lp_node->next = NULL;
	if(!lp_util_queue->head)
	{
		lp_util_queue->head = lp_node;
		lp_util_queue->tail = lp_node;
	}
	else
	{
		lp_util_queue->tail->next = lp_node;
		lp_util_queue->tail       = lp_node;
	}
}

/*
 * 向队列中追加数据
 * 队列将创建一个节点，保存输入的数据，
 * 然后追加新节点到队列的尾部，尾指针移到新追加的节点上
 * 返回值：1 成功，0 失败
 */
static int internal_append_data(UTIL_QUEUE *lp_util_queue, void *data, int user_data)
{
	QUEUE_NODE idle_node;
	if(!lp_util_queue || !data)
		return 0;

	idle_node = make_queue_node();
	if(!idle_node)
		return 0;

	idle_node->data = data;
	//idle_node->user_data = user_data;
	insert_queue_node(lp_util_queue, idle_node);
	return 1;
}

/*
 * 从队列中取头数据
 * 然后删除头节点，头指针移到下一个节点
 * 返回值：1 取到数据，0 未取到数据
 */
static int internal_get_head_data(UTIL_QUEUE *lp_util_queue, void **data, int *user_data)
{
	QUEUE_NODE lp_node;
	if(!lp_util_queue || !lp_util_queue->head)
		return 0;

	lp_node = lp_util_queue->head;
	*data = lp_node->data;

	lp_util_queue->head = lp_util_queue->head->next;
	free_queue_node(lp_node);
	return 1;
}

void internal_util_queue_release(UTIL_QUEUE *lp_util_queue)
{
	if(lp_util_queue)
	{
		QUEUE_NODE tempnode;
		while(lp_util_queue->head)
		{
			tempnode = lp_util_queue->head;
			lp_util_queue->head = lp_util_queue->head->next;

			free_queue_node(tempnode);
		}
		lp_util_queue->head = 0;
		lp_util_queue->tail = 0;
	}
}

int util_queue_append_data(UTIL_QUEUE *lp_util_queue, void *data, int user_data)
{
	int result;
	pthread_mutex_lock(&lp_util_queue->queue_mutex);
	result = internal_append_data(lp_util_queue, data, user_data);
	pthread_mutex_unlock(&lp_util_queue->queue_mutex);
	return result;
}

int util_queue_get_head_data(UTIL_QUEUE *lp_util_queue, void **data, int *user_data)
{
	int result;
	pthread_mutex_lock(&lp_util_queue->queue_mutex);
	result = internal_get_head_data(lp_util_queue, data, user_data);
	pthread_mutex_unlock(&lp_util_queue->queue_mutex);
	return result;
}

void util_queue_release(UTIL_QUEUE *lp_util_queue)
{
	pthread_mutex_lock(&lp_util_queue->queue_mutex);
	internal_util_queue_release(lp_util_queue);
	pthread_mutex_unlock(&lp_util_queue->queue_mutex);

	pthread_mutex_destroy(&lp_util_queue->queue_mutex);
}
void util_queue_init(UTIL_QUEUE *lp_util_queue)
{
	if(!lp_util_queue)
		return;

	lp_util_queue->head  = 0;
	lp_util_queue->tail  = 0;
	pthread_mutex_init(&lp_util_queue->queue_mutex, NULL);
}
