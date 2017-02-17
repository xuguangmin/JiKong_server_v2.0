/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : cccdp_send_queue.h
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2013-04-01
  功能描述   : 环形链表
              使用时，先定义一个RING_LIST类型的变量，然后调用函数：cccdp_send_queue_init
              以后就可以使用该变量了

  函数列表   :
  修改历史   :

******************************************************************************/

#ifndef __CCCDP_SEND_QUEUE_H__
#define __CCCDP_SEND_QUEUE_H__

#include <pthread.h>
#include "packet_pool/ccc_packet.h"

typedef struct cccdp_send_queue_node *CCCDP_SEND_QUEUE_NODE;
struct cccdp_send_queue_node
{
	void                  *buf;
	CCCDP_SEND_QUEUE_NODE  next;
};

struct cccdp_send_queue
{
	CCCDP_SEND_QUEUE_NODE  head;
	CCCDP_SEND_QUEUE_NODE  tail;
	int                    ring_size;
	int                    using_size;
	int                    check_point;
	pthread_mutex_t        ring_mutex;
};
typedef struct cccdp_send_queue CCCDP_SEND_QUEUE;


extern void cccdp_send_queue_init(CCCDP_SEND_QUEUE *lp_ring_list);
extern void cccdp_send_queue_release(CCCDP_SEND_QUEUE *lp_ring_list);

extern int  cccdp_send_queue_add_packet(CCCDP_SEND_QUEUE *lp_ring_list, CCCPACKET *cccpacket);
extern CCCPACKET *cccdp_send_queue_get_head(CCCDP_SEND_QUEUE *lp_ring_list);

#endif  /* __CCCDP_SEND_QUEUE_H__ */
