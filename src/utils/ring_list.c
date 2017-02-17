/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : ring_list.c
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-09-21
  ����޸�   :
  ��������   : ��������������������һϵ�е�unsigned char���͵Ļ��档�����������
              ��������Ҫ���䡢ɾ���ڴ�
              ʹ��ʱ���ȶ���һ��RING_LIST���͵ı�����Ȼ����ú�����ring_list_init
              �Ժ�Ϳ���ʹ�øñ�����
  �޸���ʷ   :

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
 * ɾ��ָ�������Ŀ��нڵ�
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
 * ���ʹ�õĽڵ�������������������һ�룬��ɾ��һ��
 */
static void internal_check_idle_ring_list_node(RING_LIST *lp_ring_list)
{
	int half = lp_ring_list->ring_size/2 -1;

	if(lp_ring_list->using_size < half)
		internal_delete_idle_ring_list_node(lp_ring_list, half);
}
/*
 * �������ݵ�������������������������½�һ���ڵ�
 */
static int internal_append_data(RING_LIST *lp_ring_list, const unsigned char *buffer, int data_len, int user_data)
{
	RING_LIST_NODE idle_node;
	if(!lp_ring_list || !buffer || data_len <= 0)
		return 0;

	if(lp_ring_list->ring_size == lp_ring_list->using_size) /* �����������׷��һ�� */
	{
		idle_node = make_ring_list_node();
		if(!idle_node)
			return 0;

		insert_ring_list_node(lp_ring_list, idle_node);
	}
	idle_node = lp_ring_list->tail->next;

	/* δ���䣬�����ڴ治�㣬���·��� */
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

	/* �������ڴ��Ƿ���� */
	if(lp_ring_list->ring_size > lp_ring_list->check_point) internal_check_idle_ring_list_node(lp_ring_list);
	return 1;
}

/*
 * ��������ȡͷ�ڵ��е�����
 *
 * ���buffer�Ĵ�С�ȼ�¼�е����ݳ���С����ֻȡǰ�߲���
 * ʣ�µĲ��֣�������Ϊһ����¼����
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
	 * �������С��ûȡ�����е�����
	 * ���ƶ���ߵ����ݵ�ǰ�ߣ�������Ϊһ����Ч��¼
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
 * ɾ��ͷ�ڵ���ָ�����ݵ�����
 *
 * ���buffer�Ĵ�С�ȼ�¼�е����ݳ���С����ֻȡǰ�߲���
 * ʣ�µĲ��֣�������Ϊһ����¼����
 * ����ֵ��
 *      ������ɾ�����ݵĳ���
 */
static int internal_ring_list_delete_data(RING_LIST *lp_ring_list, int data_len)
{
	if(!lp_ring_list || data_len <= 0 ||
		lp_ring_list->ring_size <= 0 ||
		lp_ring_list->using_size <= 0)
		return 0;

	/*
	 * ���Ҫɾ���ĳ���С�ڼ�¼�����ݵĳ���
	 * ���ƶ���ߵ����ݵ�ǰ�ߣ�������Ϊһ����Ч��¼
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
 * �������и���ͷ�ڵ��е�����
 *
 * ���buffer�Ĵ�С�ȼ�¼�е����ݳ���С����ֻ����ǰ�߲���
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
