/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : ccc_data_buffer.c
  ����    : ���Ӹ�
  �������� : 2012-11-22

  �汾    : 1.0
  �������� : �����շ����ݵ�����
           ������ͷ��β������ͷ�ڵ�ָ��Ľڵ㣬�������淢�͸�����ڵ������
           �ϲ��ȡ�������ݣ���ô�β����ʼȡ����Ϊ�µĽڵ��׷�ӵ�β��

  �޸���ʷ :

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "ccc_data_buffer.h"

typedef struct data_buffer_list
{
	DATA_BUFFER_NODE   head;
	DATA_BUFFER_NODE   tail;
	int                ring_size;
	int                using_size;
	pthread_mutex_t    dbl_mutex;
}DATA_BUFFER_LIST;

static DATA_BUFFER_LIST  g_data_buffer_list = {NULL, NULL, 0, 0, PTHREAD_MUTEX_INITIALIZER};


static DATA_BUFFER_NODE create_data_buffer_node()
{
	DATA_BUFFER_NODE lp_node = (DATA_BUFFER_NODE)malloc(sizeof(struct data_buffer_node));
	if(!lp_node)
		return NULL;

	lp_node->id               = -1;
	lp_node->connect_no       = -1;
	lp_node->b_connect_enable = 0;
	lp_node->type             = -1;

	lp_node->b_rb_get         = 0;
	recv_buffer_init(&lp_node->recv_buf);

	ring_list_init(&lp_node->send_buf);

	lp_node->prev = NULL;
	lp_node->next = NULL;
	return lp_node;
}

void free_data_buffer_node(DATA_BUFFER_NODE lp_node)
{
	recv_buffer_release(&lp_node->recv_buf);
	free(lp_node);
}

DATA_BUFFER_NODE create_data_buffer_node_for_telnet()
{
	DATA_BUFFER_NODE lp_node = create_data_buffer_node();
	if(lp_node)
	{
		lp_node->type = DATA_BUFFER_TYPE_CONN;
	}
	return lp_node;
}
DATA_BUFFER_NODE create_data_buffer_node_for_client()
{
	DATA_BUFFER_NODE lp_node = create_data_buffer_node();
	if(lp_node)
	{
		lp_node->type = DATA_BUFFER_TYPE_CLIENT;
	}
	return lp_node;
}

/*
 * �����ڵ�δʹ�õı��
 * �ڵ��Ŵ�1��ʼ��0��Ż�����һ������ڵ�
 * ����ֵ��
 *       0  ����
 *       >0 ��Ч���
 */
static int internal_search_data_buffer_node_idle_id(DATA_BUFFER_LIST *lp_db_list)
{
	int result;
	DATA_BUFFER_NODE node;
	if(!lp_db_list)
		return 0;

	result = 1;
	while(1)
	{
		int b_existed = 0;

		node = lp_db_list->head;
		while(node)
		{
			if(node->id == result)
			{
				b_existed = 1;
				break;
			}
			node = node->next;
		}

		if(!b_existed)
			break;

		result += 1;
	}
	return result;
}

/* �½ڵ�׷�ӵ�β�� */
static void internal_insert_data_buffer_node(DATA_BUFFER_LIST *lp_db_list, DATA_BUFFER_NODE lp_node)
{
	if(lp_db_list->ring_size <= 0)
	{
		lp_db_list->head = lp_node;
		lp_db_list->tail = lp_node;

		lp_node->prev = NULL;
		lp_node->next = NULL;
	}
	else
	{
		lp_node->next            = NULL;
		lp_db_list->tail->next = lp_node;
		lp_node->prev            = lp_db_list->tail;
		lp_db_list->tail       = lp_node;
	}
	lp_db_list->ring_size++;
}

/*
 * ɾ��ָ���Ľڵ�
 * ����ֵ��
 *      0 ����������ڵ㣬 ���򷵻�connect_no
 */
