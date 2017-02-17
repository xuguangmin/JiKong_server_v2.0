/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : ring_list_buf.h
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-09-21
  最近修改   :
  功能描述   : 环形链表
              使用时，先定义一个RING_LIST_BUF类型的变量，然后调用函数：ring_list_init
              以后就可以使用该变量了

  函数列表   :
  修改历史   :

******************************************************************************/

#ifndef __RING_LIST_BUF_H__
#define __RING_LIST_BUF_H__

#include <pthread.h>

typedef struct buf_node *BUF_NODE;
struct buf_node
{
	unsigned char item;
	BUF_NODE      next;
};

struct ring_list_buf
{
	BUF_NODE   head;
	BUF_NODE   tail;
	int    ring_size;
	int    using_size;
	pthread_mutex_t ring_mutex;
};
typedef struct ring_list_buf RING_LIST_BUF;


extern void ring_list_buf_init(RING_LIST_BUF *lp_ring_list);
extern int  ring_list_buf_append(RING_LIST_BUF *lp_ring_list, unsigned char item);
extern int  ring_list_buf_get_data(RING_LIST_BUF *lp_ring_list, unsigned char *buffer, int size);
extern void ring_list_buf_release(RING_LIST_BUF *lp_ring_list);
extern int  ring_list_buf_data_length(RING_LIST_BUF *lp_ring_list);

#endif  /* __RING_LIST_BUF_H__ */
