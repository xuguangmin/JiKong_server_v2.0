/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : util_list.c
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-11-08
  ����޸�   :
  ��������   : һ��ͨ�õ��б����ݳ�Աֻ��һ��voidָ�롣
             ���б��ͷ�����ӽڵ㣬�����ӵĽڵ���������ǰ��

  �����б�   :
  �޸���ʷ   :

******************************************************************************/

#include <stdlib.h>
#include "util_list.h"


static LIST_NODE        g_list_node_head  = NULL;
static pthread_mutex_t  g_list_node_mutex = PTHREAD_MUTEX_INITIALIZER;


static LIST_NODE make_list_node()
{
	LIST_NODE lp_node = (LIST_NODE)malloc(sizeof(struct list_node));
	if(!lp_node)
		return NULL;

	lp_node->data  = 0;
	lp_node->prev  = NULL;
	lp_node->next  = NULL;
	return lp_node;
}

static void free_list_node(LIST_NODE lp_node)
{
	free(lp_node);
}

static void insert_list_node(UTIL_LIST *lp_util_list, LIST_NODE lp_node)
{
	if(lp_util_list->head == NULL)
	{
		lp_node->prev = NULL;
		lp_node->next = NULL;
		lp_util_list->head = lp_node;
	}
	else
	{
		lp_node->next            = lp_util_list->head;
		lp_util_list->head->prev = lp_node;

		lp_util_list->head = lp_node;
		lp_node->prev      = NULL;
	}
}

static void interval_delete_list_node(UTIL_LIST *lp_util_list, void *data)
{
	LIST_NODE node;
	if(!lp_util_list || !lp_util_list->head)
		return;

	node = lp_util_list->head;
	while(node)
	{
		if(node->data == data)
		{
			LIST_NODE temp_node;
			if(node->prev) node->prev->next = node->next;
			if(node->next) node->next->prev = node->prev;

			temp_node = node;
			node = node->next;

			/* �����ɾ����ͷ�ڵ㣬��ͷ�ڵ�ָ����Ҫ�ƶ�*/
			if(lp_util_list->head == temp_node) lp_util_list->head = node;

			free_list_node(temp_node);
			continue;
		}

		node = node->next;
	}
}
void interval_delete_all_list_node(LIST_NODE *head)
{
	if(*head)
	{
		LIST_NODE temp_node, node = *head;
		while(node)
		{
			temp_node = node;
			node = node->next;

			free_list_node(temp_node);
		}
		*head = NULL;
	}
}

/* ����data�����ڵ� */
static LIST_NODE interval_search_list_node(UTIL_LIST *lp_util_list, void *data)
{
	LIST_NODE node;
	if(!lp_util_list || !lp_util_list->head)
		return NULL;

	node = lp_util_list->head;
	while(node)
	{
		if(node->data == data)
		{
			return node;
		}

		node = node->next;
	}
	return NULL;
}

static int interval_add_list_node(UTIL_LIST *lp_util_list, void *data)
{
	int b_new_node = 0;
	LIST_NODE lp_node;
	if(!lp_util_list || !data)
		return 0;

	lp_node = interval_search_list_node(lp_util_list, data);
	if(NULL == lp_node)
	{
		lp_node = make_list_node();
		b_new_node = 1;
	}

	if(NULL == lp_node)
		return 0;

	if(b_new_node)
	{
		lp_node->data = data;
	}

	if(b_new_node) insert_list_node(lp_util_list, lp_node);
	return 1;
}

void delete_all_list_node()
{
	pthread_mutex_lock(&g_list_node_mutex);
	interval_delete_all_list_node(&g_list_node_head);
	pthread_mutex_unlock(&g_list_node_mutex);
}

void delete_list_node(UTIL_LIST *lp_util_list, void *data)
{
	pthread_mutex_lock(&lp_util_list->list_mutex);
	interval_delete_list_node(lp_util_list, data);
	pthread_mutex_unlock(&lp_util_list->list_mutex);
}

int add_list_node(UTIL_LIST *lp_util_list, void *data)
{
	int result;
	pthread_mutex_lock(&lp_util_list->list_mutex);
	result = interval_add_list_node(lp_util_list, data);
	pthread_mutex_unlock(&lp_util_list->list_mutex);
	return result;
}

