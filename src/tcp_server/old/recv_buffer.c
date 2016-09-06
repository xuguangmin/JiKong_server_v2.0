/*
 * recv_buffer.c
 *
 *  Created on: 2012-11-28
 *      Author: flx
 */

#include <stdlib.h>
#include <string.h>
#include "recv_buffer.h"

#define RECV_BUFFER_SIZE_MAX              8192         /* 缓存最大值，超过这个值，数据将被扔掉 */
/*
 * 向接收缓存保存数据
 * 如果缓存不够，则按data_len的两倍扩大缓存
 * 如果缓存大小超过了某个限定值，则不再增大，也不会再往里边保存数据
 * 返回值：
 *       1 成功；0 失败
 */
static int interval_recv_buffer_save_data(RECV_BUFFER *recv_buf, const unsigned char *buffer, int data_len)
{
	int buffer_size;
	if(!recv_buf || !buffer || data_len <= 0)
		return 0;

	if(!recv_buf->buf)
	{
		buffer_size = data_len * 2;
		if(buffer_size > RECV_BUFFER_SIZE_MAX) buffer_size = RECV_BUFFER_SIZE_MAX;

		recv_buf->buf = (unsigned char *)malloc(buffer_size);
		if(!recv_buf->buf)
			return 0;

		recv_buf->data_len = 0;
		recv_buf->buf_size = buffer_size;
	}
	else if((recv_buf->buf_size - recv_buf->data_len) < data_len)
	{
		if(recv_buf->buf_size < RECV_BUFFER_SIZE_MAX)
		{
			unsigned char *temp_buf;
			buffer_size = recv_buf->buf_size + data_len * 2;
			if(buffer_size > RECV_BUFFER_SIZE_MAX) buffer_size = RECV_BUFFER_SIZE_MAX;

			temp_buf = (unsigned char *)malloc(buffer_size);
			if(!temp_buf)
				return 0;

			memcpy(temp_buf, recv_buf->buf, recv_buf->data_len);
			if(recv_buf->buf) free(recv_buf->buf);

			recv_buf->buf = temp_buf;
			recv_buf->buf_size = buffer_size;
		}
	}

	if(data_len <= (recv_buf->buf_size - recv_buf->data_len))
	{
		memcpy(&recv_buf->buf[recv_buf->data_len], buffer, data_len);
		recv_buf->data_len += data_len;
	}
	else{
		return 0;
	}

	//recv_buf->buf[recv_buf->data_len] = '\0';
	//printf("recv_buffer %s\n", (char *)recv_buf->buf);
	return 1;
}

static int interval_recv_buffer_copy_data(RECV_BUFFER *recv_buf, unsigned char *buffer, int buf_size)
{
	int copy_len;
	if(!recv_buf->buf || recv_buf->data_len <= 0)
		return 0;
	if(!recv_buf || !buffer || buf_size <= 0)
		return 0;

	copy_len = (recv_buf->data_len < buf_size) ? recv_buf->data_len:buf_size;
	memcpy(buffer, recv_buf->buf, copy_len);
	return copy_len;
}

/*
 * 取数据后，删除相应数据
 */
static int interval_recv_buffer_get_data(RECV_BUFFER *recv_buf, unsigned char *buffer, int buf_size)
{
	int copy_len;
	if(!recv_buf || !buffer || buf_size <= 0)
	{
		return 0;
	}
	if(!recv_buf->buf || recv_buf->data_len <= 0)
	{
		return 0;
	}

	copy_len = (recv_buf->data_len < buf_size) ? recv_buf->data_len:buf_size;
	memcpy(buffer, recv_buf->buf, copy_len);

	recv_buf->data_len -= copy_len;
	if(recv_buf->data_len > 0) memmove(recv_buf->buf, recv_buf->buf+copy_len, recv_buf->data_len);

	//buffer[copy_len] = '\0';
	//printf("recv_buffer trim data %s\n", (char *)buffer);

	return copy_len;
}

int recv_buffer_save_data(RECV_BUFFER *recv_buf, const unsigned char *buffer, int data_len)
{
	int result;
	pthread_mutex_lock(&recv_buf->buf_mutex);
	result = interval_recv_buffer_save_data(recv_buf, buffer, data_len);
	pthread_mutex_unlock(&recv_buf->buf_mutex);
	return result;
}

int recv_buffer_copy_data(RECV_BUFFER *recv_buf, unsigned char *buffer, int buf_size)
{
	int result;
	pthread_mutex_lock(&recv_buf->buf_mutex);
	result = interval_recv_buffer_copy_data(recv_buf, buffer, buf_size);
	pthread_mutex_unlock(&recv_buf->buf_mutex);
	return result;
}

int recv_buffer_get_data(RECV_BUFFER *recv_buf, unsigned char *buffer, int buf_size)
{
	int result;
	pthread_mutex_lock(&recv_buf->buf_mutex);
	result = interval_recv_buffer_get_data(recv_buf, buffer, buf_size);
	pthread_mutex_unlock(&recv_buf->buf_mutex);
	return result;
}

int recv_buffer_release(RECV_BUFFER *recv_buf)
{
	if(!recv_buf)
		return 0;

	if(recv_buf->buf) free(recv_buf->buf);
	recv_buf->buf = 0;
	recv_buf->buf_size = 0;
	recv_buf->data_len = 0;
	recv_buf->user_data = 0;
	pthread_mutex_destroy(&recv_buf->buf_mutex);
	return 1;
}
int recv_buffer_init(RECV_BUFFER *recv_buf)
{
	if(!recv_buf)
		return 0;

	recv_buf->buf = 0;
	recv_buf->buf_size = 0;
	recv_buf->data_len = 0;
	recv_buf->user_data = 0;
	pthread_mutex_init(&recv_buf->buf_mutex, NULL);
	return 1;
}


