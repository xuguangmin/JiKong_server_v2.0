/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ���    ��curl_wrapper.c
  ������    �����Ӹ�
  ��������   ��2012-11-1
  ��������   : ��curl���һ����װ��Ŀǰֻ�õ��������ʵ�ֵ�httpЭ�飬����������
             ���������ָ��
             ���ⲿ����ÿ�󣬰Ѿ�̬�⣨.a���ļ����Ƶ�lib�ļ��У�Ȼ��Ѱ�װ���ͷ
             �ļ����Ƶ�������Դ�����src/includeĿ¼��
  �����б�   :
  �޸���ʷ   :

******************************************************************************/

#include <string.h>
#include <semaphore.h>

#include "FLXCommon/flxtypes.h"
#include "FLXCommon/flxthread.h"
#include "curl/curl.h"
#include "ring_list.h"

static RING_LIST  g_curl_send_list;
static sem_t      g_curl_send_sem;

static CURL *create_http_easy_curl(const char *url)
{
	if(url)
	{
		CURL *curl = curl_easy_init();
		if(curl != NULL)
		{
			curl_easy_setopt(curl, CURLOPT_URL,     url);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
			return curl;
		}
	}
	return NULL;
}

// "http://192.168.1.10/cgi-bin/aw_ptz?cmd=%23APC7FFF7FFF&res=1"
// "http://192.168.1.10/cgi-bin/aw_ptz?cmd=%23R05&res=1"
static int curl_wrapper_send_single(const char *url)
{
	CURL *curl;
	CURLcode curl_code;
	if(!url)
		return 0;

	curl = create_http_easy_curl(url);
	if(NULL == curl)
	{
		printf("create easy curl handle failed.\n");
		return 0;
	}

	curl_code = curl_easy_perform(curl);
	if(curl_code != CURLE_OK)
	{
		printf("curl_easy_perform failed: %s\n", curl_easy_strerror(curl_code));
	}

	curl_easy_cleanup(curl);

	return 1;
}

static int append_curl_url_to_send_list(const char *url)
{
	int result = ring_list_append_data(&g_curl_send_list, (unsigned char *)url, strlen(url), 0);
	if(result) sem_post(&g_curl_send_sem);

	return result;
}

static int get_curl_url_from_send_list(unsigned char *buffer, int buf_size)
{
	int user_data;
	sem_wait(&g_curl_send_sem);
	return ring_list_get_data(&g_curl_send_list, buffer, buf_size, &user_data);
}
/*
 * �����߳�
 *
 * �ӷ����б���ȡ����
 * ����curl
 */
void *thread_func_curl_send(void *param)
{
	int len;
	unsigned char buffer[256];

	while(1)
	{
		len = get_curl_url_from_send_list(buffer, 256);
		if(len > 0)
		{
			buffer[len] = '\0';
			curl_wrapper_send_single((char *)buffer);
		}
	}
	return NULL;
}

int curl_wrapper_send(const char *url)
{
	return append_curl_url_to_send_list(url);
}

int curl_wrapper_init()
{
	FLXThread pid;
	CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
	if(CURLE_OK != res)
	{
		printf("curl_global_init failed: %s\n", curl_easy_strerror(res));
		return 0;
	}

	ring_list_init(&g_curl_send_list);
	sem_init(&g_curl_send_sem, 0, 0);
	if(thread_create(&pid, NULL, (void *)thread_func_curl_send, NULL) != 0)
	{
		printf("%s : thread create error\n", __FUNCTION__);
		return 0;
	}

	return 1;
}

int curl_wrapper_close()
{
	curl_global_cleanup();
	ring_list_release(&g_curl_send_list);
	sem_destroy(&g_curl_send_sem);
	return 1;
}

/* "http://192.168.1.10/cgi-bin/aw_ptz?cmd=%23APC7FFF7FFF&res=1"
// "http://192.168.1.10/cgi-bin/aw_ptz?cmd=%23R05&res=1"
int curl_wrapper_send_back(const char *url)
{
	CURL *curl;
	CURLcode curl_code;
	if(!url)
		return 0;

	curl = create_http_easy_curl(url);
	if(NULL == curl)
	{
		printf("create easy curl handle failed.\n");
		return 0;
	}

	curl_code = curl_easy_perform(curl);
	if(curl_code != CURLE_OK)
	{
		printf("curl_easy_perform failed: %s\n", curl_easy_strerror(curl_code));
	}

	curl_easy_cleanup(curl);

	return 1;
}*/
