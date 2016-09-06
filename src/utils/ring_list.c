/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : ring_list.c
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-09-21
  最近修改   :
  功能描述   : 环形链表，可以用来保存一系列的unsigned char类型的缓存。该链表可以自
              动根据需要分配、删除内存
              使用时，先定义一个RING_LIST类型的变量，然后调用函数：ring_list_init
              以后就可以使用该变量了
  修改历史   :

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "ring_list.h"


#define RING_LIST_CHECK_POINT   128

static RING_LIST_NODE make_ring_list_node()
{
	RING_LIST_NODE lp_node = (RING_LIST_NODE)malloc(sizeof(struct ring_list_node));
	if(!lp_node)
		return NULL;

	lp_node->buf      = NULL;
	lp_node->buf_size = 0;
	lp_node->data_len = 0;
	lp_node->is_using = 0;
	lp_node->next     = NULL;
	return lp_node;
}

static void free_ring_list_node(RING_LIST_NODE lp_node)
{
	if(lp_node->buf_size > 0) free(lp_node->buf);
	free(lp_node);
}

static void insert_ring_list_node(RING_LIST *lp_ring_list, RING_LIST_NODE lp_node)
{
	if(lp_ring_list->ring_size <= 0)
	{
		lp_ring_list->head = lp_node;
		lp_ring_list->tail = lp_node;
		lp_node->next = lp_node;
	}
	else
	{
		lp_node->next            = lp_ring_list->tail->next;
		lp_ring_list->tail->next = lp_node;
	}
	lp_ring_list->ring_size++;
}

/*
 * 删掉指定数量的空闲节点
 */
static void internal_delete_idle_ring_list_node(RING_LIST *lp_ring_list, int size)
{
	int count = 0;
	int loop = lp_ring_list->ring_size;
	RING_LIST_NODE rl_node;

	while(loop > lp_ring_list->using_size)
	{
		if(count >= size)
			break;

		rl_node = lp_ring_list->tail->next;
		lp_ring_list->tail->next = rl_node->next;

		free_ring_list_node(rl_node);
		lp_ring_list->ring_size--;

		count++;
		loop--;
	}

	/*printf("internal_delete_idle_ring_list_node delete %d, left %d\n", count, lp_ring_list->ring_size);*/
}

/*
 * 如果使用的节点数量不到所有数量的一半，则删掉一半
 */
static void internal_check_idle_ring_list_node(RING_LIST *lp_ring_list)
{
	int half = lp_ring_list->ring_size/2 -1;

	if(lp_ring_list->using_size < half)
		internal_delete_idle_ring_list_node(lp_ring_list, half);
}
/*
 * 增加数据到环形链表，如果链表已满，则新建一个节点
 */
static int internal_append_data(RING_LIST *lp_ring_list, const unsigned char *buffer, int data_len, int user_data)
{
	RING_LIST_NODE idle_node;
	if(!lp_ring_list || !buffer || data_len <= 0)
		return 0;

	if(lp_ring_list->ring_size == lp_ring_list->using_size) /* 如果已满，新追加一个 */
	{
		idle_node = make_ring_list_node();
		if(!idle_node)
			return 0;

		insert_ring_list_node(lp_ring_list, idle_node);
	}
	idle_node = lp_ring_list->tail->next;

	/* 未分配，或者内存不足，则新分配 */
	if(idle_node->buf_size < data_len)
	{
		if(idle_node->buf_size > 0)
		{
			free(idle_node->buf);
			idle_node->buf = NULL;
		}

		idle_node->buf = (unsigned char *)malloc(sizeof(unsigned char) * data_len);
		if(!idle_node->buf)
		{
			idle_node->buf_size = 0;
			return 0;
		}
		idle_node->buf_size = data_len;
	}

	memcpy(idle_node->buf, buffer, data_len);
	idle_node->data_len  = data_len;
	idle_node->user_data = user_data;
	idle_node->is_using  = 1;

	lp_ring_list->tail = idle_node;
	lp_ring_list->using_size++;

	/* 检查空闲内存是否过大 */
	if(lp_ring_list->ring_size > lp_ring_list->check_point) internal_check_idle_ring_list_node(lp_ring_list);
	return 1;
}

/*
 * 从链表中取头节点中的数据
 *
 * 如果buffer的大小比记录中的数据长度小，则只取前边部分
 * 剩下的部分，继续做为一条记录存在
 */
static int internal_ring_list_get_data(RING_LIST *lp_ring_list, unsigned char *buffer, int size, int *user_data)
{
	int len = 0;
	if(!lp_ring_list || !buffer || size <= 0 ||
		lp_ring_list->ring_size <= 0 ||
		lp_ring_list->using_size <= 0)
		return 0;

	unsigned char *p = lp_ring_list->head->buf;
	len = lp_ring_list->head->data_len;

	if(len > size)
		len = size;

	memcpy(buffer, p, len);
	if(user_data) *user_data = lp_ring_list->head->user_data;

	/*
	 * 如果缓存小，没取走所有的数据
	 * 则移动后边的数据到前边，继续做为一条有效记录
	 */
	if(size < lp_ring_list->head->data_len)
	{
		lp_ring_list->head->data_len -= len;
		memmove(p, p +len, lp_ring_list->head->data_len);
	}
	else
	{
		lp_ring_list->head->is_using = 0;
		lp_ring_list->head = lp_ring_list->head->next;

		lp_ring_list->using_size -= 1;
	}

	return len;
}

