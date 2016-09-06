/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : ring_list_buf.h
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-09-21
  ����޸�   :
  ��������   : ��������
              ʹ��ʱ���ȶ���һ��RING_LIST_BUF���͵ı�����Ȼ����ú�����ring_list_init
              �Ժ�Ϳ���ʹ�øñ�����

  �����б�   :
  �޸���ʷ   :

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