static int internal_delete_data_buffer_node(DATA_BUFFER_LIST *lp_db_list, int id)
{
	DATA_BUFFER_NODE node;
	if(!lp_db_list->head)
		return 0;

	node = lp_db_list->head;
	while(node)
	{
		if(node->id == id)
		{
			int connect_no;
			DATA_BUFFER_NODE temp_node;
			if(node->prev) node->prev->next = node->next;
			if(node->next) node->next->prev = node->prev;

			temp_node = node;
			node = node->next;

			/* �����ɾ����ͷ�ڵ��β�ڵ㣬��ͷ�ڵ��β�ڵ�ָ����Ҫ�ƶ�*/
			if(lp_db_list->head == temp_node) lp_db_list->head = node;
			if(lp_db_list->tail == temp_node) lp_db_list->tail = node;

			connect_no = temp_node->connect_no;
			free_data_buffer_node(temp_node);
			lp_db_list->ring_size--;
			lp_db_list->using_size--;

			printf("%s 222 id %d\n", __FUNCTION__, id);
			return connect_no;
		}

		node = node->next;
	}
	return 0;
}

/*
 * �õ�ָ���ڵ��б����connect_no
 * ����ֵ��
 *      ʧ�ܷ���0�� ���򷵻�connect_no
 */
static int internal_get_data_buffer_node_connect_no(DATA_BUFFER_LIST *lp_db_list, int id)
{
	DATA_BUFFER_NODE node = lp_db_list->head;
	while(node)
	{
		if(node->id == id) return node->connect_no;
		node = node->next;
	}
	return 0;
}

/*
 * ����ָ���ڵ��Ӧ�������Ƿ���Ч
 * ����ֵ��
 *      ʧ�ܷ���0�� ���򷵻�1
 *      ������ش���1���������������
 */
static int internal_data_buffer_node_set_connect_enable(DATA_BUFFER_LIST *lp_db_list, int connect_no, int b_enable)
{
	int result = 0;
	DATA_BUFFER_NODE node = lp_db_list->head;
	while(node)
	{
		if(node->connect_no == connect_no)
		{
			node->b_connect_enable = b_enable;
			result += 1;
		}
		node = node->next;
	}
	return result;
}

/*
 * ����ֵ��
 *      ��ȡ�����ݵĳ���
 */
static int internal_data_buffer_poll_recv_data(DATA_BUFFER_LIST *lp_db_list, unsigned char *buffer, int buf_size, int *id, int *type)
{
	int result;
	DATA_BUFFER_NODE  node;
	if(!lp_db_list || !buffer)
		return 0;
	if(!lp_db_list->tail)
		return 0;

	node = lp_db_list->tail;
	while(node)
	{
		switch(node->type)
		{
		case DATA_BUFFER_TYPE_CONN:
			{
				result = recv_buffer_get_data(&node->recv_buf, buffer, buf_size);
				if(result > 0)
				{
					if(id)   *id= node->id;
					if(type) *type = node->type;
					return result;
				}
			}
			break;

		case DATA_BUFFER_TYPE_CLIENT:
			if(!node->b_rb_get)
			{
				result = recv_buffer_copy_data(&node->recv_buf, buffer, buf_size);
				if(result > 0)
				{
					if(id)   *id= node->id;
					if(type) *type = node->type;

					node->b_rb_get = 1;
					return result;
				}
			}
			break;
		}
		node = node->prev;
	}

	return 0;
}

static int internal_data_buffer_save_send_data(DATA_BUFFER_LIST *lp_db_list, int id, const unsigned char *buffer, int data_len)
{
	DATA_BUFFER_NODE  node;
	if(!lp_db_list || !lp_db_list->head)
		return 0;

	node = lp_db_list->head;
	while(node)
	{
		if(id == node->id)
		{
			ring_list_append_data(&node->send_buf, buffer, data_len, 0);
			return 1;
		}
		node = node->next;
	}
	return 0;
}

/*
 * ��ѯ���п��Է��͵�����
 * �����������ݣ�
 * Ȼ�������Ӵ��ڿ���״̬
 * ����ֵ��
 *      �������0���򷵻ص����ݻᱣ����buffer�У�
 *      connect_no �����Ӧ�����ӱ��
 */
static int internal_data_buffer_poll_send_data(DATA_BUFFER_LIST *lp_db_list, unsigned char *buffer, int buf_size, int *connect_no)
{
	int result = 0;
	DATA_BUFFER_NODE  node;
	if(!lp_db_list || !buffer || !connect_no)
		return 0;

	node = lp_db_list->head;
	while(node)
	{
		if(node->b_connect_enable)
		{
			result = ring_list_copy_data(&node->send_buf, buffer, buf_size, NULL);
			if(result > 0)
			{
				*connect_no = node->connect_no;
				break;
			}
		}
		node = node->next;
	}

	return result;
}

/*
 * ɾ��ָ�����ӵ�ָ�����ȵ�����
 * ����ֵ��
 *      ������ɾ�����ݵĳ���
 */
