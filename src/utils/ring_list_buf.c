/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : ring_list_buf.c
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-09-21
  ����޸�   :
  ��������   : ��������
              ʹ��ʱ���ȶ���һ��RING_LIST���͵ı�����Ȼ����ú�����ring_list_init
              �Ժ�Ϳ���ʹ�øñ�����

  �����б�   :
  �޸���ʷ   :

******************************************************************************/

#include <stdlib.h>
#include "ring_list_buf.h"


#define RING_LIST_BUF_CHECK_POINT   256

static BUF_NODE make_node()
{
	BUF_NODE lp_node = (BUF_NODE)malloc(sizeof(struct buf_node));
	if(!lp_node)
		return NULL;

	lp_node->item = '\0';
	lp_node->next = NULL;
	return lp_node;
}

static void free_node(BUF_NODE lp_node)
{
	free(lp_node);
}

static void insert_node(RING_LIST_BUF *lp_ring_list, BUF_NODE lp_node)
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
 * ɾ��ָ�������Ŀ��нڵ�
 */
static void internal_delete_idle_node(RING_LIST_BUF *lp_ring_list, int size)
{
	int count = 0;
	int loop = lp_ring_list->ring_size;
	BUF_NODE lp_node;

	while(loop > lp_ring_list->using_size)
	{
		if(count >= size)
			break;

		lp_node = lp_ring_list->tail->next;
		lp_ring_list->tail->next = lp_node->next;

		free_node(lp_node);
		lp_ring_list->ring_size--;

		count++;
		loop--;
	}

	/*printf("internal_delete_idle_node delete %d, left %d\n", count, lp_ring_list->ring_size);*/
}

/*
 * ���ʹ�õĽڵ�������������������һ�룬��ɾ��һ��
 */
static void internal_check_idle_node(RING_LIST_BUF *lp_ring_list)
{
	int half = lp_ring_list->ring_size/2 -1;

	if(lp_ring_list->using_size < half)
		internal_delete_idle_node(lp_ring_list, half);
}

/*
 * �������ݵ�������������������������½�һ���ڵ�
 */
static int internal_buf_append(RING_LIST_BUF *lp_ring_list, unsigned char item)
{
	BUF_NODE lp_node;
	if(!lp_ring_list)
		return 0;

	if(lp_ring_list->ring_size == lp_ring_list->using_size)
	{
		lp_node = make_node();
		insert_node(lp_ring_list, lp_node);
	}

	lp_node = lp_ring_list->tail->next;
	lp_node->item = item;

	lp_ring_list->tail = lp_node;
	lp_ring_list->using_size++;

	/* �������ڴ��Ƿ���� */
	if(lp_ring_list->ring_size > RING_LIST_BUF_CHECK_POINT) internal_check_idle_node(lp_ring_list);
	return 1;
}

static int internal_get_buf(RING_LIST_BUF *lp_ring_list, unsigned char *buffer, int size)
{
	int ix = 0;
	if(!lp_ring_list || !buffer)
		return 0;

	while(lp_ring_list->using_size > 0 &&
		  ix < size)
	{
		buffer[ix++] = lp_ring_list->head->item;

		lp_ring_list->head = lp_ring_list->head->next;
		lp_ring_list->using_size -= 1;
	}

	return ix;
}

static int internal_get_buf_using_size(RING_LIST_BUF *lp_ring_list)
{
	if(!lp_ring_list) return 0;
	return lp_ring_list->using_size;
}

int ring_list_buf_data_length(RING_LIST_BUF *lp_ring_list)
{
	int result;
	pthread_mutex_lock(&lp_ring_list->ring_mutex);
	result = internal_get_buf_using_size(lp_ring_list);
	pthread_mutex_unlock(&lp_ring_list->ring_mutex);
	return result;
}

int ring_list_buf_append(RING_LIST_BUF *lp_ring_list, unsigned char item)
{
	int result;
	pthread_mutex_lock(&lp_ring_list->ring_mutex);
	result = internal_buf_append(lp_ring_list, item);
	pthread_mutex_unlock(&lp_ring_list->ring_mutex);
	return result;
}

int ring_list_buf_get_data(RING_LIST_BUF *lp_ring_list, unsigned char *buffer, int size)
{
	int result;
	pthread_mutex_lock(&lp_ring_list->ring_mutex);
	result = internal_get_buf(lp_ring_list, buffer, size);
	pthread_mutex_unlock(&lp_ring_list->ring_mutex);
	return result;
}
void ring_list_buf_release(RING_LIST_BUF *lp_ring_list)
{
	BUF_NODE lp_node;
	while(lp_ring_list->ring_size > 0)
	{
		lp_node = lp_ring_list->head;
		lp_ring_list->head = lp_ring_list->head->next;
		lp_ring_list->ring_size -= 1;

		free_node(lp_node);
	}
	lp_ring_list->head       = 0;
	lp_ring_list->tail       = 0;
	lp_ring_list->ring_size  = 0;
	lp_ring_list->using_size = 0;
	pthread_mutex_destroy(&lp_ring_list->ring_mutex);
}
void ring_list_buf_init(RING_LIST_BUF *lp_ring_list)
{
	if(!lp_ring_list)
		return;

	lp_ring_list->head       = 0;
	lp_ring_list->tail       = 0;
	lp_ring_list->ring_size  = 0;
	lp_ring_list->using_size = 0;
	pthread_mutex_init(&lp_ring_list->ring_mutex, NULL);
}