/*
 * 删除头节点中指定数据的数据
 *
 * 如果buffer的大小比记录中的数据长度小，则只取前边部分
 * 剩下的部分，继续做为一条记录存在
 * 返回值：
 *      返回已删除数据的长度
 */
static int internal_ring_list_delete_data(RING_LIST *lp_ring_list, int data_len)
{
	if(!lp_ring_list || data_len <= 0 ||
		lp_ring_list->ring_size <= 0 ||
		lp_ring_list->using_size <= 0)
		return 0;

	/*
	 * 如果要删除的长度小于记录中数据的长度
	 * 则移动后边的数据到前边，继续做为一条有效记录
	 */
	if(data_len < lp_ring_list->head->data_len)
	{
		unsigned char *p = lp_ring_list->head->buf;
		lp_ring_list->head->data_len -= data_len;
		memmove(p, p +data_len, lp_ring_list->head->data_len);
	}
	else
	{
		data_len = lp_ring_list->head->data_len;
		lp_ring_list->head->is_using = 0;
		lp_ring_list->using_size -= 1;

		lp_ring_list->head = lp_ring_list->head->next;
	}
	return data_len;
}

/*
 * 从链表中复制头节点中的数据
 *
 * 如果buffer的大小比记录中的数据长度小，则只复制前边部分
 */
static int internal_ring_list_copy_data(RING_LIST *lp_ring_list, unsigned char *buffer, int size, int *user_data)
{
	int len = 0;
	if(!lp_ring_list || !buffer || size <= 0 ||
		lp_ring_list->ring_size <= 0 ||
		lp_ring_list->using_size <= 0)
		return 0;

	unsigned char *p = lp_ring_list->head->buf;
	len = lp_ring_list->head->data_len;

	if(len > size)
		len = size;

	memcpy(buffer, p, len);
	if(user_data) *user_data = lp_ring_list->head->user_data;

	return len;
}

int ring_list_copy_data(RING_LIST *lp_ring_list, unsigned char *buffer, int size, int *user_data)
{
	int result;
	pthread_mutex_lock(&lp_ring_list->ring_mutex);
	result = internal_ring_list_copy_data(lp_ring_list, buffer, size, user_data);
	pthread_mutex_unlock(&lp_ring_list->ring_mutex);
	return result;
}
int ring_list_delete_data(RING_LIST *lp_ring_list, int data_len)
{
	int result;
	pthread_mutex_lock(&lp_ring_list->ring_mutex);
	result = internal_ring_list_delete_data(lp_ring_list, data_len);
	pthread_mutex_unlock(&lp_ring_list->ring_mutex);
	return result;
}

int ring_list_append_data(RING_LIST *lp_ring_list, const unsigned char *buffer, int data_len, int user_data)
{
	int result;
	pthread_mutex_lock(&lp_ring_list->ring_mutex);
	result = internal_append_data(lp_ring_list, buffer, data_len, user_data);
	pthread_mutex_unlock(&lp_ring_list->ring_mutex);
	return result;
}

int ring_list_get_data(RING_LIST *lp_ring_list, unsigned char *buffer, int size, int *user_data)
{
	int result;
	pthread_mutex_lock(&lp_ring_list->ring_mutex);
	result = internal_ring_list_get_data(lp_ring_list, buffer, size, user_data);
	pthread_mutex_unlock(&lp_ring_list->ring_mutex);
	return result;
}

void ring_list_release(RING_LIST *lp_ring_list)
{
	RING_LIST_NODE lp_node;
	while(lp_ring_list->ring_size > 0)
	{
		lp_node = lp_ring_list->head;

		lp_ring_list->head = lp_ring_list->head->next;
		lp_ring_list->ring_size -= 1;

		free_ring_list_node(lp_node);
	}

	lp_ring_list->head       = 0;
	lp_ring_list->ring_size  = 0;
	lp_ring_list->using_size = 0;
	pthread_mutex_destroy(&lp_ring_list->ring_mutex);
}

void ring_list_check_point(RING_LIST *lp_ring_list, int check_point)
{
	if(!lp_ring_list)
		return;

	lp_ring_list->check_point   = check_point;
}
void ring_list_init(RING_LIST *lp_ring_list)
{
	if(!lp_ring_list)
		return;

	lp_ring_list->head        = 0;
	lp_ring_list->ring_size   = 0;
	lp_ring_list->using_size  = 0;
	lp_ring_list->check_point = 128;
	pthread_mutex_init(&lp_ring_list->ring_mutex, NULL);
}
