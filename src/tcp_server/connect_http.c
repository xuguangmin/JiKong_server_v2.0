/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : connect_http.c
  作者    : 贾延刚
  生成日期 : 2013-11-21

  版本    : 1.0
  功能描述 :

  修改历史 :

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include "FLXCommon/flxtypes.h"
#include "FLXCommon/flxthread.h"
#include "ghttp_wrapper.h"
#include "util_queue.h"
#include "util_func.h"


#define HTTP_REQ_TYPE_URL           1
#define HTTP_REQ_TYPE_ONVIF         2


static UTIL_QUEUE   g_http_req_queue;
static sem_t        g_http_send_sem;

struct __HTTP_REQUEST
{
	int    type;
	char  *uri;
	char  *body;
};
typedef struct __HTTP_REQUEST HTTP_REQUEST;


static HTTP_REQUEST *internal_request_init(int type, const char *uri, const char *body)
{
	HTTP_REQUEST *http_req = (HTTP_REQUEST*)malloc(sizeof(struct __HTTP_REQUEST));
	if(!http_req)
		return NULL;

	memset(http_req, 0, sizeof(struct __HTTP_REQUEST));

	http_req->type = type;
	http_req->uri = util_strcpy(uri);
	if(!http_req->uri)
	{
		free(http_req);
		return NULL;
	}

	if(body)
	{
		http_req->body = util_strcpy(body);
		if(!http_req->body)
		{
			free(http_req);
			return NULL;
		}
	}
	return http_req;
}

static void internal_request_destory(HTTP_REQUEST *req)
{
	if(!req)
		return;

	if(req->uri)  free(req->uri);
	if(req->body) free(req->body);

	free(req);
}

static int internal_connect_http_send(int type, const char *uri, const char *http_body)
{
	HTTP_REQUEST *http_req = internal_request_init(type, uri, http_body);
	if(!http_req)
	{
		printf("%s internal_request_init error\n", __FUNCTION__);
		return 0;
	}

	if(!util_queue_append_data(&g_http_req_queue, http_req, 0))
	{
		printf("%s util_queue_append_data error\n", __FUNCTION__);
		return 0;
	}

	if(sem_post(&g_http_send_sem) != 0)
	{
		printf("%s sem_post error, %d %s\n", __FUNCTION__, errno, strerror(errno));
		return 0;
	}

	return 1;
}

/*
 * 发送线程
 *
 * 从发送列表中取数据和对应的串口编号
 * 发往串口
 */
int thread_func_http_send(void *param)
{
	int temp;
	HTTP_REQUEST *http_req;

	while(1)
	{
		sem_wait(&g_http_send_sem);
		if(util_queue_get_head_data(&g_http_req_queue, (void **)&http_req, &temp))
		{
			switch(http_req->type)
			{
			case HTTP_REQ_TYPE_URL:
				ghttp_wrapper_send_url(http_req->uri);
				break;
			case HTTP_REQ_TYPE_ONVIF:
				ghttp_wrapper_send_onvif(http_req->uri, http_req->body);
				break;
			}
			internal_request_destory(http_req);
		}
	}
	return 0;
}

int connect_http_send_url(const char *url)
{
	return internal_connect_http_send(HTTP_REQ_TYPE_URL, url, NULL);
}

int connect_http_send_onvif(const char *uri, const char *http_body)
{
	return internal_connect_http_send(HTTP_REQ_TYPE_ONVIF, uri, http_body);
}

int connect_http_init()
{
	FLXThread pid;
	if(sem_init(&g_http_send_sem, 0, 0) != 0)
	{
		printf("sem_init error. %s\n", strerror(errno));
		return 0;
	}

	util_queue_init(&g_http_req_queue);
	if(thread_create(&pid, NULL, (void *)thread_func_http_send, NULL) != 0)
	{
		printf("%s : thread_func_http_send error\n", __FUNCTION__);
		return 0;
	}

	return 1;
}

int connect_http_close()
{
	ghttp_wrapper_clear();
	util_queue_release(&g_http_req_queue);
	return 1;
}



