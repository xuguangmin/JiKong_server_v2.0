/*
 * cccdp_recv_buffer.h
 *
 *  Created on: 2013-03-26
 *      Author: flx
 */

#ifndef __CCCDP_RECV_BUFFER_H__
#define __CCCDP_RECV_BUFFER_H__

#include <pthread.h>


struct cccdp_recv_buffer
{
	unsigned char   *buf;
	int              buf_size;       /* 接收缓存目前的大小 */
	int              data_len;       /* 接收缓存中数据长度 */
};
typedef struct cccdp_recv_buffer CCCDP_RECV_BUFFER;

extern int cccdp_recv_buffer_init(CCCDP_RECV_BUFFER *recv_buf);
extern int cccdp_recv_buffer_release(CCCDP_RECV_BUFFER *recv_buf);
extern int cccdp_recv_buffer_save_data(CCCDP_RECV_BUFFER *recv_buf, const unsigned char *buffer, int data_len);

#endif /* __CCCDP_RECV_BUFFER_H__ */
