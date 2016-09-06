/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : download_file.c
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-11-09
  最近修改   :
  功能描述   : 一个保存下载文件列表的队列
             在队列的头部增加节点，新增加的节点总是在最前边
             每一个节点对应一个客户端，节点保存的当前正在上传的文件的信息
             因为协议在下载过程的应答中未提供文件类型信息，所以一个客户端
             同一个时候只能下载一个文件

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "FLXCommon/flxnettypes.h"

typedef struct download_file_node *DOWNLOAD_FILE_NODE;
struct download_file_node
{
	FLXSocket          sock;				   // 客户端套接字
	unsigned char      protocol_file_type;     // 未使用，当前正在传输的文件类型
	FILE              *file_handle;			   // 针对于当前sock所对应的要处理的文件
	char              *filename;               // 客户端提交的文件名

	DOWNLOAD_FILE_NODE prev;
	DOWNLOAD_FILE_NODE next;
};
static DOWNLOAD_FILE_NODE  g_download_file_node_head = NULL;
static pthread_mutex_t     g_file_dl_mutex = PTHREAD_MUTEX_INITIALIZER;

static DOWNLOAD_FILE_NODE make_download_file_node()
{
	DOWNLOAD_FILE_NODE lp_node = (DOWNLOAD_FILE_NODE)malloc(sizeof(struct download_file_node));
	if(!lp_node)
		return NULL;

	lp_node->sock           = INVALID_SOCKET;
	lp_node->file_handle    = NULL;
	lp_node->filename       = NULL;

	lp_node->prev           = NULL;
	lp_node->next           = NULL;
	return lp_node;
}

static void free_download_file_node(DOWNLOAD_FILE_NODE lp_node)
{
	if(lp_node->file_handle) fclose(lp_node->file_handle);
	if(lp_node->filename)    free(lp_node->filename);
	free(lp_node);
}

static void insert_download_file_node(DOWNLOAD_FILE_NODE *head, DOWNLOAD_FILE_NODE lp_node)
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
/*
 * 删除一个节点
 * 返回值：
 *      1 该节点存在
 *      0 该节点不存在
 */
static int interval_delete_download_file_node(DOWNLOAD_FILE_NODE *head, FLXSocket sock)
{
	DOWNLOAD_FILE_NODE node;
	if(!(*head))
		return 0;

	node = *head;
	while(node)
	{
		if(node->sock == sock)
		{
			DOWNLOAD_FILE_NODE temp_node;
			if(node->prev) node->prev->next = node->next;
			if(node->next) node->next->prev = node->prev;

			temp_node = node;
			node = node->next;

			/* 如果是删除的头节点，则头节点指针需要移动*/
			if(*head == temp_node) *head = node;

			printf("socket %d download file finished. %s\n", (int)temp_node->sock, temp_node->filename);
			free_download_file_node(temp_node);
			return 1;
		}

		node = node->next;
	}
	return 0;
}
/*
 * 从队列头删除一个节点
 *      sock 保存节点的sock值
 * 返回值：
 *      1 该节点存在
 *      0 该节点不存在
 */
static int interval_pop_download_file_node(DOWNLOAD_FILE_NODE *head, FLXSocket *sock, unsigned char *protocol_file_type)
{
	DOWNLOAD_FILE_NODE node;
	if(!(*head))
		return 0;

	node = *head;
	*head = node->next;
	if(*head) (*head)->prev = NULL;

	if(sock) *sock = node->sock;
	if(protocol_file_type) *protocol_file_type = node->protocol_file_type;
	free_download_file_node(node);
	return 1;
}
/* 根据sock搜索节点 */
static DOWNLOAD_FILE_NODE interval_search_download_file_node(DOWNLOAD_FILE_NODE *head, FLXSocket sock)
{
	DOWNLOAD_FILE_NODE node;
	if(!(*head))
		return NULL;

	node = *head;
	while(node)
	{
		if(node->sock == sock)
		{
			return node;
		}

		node = node->next;
	}
	return NULL;
}

/* 第一个参数需要 g_download_file_node_head */
static int interval_add_download_file_node(FLXSocket sock, FILE *file_handle, const char *filename)
{
	int len;
	DOWNLOAD_FILE_NODE lp_node;
	if(!filename)
		return 0;

	while(1)
	{
		/*
		 * 这属于异常情况下才有，正常的交互过程是不应该发生的
		 */
		lp_node = interval_search_download_file_node(&g_download_file_node_head, sock);
		if(!lp_node)
			break;

		printf("warning : redundant download file %s\n", lp_node->filename);
		interval_delete_download_file_node(&g_download_file_node_head, sock);
	}

	lp_node = make_download_file_node();
	if(NULL == lp_node)
		return 0;

	lp_node->sock        = sock;
	lp_node->file_handle = file_handle;
	//lp_node->protocol_file_type = protocol_file_type;

	/* filename */
	len = strlen(filename);
	lp_node->filename = (char *)malloc(len + 1);
	strcpy(lp_node->filename, filename);
	lp_node->filename[len] = '\0';

	insert_download_file_node(&g_download_file_node_head, lp_node);
	printf("socket %d download file :%s\n", (int)sock, filename);
	return 1;
}

DOWNLOAD_FILE_NODE search_download_file_node(FLXSocket sock)
{
	DOWNLOAD_FILE_NODE lp_node;
	pthread_mutex_lock(&g_file_dl_mutex);
	lp_node = interval_search_download_file_node(&g_download_file_node_head, sock);
	pthread_mutex_unlock(&g_file_dl_mutex);
	return lp_node;
}

int pop_download_file_node(FLXSocket *sock, unsigned char *protocol_file_type)
{
	int result;
	pthread_mutex_lock(&g_file_dl_mutex);
	result = interval_pop_download_file_node(&g_download_file_node_head, sock, protocol_file_type);
	pthread_mutex_unlock(&g_file_dl_mutex);
	return result;
}

int delete_download_file_node(FLXSocket sock)
{
	int result;
	pthread_mutex_lock(&g_file_dl_mutex);
	result = interval_delete_download_file_node(&g_download_file_node_head, sock);
	pthread_mutex_unlock(&g_file_dl_mutex);
	return result;
}

int add_download_file_node(FLXSocket sock, FILE *file_handle, const char *filename)
{
	int result;
	pthread_mutex_lock(&g_file_dl_mutex);
	result = interval_add_download_file_node(sock, file_handle, filename);
	pthread_mutex_unlock(&g_file_dl_mutex);
	return result;
}

/*
 * 目前，只对队列进行了加锁
 * 未对每个节点进行加锁
 * 因为目前的情况是，每个sock对应一个线程，
 * 也就是每个节点的信息都是在一个线程中处理的，节点本身不涉及到线程的同步问题
 * 以后，如果同一个节点可能在多个线程中被访问的话，再对每个节点进行加锁
 */
FILE *get_download_file_handle(FLXSocket sock)
{
	DOWNLOAD_FILE_NODE lp_node;
	lp_node = search_download_file_node(sock);
	if(NULL == lp_node)
		return NULL;

	return lp_node->file_handle;
}