static int internal_data_buffer_delete_send_data(DATA_BUFFER_LIST *lp_db_list, int connect_no, int data_len)
{
	DATA_BUFFER_NODE  node;
	if(!lp_db_list || data_len <= 0)
		return 0;

	node = lp_db_list->head;
	while(node)
	{
		if(node->connect_no == connect_no)
		{
			int len = ring_list_delete_data(&node->send_buf, data_len);
			printf("%s, connect_no %d deleted %d\n", __FUNCTION__, connect_no, len);
			return len;
		}
		node = node->next;
	}
	return 0;
}

static int internal_data_buffer_save_recv_data(DATA_BUFFER_NODE db_node, const unsigned char *buffer, int data_len)
{
	if(!db_node)
		return 0;

	db_node->b_rb_get = 0;
	return recv_buffer_save_data(&db_node->recv_buf, buffer, data_len);
}

int get_data_buffer_node_connect_no(int id)
{
	int result;
	pthread_mutex_lock(&g_data_buffer_list.dbl_mutex);
	result = internal_get_data_buffer_node_connect_no(&g_data_buffer_list, id);
	pthread_mutex_unlock(&g_data_buffer_list.dbl_mutex);
	return result;
}

int data_buffer_node_set_connect_enable(int connect_no, int b_enable)
{
	int result;
	pthread_mutex_lock(&g_data_buffer_list.dbl_mutex);
	result = internal_data_buffer_node_set_connect_enable(&g_data_buffer_list, connect_no, b_enable);
	pthread_mutex_unlock(&g_data_buffer_list.dbl_mutex);
	return result;

}
int data_buffer_poll_recv_data(unsigned char *buffer, int buf_size, int *id, int *type)
{
	int result;
	pthread_mutex_lock(&g_data_buffer_list.dbl_mutex);
	result = internal_data_buffer_poll_recv_data(&g_data_buffer_list, buffer, buf_size, id, type);
	pthread_mutex_unlock(&g_data_buffer_list.dbl_mutex);
	return result;
}

int data_buffer_poll_send_data(unsigned char *buffer, int buf_size, int *connect_no)
{
	int result;
	pthread_mutex_lock(&g_data_buffer_list.dbl_mutex);
	result = internal_data_buffer_poll_send_data(&g_data_buffer_list, buffer, buf_size, connect_no);
	pthread_mutex_unlock(&g_data_buffer_list.dbl_mutex);
	return result;
}
int data_buffer_delete_send_data(int connect_no, int data_len)
{
	int result;
	pthread_mutex_lock(&g_data_buffer_list.dbl_mutex);
	result = internal_data_buffer_delete_send_data(&g_data_buffer_list, connect_no, data_len);
	pthread_mutex_unlock(&g_data_buffer_list.dbl_mutex);
	return result;

}
int data_buffer_save_send_data(int id, const unsigned char *buffer, int data_len)
{
	int result;
	pthread_mutex_lock(&g_data_buffer_list.dbl_mutex);
	result = internal_data_buffer_save_send_data(&g_data_buffer_list, id, buffer, data_len);
	pthread_mutex_unlock(&g_data_buffer_list.dbl_mutex);
	return result;
}

int data_buffer_save_recv_data(DATA_BUFFER_NODE db_node, const unsigned char *buffer, int data_len)
{
	int result;
	pthread_mutex_lock(&g_data_buffer_list.dbl_mutex);
	result = internal_data_buffer_save_recv_data(db_node, buffer, data_len);
	pthread_mutex_unlock(&g_data_buffer_list.dbl_mutex);
	return result;
}

int delete_data_buffer_node(int id)
{
	int result;
	pthread_mutex_lock(&g_data_buffer_list.dbl_mutex);
	result = internal_delete_data_buffer_node(&g_data_buffer_list, id);
	pthread_mutex_unlock(&g_data_buffer_list.dbl_mutex);
	return result;
}

int insert_data_buffer_node(DATA_BUFFER_NODE db_node)
{
	pthread_mutex_lock(&g_data_buffer_list.dbl_mutex);
	db_node->id = internal_search_data_buffer_node_idle_id(&g_data_buffer_list);
	internal_insert_data_buffer_node(&g_data_buffer_list, db_node);
	pthread_mutex_unlock(&g_data_buffer_list.dbl_mutex);
	return db_node->id;
}
