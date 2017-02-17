/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : telnet_info.c
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-12-07
  ����޸�   :
  ��������   : һ������telnet������Ϣ�Ķ���
             �ڶ��е�ͷ�����ӽڵ㣬�����ӵĽڵ���������ǰ��


******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct telnet_info_node *TELNET_INFO_NODE;
struct telnet_info_node
{
	int               controlId;              // ���Զ�̬�����Ϣ
	char             *ip_addr;                // IP
	int               port;                   // telnet�˿�

	int               id;                     // �����²�ͨѶ���id

	TELNET_INFO_NODE  prev;
	TELNET_INFO_NODE  next;
};
static TELNET_INFO_NODE  g_telnet_info_node_head = NULL;
static pthread_mutex_t   g_telnet_info_mutex = PTHREAD_MUTEX_INITIALIZER;

static TELNET_INFO_NODE make_telnet_info_node()
{
	TELNET_INFO_NODE lp_node = (TELNET_INFO_NODE)malloc(sizeof(struct telnet_info_node));
	if(!lp_node)
		return NULL;

	lp_node->ip_addr        = NULL;
	lp_node->prev           = NULL;
	lp_node->next           = NULL;
	return lp_node;
}

static void free_telnet_info_node(TELNET_INFO_NODE lp_node)
{
	if(lp_node->ip_addr)    free(lp_node->ip_addr);
	free(lp_node);
}

static void insert_telnet_info_node(TELNET_INFO_NODE *head, TELNET_INFO_NODE lp_node)
{
	if(*head == NULL)
	{
		lp_node->prev = NULL;
		lp_node->next = NULL;
		*head = lp_node;
	}
	else
	{
		lp_node->next = *head;
		(*head)->prev = lp_node;

		*head = lp_node;
		lp_node->prev = NULL;
	}
}

/* ����sock�����ڵ� */
static TELNET_INFO_NODE interval_search_telnet_info_node(TELNET_INFO_NODE *head, int controlId)
{
	TELNET_INFO_NODE node;
	if(!(*head))
		return NULL;

	printf("%s, controlId %d\n", __FUNCTION__, controlId);

	node = *head;
	while(node)
	{
		if(node->controlId == controlId)
		{
			printf("%s, node->controlId %d\n", __FUNCTION__, node->controlId);
			return node;
		}

		node = node->next;
	}
	return NULL;
}

/*
 * ɾ��һ���ڵ�
 * ����ֵ��
 *      1 �ýڵ����
 *      0 �ýڵ㲻����
 */
static int interval_delete_telnet_info_node(TELNET_INFO_NODE *head, int controlId)
{
	TELNET_INFO_NODE node;
	if(!(*head))
		return 0;

	node = *head;
	while(node)
	{
		if(node->controlId == controlId)
		{
			TELNET_INFO_NODE temp_node;
			if(node->prev) node->prev->next = node->next;
			if(node->next) node->next->prev = node->prev;

			temp_node = node;
			node = node->next;

			/* �����ɾ����ͷ�ڵ㣬��ͷ�ڵ�ָ����Ҫ�ƶ�*/
			if(*head == temp_node) *head = node;
			free_telnet_info_node(temp_node);
			return 1;
		}

		node = node->next;
	}
	return 0;
}

/* ��һ��������Ҫ g_telnet_info_node_head */
static int interval_add_telnet_info_node(TELNET_INFO_NODE *head, int controlId, const char *ip_address, int port, int id)
{
	int len;
	TELNET_INFO_NODE lp_node;
	if(!ip_address)
		return 0;

	while(1)
	{
		/*
		 * �������쳣����²��У������Ľ��������ǲ�Ӧ�÷�����
		 */
		lp_node = interval_search_telnet_info_node(head, controlId);
		if(!lp_node)
			break;

		printf("warning : redundant telnet_info_node\n");
		interval_delete_telnet_info_node(head, controlId);
	}

	lp_node = make_telnet_info_node();
	if(NULL == lp_node)
		return 0;

	lp_node->controlId  = controlId;
	lp_node->port       = port;
	lp_node->id         = id;

	/* filename */
	len = strlen(ip_address);
	lp_node->ip_addr = (char *)malloc(len + 1);
	strcpy(lp_node->ip_addr, ip_address);
	lp_node->ip_addr[len] = '\0';

	insert_telnet_info_node(&g_telnet_info_node_head, lp_node);
	return 1;
}

TELNET_INFO_NODE search_telnet_info_node(int controlId)
{
	TELNET_INFO_NODE lp_node;
	pthread_mutex_lock(&g_telnet_info_mutex);
	lp_node = interval_search_telnet_info_node(&g_telnet_info_node_head, controlId);
	pthread_mutex_unlock(&g_telnet_info_mutex);
	return lp_node;
}

/*
 * Ŀǰ��ֻ�Զ��н����˼���
 * δ��ÿ���ڵ���м���
 * ��ΪĿǰ������ǣ�ÿ��sock��Ӧһ���̣߳�
 * Ҳ����ÿ���ڵ����Ϣ������һ���߳��д���ģ��ڵ㱾���漰���̵߳�ͬ������
 * �Ժ����ͬһ���ڵ�����ڶ���߳��б����ʵĻ����ٶ�ÿ���ڵ���м���
 */
int get_telnet_info_node_id(int controlId)
{
	TELNET_INFO_NODE lp_node = search_telnet_info_node(controlId);
	if(NULL == lp_node)
		return -1;

	return lp_node->id;
}

int delete_telnet_info_node(int controlId)
{
	int result;
	pthread_mutex_lock(&g_telnet_info_mutex);
	result = interval_delete_telnet_info_node(&g_telnet_info_node_head, controlId);
	pthread_mutex_unlock(&g_telnet_info_mutex);
	return result;
}

int add_telnet_info_node(int controlId, const char *ip_address, int port, int id)
{
	int result;
	pthread_mutex_lock(&g_telnet_info_mutex);
	result = interval_add_telnet_info_node(&g_telnet_info_node_head, controlId, ip_address, port, id);
	pthread_mutex_unlock(&g_telnet_info_mutex);
	return result;
}
