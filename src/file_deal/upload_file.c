/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : upload_file.c
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-10-26
  ����޸�   :
  ��������   : һ�������ϴ��ļ��б�Ķ���
             �ڶ��е�ͷ�����ӽڵ㣬�����ӵĽڵ���������ǰ��
             ÿһ���ڵ��Ӧһ���ͻ��ˣ�����ͻ��˱����ϴ�������
             �ļ���Ϣ��������ÿ���ڵ��ڲ���һ�������У��ڵ㱣���
             �ǵ�ǰ�����ϴ����ļ�����Ϣ

  �����б�   :
  �޸���ʷ   :

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
	FLXSocket         sock;					   //�ͻ����׽���
	unsigned char     protocol_file_type;      //��ǰ���ڴ�����ļ�����
	FILE             *file_handle;			   //����ڵ�ǰsock����Ӧ��Ҫ������ļ�
	char             *filename;                //�ͻ����ύ���ļ���
	char             *filename_tx;             //����ʱʹ�õ��ļ���
	UTIL_QUEUE        file_queue;              //���汾���ϴ������У����д�������ļ�

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

			/* �����ɾ����ͷ�ڵ㣬��ͷ�ڵ�ָ����Ҫ�ƶ�*/
			if(*head == temp_node) *head = node;
			free_upload_file_node(temp_node);
			continue;
		}

		node = node->next;
	}
}
/* ����sock�����ڵ� */
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

/* ���浱ǰ�����ļ�����Ϣ��һ�������� */
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
		 * �����Ľ���������ǲ���������������
		 * ����ͻ��˲���Э�����̽�����������
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

/* Ŀǰ��ֻ�Զ��н����˼���
 * δ��ÿ���ڵ���м���
 * ��ΪĿǰ������ǣ�ÿ��sock��Ӧһ���̣߳�
 * Ҳ����ÿ���ڵ����Ϣ������һ���߳��д���ģ��ڵ㱾���漰���̵߳�ͬ������
 * �Ժ����ͬһ���ڵ�����ڶ���߳��б����ʵĻ����ٶ�ÿ���ڵ���м���
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
 * �����ļ��ϴ���ɣ����һЩ��Ϣ
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

