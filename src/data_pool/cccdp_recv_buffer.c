/*
 * cccdp_recv_buffer.c
 *
 *  Created on: 2012-11-28
 *      Author: flx
 */

#include <stdlib.h>
#include <string.h>
#include "cccdp_recv_buffer.h"

#define CCCDP_RECV_BUFFER_SIZE_MAX              8192         /* �������ֵ���������ֵ�����ݽ����ӵ� */
/*
 * ����ջ��汣������
 * ������治������data_len���������󻺴�
 * ��������С������ĳ���޶�ֵ����������Ҳ����������߱�������
 * ����ֵ��
 *       1 �ɹ���0 ʧ��
 */
static int interval_cccdp_recv_buffer_save_data(CCCDP_RECV_BUFFER *recv_buf, const unsigned char *buffer, int data_len)
{
	int buffer_size;
	if(!recv_buf || !buffer || data_len <= 0)
		return 0;

	if(!recv_buf->buf)
	{
		buffer_size = data_len * 2;
		if(buffer_size > CCCDP_RECV_BUFFER_SIZE_MAX) buffer_size = CCCDP_RECV_BUFFER_SIZE_MAX;

		recv_buf->buf = (unsigned char *)malloc(buffer_size);
		if(!recv_buf->buf)
			return 0;

		recv_buf->data_len = 0;
		recv_buf->buf_size = buffer_size;
	}
	else if((recv_buf->buf_size - recv_buf->data_len) < data_len)
	{
		if(recv_buf->buf_size < CCCDP_RECV_BUFFER_SIZE_MAX)
		{
			unsigned char *temp_buf;
			buffer_size = recv_buf->buf_size + data_len * 2;
			if(buffer_size > CCCDP_RECV_BUFFER_SIZE_MAX) buffer_size = CCCDP_RECV_BUFFER_SIZE_MAX;

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
	//printf("cccdp_recv_buffer %s\n", (char *)recv_buf->buf);
	return 1;
}

int cccdp_recv_buffer_save_data(CCCDP_RECV_BUFFER *recv_buf, const unsigned char *buffer, int data_len)
{
	return interval_cccdp_recv_buffer_save_data(recv_buf, buffer, data_len);
}

int cccdp_recv_buffer_release(CCCDP_RECV_BUFFER *recv_buf)
{
	if(!recv_buf)
		return 0;

	if(recv_buf->buf) free(recv_buf->buf);
	recv_buf->buf = 0;
	recv_buf->buf_size = 0;
	recv_buf->data_len = 0;
	return 1;
}
int cccdp_recv_buffer_init(CCCDP_RECV_BUFFER *recv_buf)
{
	if(!recv_buf)
		return 0;

	recv_buf->buf = 0;
	recv_buf->buf_size = 0;
	recv_buf->data_len = 0;
	return 1;
}


