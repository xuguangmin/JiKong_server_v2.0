/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : upload_file.c
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-10-26
  最近修改   :
  功能描述   : 一个保存上传文件列表的队列
             在队列的头部增加节点，新增加的节点总是在最前边
             每一个节点对应一个客户端，这个客户端本次上传的所有
             文件信息，保存在每个节点内部的一个队列中，节点保存的
             是当前正在上传的文件的信息

  函数列表   :
  修改历史   :

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "upload_file.h"
#include "util_log.h"

typedef struct upload_file_node *UPLOAD_FILE_NODE;
struct upload_file_node
{
	FLXSocket         sock;					   //客户端套接字
	unsigned char     protocol_file_type;      //当前正在传输的文件类型
	FILE             *file_handle;			   //针对于当前sock所对应的要处理的文件
	char             *filename;                //客户端提交的文件名
	char             *filename_tx;             //传输时使用的文件名
	UTIL_QUEUE        file_queue;              //保存本次上传任务中，所有传输过的文件

	UPLOAD_FILE_NODE  prev;
	UPLOAD_FILE_NODE  next;
};

static UPLOAD_FILE_NODE  g_upload_file_node_head = NULL;
static pthread_mutex_t   g_file_tx_mutex = PTHREAD_MUTEX_INITIALIZER;


static UPLOAD_FILE_NODE make_upload_file_node()
{
	UPLOAD_FILE_NODE lp_node = (UPLOAD_FILE_NODE)malloc(sizeof(struct upload_file_node));
	if(!lp_node)
		return NULL;

	lp_node->sock           = INVALID_SOCKET;
	lp_node->file_handle    = NULL;
	lp_node->filename       = NULL;
	lp_node->filename_tx    = NULL;
	util_queue_init(&lp_node->file_queue);

	lp_node->prev           = NULL;
	lp_node->next           = NULL;
	return lp_node;
}

static void free_upload_file_node(UPLOAD_FILE_NODE lp_node)
{
	void *data;
	int user_data;
	while(util_queue_get_head_data(&lp_node->file_queue, &data, &user_data))
	{
		struct ul_file_info *lp_tx_file_info = (struct ul_file_info *)data;
		if(lp_tx_file_info)
		{
			if(lp_tx_file_info->filename)    free(lp_tx_file_info->filename);
			if(lp_tx_file_info->filename_tx) free(lp_tx_file_info->filename_tx);
			free(lp_tx_file_info);
		}
	}
	free(lp_node);
}

static void insert_upload_file_node(UPLOAD_FILE_NODE *head, UPLOAD_FILE_NODE lp_node)
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

