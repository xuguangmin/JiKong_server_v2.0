/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : curl_wrapper.c
  作者    : 贾延刚
  生成日期 : 2012-11-01

  版本    : 1.0
  功能描述 : 对libghttp库的一个封装。
            需要先在libghttp中编译该库。

  修改历史 :

******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>

#include "FLXCommon/flxtypes.h"
#include "FLXCommon/flxthread.h"
#include "ring_list.h"
#include "ghttp.h"

#define CURL_BUF_SIZE         1024

static RING_LIST  g_curl_send_list;
static sem_t      g_irda_send_sem;


static int append_curl_url_to_send_list(const unsigned char *url_buffer, int data_len)
{
	int result = ring_list_append_data(&g_curl_send_list, url_buffer, data_len, 0);
	return result;
}

static int get_curl_url_from_send_list(char *buffer, int buf_size)
{
	int serial_no;
	int len = ring_list_get_data(&g_curl_send_list, (unsigned char *)buffer, buf_size-1, &serial_no);
	if(len > 0)
	{
		buffer[len] = '\0';
		return 1;
	}
	return 0;
}

// char *uri = "http://192.168.1.4";//"http://localhost:8080/index.html";
static int internal_ghttp_wrapper_send(char *uri)
{
    ghttp_request *request = NULL;
    ghttp_status status;
    int error;


    request = ghttp_request_new();
    if(ghttp_set_uri(request, uri) == -1)
    	return 0;

    if(ghttp_set_type(request, ghttp_type_post) == -1)
    {
        printf("ghttp_set_type :%s\n", ghttp_get_error(request));
        return 0;
    }
    if(ghttp_set_sync(request, ghttp_async) == -1)
    {
    	printf("ghttp_set_sync :%s\n", ghttp_get_error(request));
    	return 0;
    }


    ghttp_prepare(request);
    status = ghttp_process(request);
    if(status == ghttp_error)
    {
    	printf("status == ghttp_error :%s\n", ghttp_get_error(request));
    	return 0;
    }
    printf("ghttp_process success\n");
    /* OK, done */
    error = ghttp_status_code(request);
    printf("Status code -> %d %s\n", error, ghttp_get_error(request));
    //buf = ghttp_get_body(request);
    //bytes_read = ghttp_get_body_len(request);
    //printf("http -- %s\n", buf);
    ghttp_request_destroy(request);
    return 1;
}


// char *uri = "http://192.168.1.4";//"http://localhost:8080/index.html";
static int internal_ghttp_wrapper_onvif_send(char *uri, char *body)
{
    ghttp_request *request = NULL;
    ghttp_status status;
    int error;

    request = ghttp_request_new();
    if(ghttp_set_uri(request, uri) == -1)
    	return 0;

    ghttp_set_header(request, "Content-Type", "application/soap+xml");
    if(ghttp_set_type(request, ghttp_type_post) == -1)
    {
        printf("ghttp_set_type :%s\n", ghttp_get_error(request));
        return 0;
    }
    if(ghttp_set_body(request, body, strlen(body)) == -1)
	{
	   printf("ghttp_set_body failed. %s\n", ghttp_get_error(request));
	   return 0;
	}

    if(ghttp_set_sync(request, ghttp_async) == -1)
    {
    	printf("ghttp_set_sync :%s\n", ghttp_get_error(request));
    	return 0;
    }


    ghttp_prepare(request);
    status = ghttp_process(request);
    if(status == ghttp_error)
    {
    	printf("status == ghttp_error :%s\n", ghttp_get_error(request));
    	return 0;
    }
    printf("ghttp_process success\n");
    /* OK, done */
    error = ghttp_status_code(request);
    printf("Status code -> %d %s\n", error, ghttp_get_error(request));
    //buf = ghttp_get_body(request);
    //bytes_read = ghttp_get_body_len(request);
    //printf("http -- %s\n", buf);
    ghttp_request_destroy(request);
    return 1;
}

/*
 * 发送线程
 *
 * 从发送列表中取数据和对应的串口编号
 * 发往串口
 */
int thread_func_curl_send(void *param)
{
	char buffer[CURL_BUF_SIZE];

	while(1)
	{
		printf("thread_func_curl_send --------\n");
		sem_wait(&g_irda_send_sem);
		if(get_curl_url_from_send_list(buffer, CURL_BUF_SIZE))
		{
			internal_ghttp_wrapper_send(buffer);
		}
	}
	return 0;
}

int curl_wrapper_send(const unsigned char *url_buffer, int data_len)
{
	int result;
	result = append_curl_url_to_send_list(url_buffer, data_len);
	if(result)
	{
		if(sem_post(&g_irda_send_sem) != 0)
		{
			printf("%s sem_post error, %d %s\n", __FUNCTION__, errno, strerror(errno));
		}
		else
		{
			printf("curl_wrapper_send success\n");
		}
	}
	else
	{
		printf("%s ring_list_append_data error\n", __FUNCTION__);
	}

	return result;
}

int ghttp_wrapper_onvif_send(const char *uri, const char *http_body)
{
	internal_ghttp_wrapper_onvif_send(uri, http_body);
	return 1;
}
int curl_wrapper_init()
{

	FLXThread pid;
	if(sem_init(&g_irda_send_sem, 0, 0) != 0)
	{
		printf("sem_init error. %s\n", strerror(errno));
		return 0;
	}

	ring_list_init(&g_curl_send_list);
	if(thread_create(&pid, NULL, (void *)thread_func_curl_send, NULL) != 0)
	{
		printf("%s : thread create error\n", __FUNCTION__);
		return 0;
	}

	return 1;
}

int curl_wrapper_close()
{
	//curl_glo
	ring_list_release(&g_curl_send_list);
	return 1;
}
