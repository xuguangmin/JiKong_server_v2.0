/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : util_queue.h
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-10-29
  最近修改   :
  功能描述   : 一个通用的队列，数据成员只有一个void指针。
             使用时，先定义一个UTIL_QUEUE类型的变量，然后调用函数：util_queue_init，
             对队列初始化后，就可以使用了。
             如果使用util_queue_release清空了队列，需要重新初始化以后才可以使用

  函数列表   :  检查删除后的头尾节点的值
  修改历史   :

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