static void interval_delete_upload_file_node(UPLOAD_FILE_NODE *head, FLXSocket sock)
{
	UPLOAD_FILE_NODE node;
	if(!(*head))
		return;

	node = *head;
	while(node)
	{
		if(node->sock == sock)
		{
			UPLOAD_FILE_NODE temp_node;
			if(node->prev) node->prev->next = node->next;
			if(node->next) node->next->prev = node->prev;

			temp_node = node;
			node = node->next;

			/* 如果是删除的头节点，则头节点指针需要移动*/
			if(*head == temp_node) *head = node;
			free_upload_file_node(temp_node);
			continue;
		}

		node = node->next;
	}
}
/* 根据sock搜索节点 */
static UPLOAD_FILE_NODE interval_search_upload_file_node(UPLOAD_FILE_NODE *head, FLXSocket sock)
{
	UPLOAD_FILE_NODE node;
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

static void interval_clear_preview_upload_info(UPLOAD_FILE_NODE lp_node)
{
	if(NULL == lp_node)
		return;

	if(lp_node->file_handle) fclose(lp_node->file_handle);
	lp_node->file_handle = NULL;
	lp_node->filename = NULL;
	lp_node->filename_tx = NULL;
}

/* 保存当前传输文件的信息到一个队列中 */
static int save_upload_history(UTIL_QUEUE *file_queue, unsigned char protocol_file_type, char *filename, char *filename_tx)
{
	struct ul_file_info *lp_ul_file_info;
	if(!file_queue)
		return 0;

	lp_ul_file_info = (struct ul_file_info *)malloc(sizeof(struct ul_file_info));
	if(!lp_ul_file_info)
		return 0;
	lp_ul_file_info->filename           = filename;
	lp_ul_file_info->filename_tx        = filename_tx;
	lp_ul_file_info->protocol_file_type = protocol_file_type;

	if(!util_queue_append_data(file_queue, (void *)lp_ul_file_info, 0))
		return 0;

	return 1;
}

static char *save_upload_filename(const char *filename)
{
	int len;
	char *filename_0;
	if(!filename || strlen(filename) <= 0)
		return NULL;

	len = strlen(filename);
	filename_0 = (char *)malloc(len + 1);
	if(!filename_0)
		return 0;

	strcpy(filename_0, filename);
	filename_0[len] = '\0';
	return filename_0;
}
static char *produce_tx_filename(const char *filename, FLXSocket sock, unsigned char protocol_file_type)
{
	int len;
	char *filename_tx;
	if(!filename || strlen(filename) <= 0)
		return NULL;

	len = strlen(filename) + 256;
	filename_tx = (char *)malloc(len);
	if(!filename_tx)
		return NULL;

	sprintf(filename_tx, "%s_%d_%d", filename, (int)sock, (int)protocol_file_type);
	return filename_tx;
}

static FILE *open_tx_filename(const char *filename)
{
	FILE *pfile = fopen(filename, "w+");
	if (!pfile)
	{
		CCC_LOG_OUT("create file %s error : %s\n", filename, strerror(errno));
	}
	else{
		CCC_LOG_OUT("create file success : %s\n", filename);
	}
	return pfile;
}
static int interval_add_upload_file_node(UPLOAD_FILE_NODE *head, FLXSocket sock, unsigned char protocol_file_type, const char *filename)
{
	int b_new_node = 0;
	UPLOAD_FILE_NODE lp_node;
	if(!filename)
		return 0;

	lp_node = interval_search_upload_file_node(head, sock);
	if(NULL == lp_node)
	{
		lp_node = make_upload_file_node();
		b_new_node = 1;
	}

	if(NULL == lp_node)
		return 0;

	if(!b_new_node)
	{
		/*
		 * 正常的交互情况下是不会出现这种情况的
		 * 如果客户端不按协议流程交互，则会出现
		 */
		interval_clear_preview_upload_info(lp_node);
	}

	lp_node->sock = sock;
	lp_node->protocol_file_type = protocol_file_type;

	/* origin filename */
	lp_node->filename = save_upload_filename(filename);
	if(!lp_node->filename)
	{
		free_upload_file_node(lp_node);
		return 0;
	}

	/* temp filename for tx */
	lp_node->filename_tx = produce_tx_filename(filename, sock, protocol_file_type);
	if(!lp_node->filename_tx)
	{
		free_upload_file_node(lp_node);
		return 0;
	}

	lp_node->file_handle = open_tx_filename(lp_node->filename_tx);
	if(!lp_node->file_handle)
	{
		free_upload_file_node(lp_node);
		return 0;
	}

	if(!save_upload_history(&lp_node->file_queue, lp_node->protocol_file_type, lp_node->filename, lp_node->filename_tx))
	{
		free_upload_file_node(lp_node);
		return 0;
	}

	if(b_new_node) insert_upload_file_node(head, lp_node);
	printf("socket %d upload file :%s\n", (int)lp_node->sock, lp_node->filename);
	return 1;
}


static int interval_clear_single_upload(UPLOAD_FILE_NODE *head, FLXSocket sock)
{
	UPLOAD_FILE_NODE lp_node = interval_search_upload_file_node(head, sock);
	if(NULL == lp_node)
		return 0;

	printf("socket %d upload file %s finished.\n", (int)lp_node->sock, lp_node->filename);
	interval_clear_preview_upload_info(lp_node);
	return 1;
}

UPLOAD_FILE_NODE search_upload_file_node(FLXSocket sock)
{
	UPLOAD_FILE_NODE lp_node;
	pthread_mutex_lock(&g_file_tx_mutex);
	lp_node = interval_search_upload_file_node(&g_upload_file_node_head, sock);
	pthread_mutex_unlock(&g_file_tx_mutex);
	return lp_node;
}

void delete_upload_file_node(FLXSocket sock)
{
	pthread_mutex_lock(&g_file_tx_mutex);
	interval_delete_upload_file_node(&g_upload_file_node_head, sock);
	pthread_mutex_unlock(&g_file_tx_mutex);
}

int add_upload_file_node(FLXSocket sock, unsigned char protocol_file_type, const char *filename)
{
	int result;
	pthread_mutex_lock(&g_file_tx_mutex);
	result = interval_add_upload_file_node(&g_upload_file_node_head, sock, protocol_file_type, filename);
	pthread_mutex_unlock(&g_file_tx_mutex);
	return result;
}

/* 目前，只对队列进行了加锁
 * 未对每个节点进行加锁
 * 因为目前的情况是，每个sock对应一个线程，
 * 也就是每个节点的信息都是在一个线程中处理的，节点本身不涉及到线程的同步问题
 * 以后，如果同一个节点可能在多个线程中被访问的话，再对每个节点进行加锁
 */
FILE *get_upload_file_handle(FLXSocket sock)
{
	UPLOAD_FILE_NODE lp_node;
	lp_node = search_upload_file_node(sock);
	if(NULL == lp_node)
		return NULL;

	return lp_node->file_handle;
}

/*
 * 单个文件上传完成，清除一些信息
 */
int clear_single_upload(FLXSocket sock)
{
	int result;
	pthread_mutex_lock(&g_file_tx_mutex);
	result = interval_clear_single_upload(&g_upload_file_node_head, sock);
	pthread_mutex_unlock(&g_file_tx_mutex);
	return result;
}

const char *get_upload_filename_tx(FLXSocket sock)
{
	UPLOAD_FILE_NODE lp_node;
	lp_node = search_upload_file_node(sock);
	if(NULL == lp_node)
		return NULL;

	return lp_node->filename_tx;
}

UTIL_QUEUE *get_upload_file_history(FLXSocket sock)
{
	UPLOAD_FILE_NODE lp_node;
	lp_node = search_upload_file_node(sock);
	if(lp_node)
	{
		return &lp_node->file_queue;
	}
	return NULL;
}

