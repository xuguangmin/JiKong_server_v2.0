/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : download_file.c
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-11-09
  ����޸�   :
  ��������   : һ�����������ļ��б�Ķ���
             �ڶ��е�ͷ�����ӽڵ㣬�����ӵĽڵ���������ǰ��
             ÿһ���ڵ��Ӧһ���ͻ��ˣ��ڵ㱣��ĵ�ǰ�����ϴ����ļ�����Ϣ
             ��ΪЭ�������ع��̵�Ӧ����δ�ṩ�ļ�������Ϣ������һ���ͻ���
             ͬһ��ʱ��ֻ������һ���ļ�

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "FLXCommon/flxnettypes.h"

typedef struct download_file_node *DOWNLOAD_FILE_NODE;
struct download_file_node
{
	FLXSocket          sock;				   // �ͻ����׽���
	unsigned char      protocol_file_type;     // δʹ�ã���ǰ���ڴ�����ļ�����
	FILE              *file_handle;			   // ����ڵ�ǰsock����Ӧ��Ҫ������ļ�
	char              *filename;               // �ͻ����ύ���ļ���

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
 * ɾ��һ���ڵ�
 * ����ֵ��
 *      1 �ýڵ����
 *      0 �ýڵ㲻����
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

			/* �����ɾ����ͷ�ڵ㣬��ͷ�ڵ�ָ����Ҫ�ƶ�*/
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
 * �Ӷ���ͷɾ��һ���ڵ�
 *      sock ����ڵ��sockֵ
 * ����ֵ��
 *      1 �ýڵ����
 *      0 �ýڵ㲻����
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
/* ����sock�����ڵ� */
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

/* ��һ��������Ҫ g_download_file_node_head */
static int interval_add_download_file_node(FLXSocket sock, FILE *file_handle, const char *filename)
{
	int len;
	DOWNLOAD_FILE_NODE lp_node;
	if(!filename)
		return 0;

	while(1)
	{
		/*
		 * �������쳣����²��У������Ľ��������ǲ�Ӧ�÷�����
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
 * Ŀǰ��ֻ�Զ��н����˼���
 * δ��ÿ���ڵ���м���
 * ��ΪĿǰ������ǣ�ÿ��sock��Ӧһ���̣߳�
 * Ҳ����ÿ���ڵ����Ϣ������һ���߳��д���ģ��ڵ㱾���漰���̵߳�ͬ������
 * �Ժ����ͬһ���ڵ�����ڶ���߳��б����ʵĻ����ٶ�ÿ���ڵ���м���
 */
FILE *get_download_file_handle(FLXSocket sock)
{
	DOWNLOAD_FILE_NODE lp_node;
	lp_node = search_download_file_node(sock);
	if(NULL == lp_node)
		return NULL;

	return lp_node->file_handle;
}
