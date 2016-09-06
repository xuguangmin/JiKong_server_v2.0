/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : ghttp_wrapper.c
  作者    : 贾延刚
  生成日期 : 2013-11-21

  版本    : 1.0
  功能描述 : 对libghttp库的一个封装。
            实现发送url地址、发送onvif协议包

  修改历史 :
           需要先在libghttp中编译该库

******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ghttp.h"

static ghttp_request *g_ghttp_request = NULL;


/*
	"http://192.168.1.10/cgi-bin/aw_ptz?cmd=%23APC7FFF7FFF&res=1"
	"http://192.168.1.10/cgi-bin/aw_ptz?cmd=%23R05&res=1"
 */
static int internal_ghttp_wrapper_send_url(ghttp_request *request, char *url)
{
	//int error;
	ghttp_status status;
    if(ghttp_set_uri(request, url) == -1)
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

    for(;;)
	{
		status = ghttp_process(request);
		if(status == ghttp_error)
		{
			ghttp_close(request);
			printf("status == ghttp_error :%s\n", ghttp_get_error(request));
			return 0;
		}

		printf("ghttp status code -> %d\n", status);
		if(status == ghttp_done)
			break;
	}

    //printf("ghttp_process success\n");

    /* OK, done */
    //error = ghttp_status_code(request);
    //printf("Status code -> %d %s\n", error, ghttp_get_error(request));
	return 1;
}

int ghttp_wrapper_send_url(char *url)
{
	int result = 0;
	ghttp_request *request;
	if(!url || strlen(url) <= 0)
		return 0;

	request = ghttp_request_new();
	if(NULL == request)
		return 0;

	result = internal_ghttp_wrapper_send_url(request, url);
	ghttp_close(request);
	ghttp_request_destroy(request);
	return result;
}

/*
int ghttp_wrapper_send_url(char *url)
{
	if(!url || strlen(url) <= 0)
		return 0;

	if(NULL == g_ghttp_request) g_ghttp_request = ghttp_request_new();
	if(NULL == g_ghttp_request)
		return 0;

	ghttp_clean(g_ghttp_request);
	return internal_ghttp_wrapper_send_url(g_ghttp_request, url);
}
 */
static int internal_ghttp_wrapper_send_onvif(ghttp_request *request, char *uri, char *body)
{
    ghttp_status status;
    if(ghttp_set_uri(request, uri) == -1)
    	return 0;

    ghttp_set_header(request, http_hdr_Content_Type, "application/soap+xml");
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

    for(;;)
    {
		status = ghttp_process(request);
		if(status == ghttp_error)
		{
			printf("status == ghttp_error :%s\n", ghttp_get_error(request));
			return 0;
		}

		printf("ghttp status code -> %d\n", status);
		if(status == ghttp_done)
			break;
    }
    //printf("ghttp_process success\n");

    /* OK, done */
    //error = ghttp_status_code(request);
    //printf("Status code -> %d %s\n", error, ghttp_get_error(request));
    return 1;
}

int ghttp_wrapper_send_onvif(char *uri, char *body)
{
	int result = 0;
	ghttp_request *request;
	if(!uri || strlen(uri) <= 0)
		return 0;
	if(!body || strlen(body) <= 0)
		return 0;

	request = ghttp_request_new();
	if(NULL == request)
		return 0;

	result = internal_ghttp_wrapper_send_onvif(request, uri, body);

	ghttp_close(request);
	ghttp_request_destroy(request);
	return result;
}

void ghttp_wrapper_clear()
{
	if(g_ghttp_request != NULL)
	{
		ghttp_request_destroy(g_ghttp_request);
		g_ghttp_request = NULL;
	}
}
