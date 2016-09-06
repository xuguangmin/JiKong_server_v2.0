/*
 * recv_buffer.h
 *
 *  Created on: 2012-11-28
 *      Author: flx
 */

#ifndef __RECV_BUFFER_H__
#define __RECV_BUFFER_H__

#include <pthread.h>


struct recv_buffer
{
	unsigned char   *buf;
	int              buf_size;       /* ���ջ���Ŀǰ�Ĵ�С */
	int              data_len;       /* ���ջ��������ݳ��� */
	int              user_data;      /* ���Ա���һ����������� */

	pthread_mutex_t  buf_mutex;
};
typedef struct recv_buffer RECV_BUFFER;

#define RECV_BUFFER_INITIALIZER   {{0, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER}}

extern int recv_buffer_init(RECV_BUFFER *recv_buf);
extern int recv_buffer_release(RECV_BUFFER *recv_buf);
extern int recv_buffer_save_data(RECV_BUFFER *recv_buf, const unsigned char *buffer, int data_len);
extern int recv_buffer_copy_data(RECV_BUFFER *recv_buf, unsigned char *buffer, int buf_size);
extern int recv_buffer_get_data(RECV_BUFFER *recv_buf, unsigned char *buffer, int buf_size);

#endif /* __RECV_BUFFER_H__ */
