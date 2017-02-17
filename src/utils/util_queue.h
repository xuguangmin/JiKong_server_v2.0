/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : util_queue.h
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-10-29
  ����޸�   :
  ��������   : һ��ͨ�õĶ��У����ݳ�Աֻ��һ��voidָ�롣
             ʹ��ʱ���ȶ���һ��UTIL_QUEUE���͵ı�����Ȼ����ú�����util_queue_init��
             �Զ��г�ʼ���󣬾Ϳ���ʹ���ˡ�
             ���ʹ��util_queue_release����˶��У���Ҫ���³�ʼ���Ժ�ſ���ʹ��

  �����б�   :  ���ɾ�����ͷβ�ڵ��ֵ
  �޸���ʷ   :

******************************************************************************/

#ifndef __UTIL_QUEUE_H__
#define __UTIL_QUEUE_H__

#include <pthread.h>

typedef struct queue_node *QUEUE_NODE;
struct queue_node
{
	void       *data;
	QUEUE_NODE  next;
};

struct util_queue
{
	QUEUE_NODE   head;
	QUEUE_NODE   tail;

	pthread_mutex_t queue_mutex;
};
typedef struct util_queue UTIL_QUEUE;

#define UTIL_QUEUE_INITIALIZER   { {0, 0, PTHREAD_MUTEX_INITIALIZER} }


extern void util_queue_init(UTIL_QUEUE *lp_util_queue);
extern int  util_queue_append_data(UTIL_QUEUE *lp_util_queue, void *data, int user_data);
extern int  util_queue_get_head_data(UTIL_QUEUE *lp_util_queue, void **data, int *user_data);
extern void util_queue_release(UTIL_QUEUE *lp_util_queue);

#endif  /* __UTIL_QUEUE_H__ */
