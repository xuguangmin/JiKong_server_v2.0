/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : telnet_info.c
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-12-07
  最近修改   :
  功能描述   : 一个保存telnet连接信息的队列
             在队列的头部增加节点，新增加的节点总是在最前边


******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct telnet_info_node *TELNET_INFO_NODE;
struct telnet_info_node
{
	int               controlId;              // 来自动态库的信息
	char             *ip_addr;                // IP
	int               port;                   // telnet端口

	int               id;                     // 来自下层通讯层的id

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

/* 根据sock搜索节点 */
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
 * 删除一个节点
 * 返回值：
 *      1 该节点存在
 *      0 该节点不存在
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

			/* 如果是删除的头节点，则头节点指针需要移动*/
			if(*head == temp_node) *head = node;
			free_telnet_info_node(temp_node);
			return 1;
		}

		node = node->next;
	}
	return 0;
}

/* 第一个参数需要 g_telnet_info_node_head */
static int interval_add_telnet_info_node(TELNET_INFO_NODE *head, int controlId, const char *ip_address, int port, int id)
{
	int len;
	TELNET_INFO_NODE lp_node;
	if(!ip_address)
		return 0;

	while(1)
	{
		/*
		 * 这属于异常情况下才有，正常的交互过程是不应该发生的
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
 * 目前，只对队列进行了加锁
 * 未对每个节点进行加锁
 * 因为目前的情况是，每个sock对应一个线程，
 * 也就是每个节点的信息都是在一个线程中处理的，节点本身不涉及到线程的同步问题
 * 以后，如果同一个节点可能在多个线程中被访问的话，再对每个节点进行加锁
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
