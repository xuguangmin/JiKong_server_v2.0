/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : util_queue.c
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-10-29
  ����޸�   :
  ��������   : һ��ͨ�õĶ��У����ݳ�Աֻ��һ��voidָ�롣
             ��β��׷�ӣ���ͷ��ȡ
             ʹ��ʱ���ȶ���һ��UTIL_QUEUE���͵ı�����Ȼ����ú�����util_queue_init��
             �Զ��г�ʼ���󣬾Ϳ���ʹ���ˡ�
             ���ʹ��util_queue_release����˶��У���Ҫ���³�ʼ���Ժ�ſ���ʹ��

  �����б�   :
  �޸���ʷ   :

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
 * �������׷������
 * ���н�����һ���ڵ㣬������������ݣ�
 * Ȼ��׷���½ڵ㵽���е�β����βָ���Ƶ���׷�ӵĽڵ���
 * ����ֵ��1 �ɹ���0 ʧ��
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
 * �Ӷ�����ȡͷ����
 * Ȼ��ɾ��ͷ�ڵ㣬ͷָ���Ƶ���һ���ڵ�
 * ����ֵ��1 ȡ�����ݣ�0 δȡ������
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
