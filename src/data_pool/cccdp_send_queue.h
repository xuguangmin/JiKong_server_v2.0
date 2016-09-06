/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : cccdp_send_queue.h
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2013-04-01
  ��������   : ��������
              ʹ��ʱ���ȶ���һ��RING_LIST���͵ı�����Ȼ����ú�����cccdp_send_queue_init
              �Ժ�Ϳ���ʹ�øñ�����

  �����б�   :
  �޸���ʷ   :

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
