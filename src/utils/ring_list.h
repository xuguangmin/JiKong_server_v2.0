/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : ring_list.h
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-09-21
  ��������   : ��������
              ʹ��ʱ���ȶ���һ��RING_LIST���͵ı�����Ȼ����ú�����ring_list_init
              �Ժ�Ϳ���ʹ�øñ�����

  �����б�   :
  �޸���ʷ   :

******************************************************************************/

#ifndef __RING_LIST_H__
#define __RING_LIST_H__

#include <pthread.h>


typedef struct ring_list_node *RING_LIST_NODE;
struct ring_list_node
{
	unsigned char *buf;
	int            buf_size;
	int            data_len;
	int            is_using;        /* �Ƿ�ʹ�� */
	int            user_data;       /* ���Ա���һ����������� */

	RING_LIST_NODE next;
};

struct ring_list
{
	RING_LIST_NODE  head;
	RING_LIST_NODE  tail;
	int             ring_size;
	int             using_size;
	int             check_point;
	pthread_mutex_t ring_mutex;
};
typedef struct ring_list RING_LIST;


extern void ring_list_init(RING_LIST *lp_ring_list);
extern void ring_list_check_point(RING_LIST *lp_ring_list, int check_point);
extern int  ring_list_append_data(RING_LIST *lp_ring_list, const unsigned char *buffer, int data_len, int user_data);
extern int  ring_list_get_data(RING_LIST *lp_ring_list, unsigned char *buffer, int size, int *user_data);
extern void ring_list_release(RING_LIST *lp_ring_list);

extern int  ring_list_copy_data(RING_LIST *lp_ring_list, unsigned char *buffer, int size, int *user_data);
extern int  ring_list_delete_data(RING_LIST *lp_ring_list, int data_len);

#endif  /* __RING_LIST_H__ */
