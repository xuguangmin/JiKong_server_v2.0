/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : ring_list.h
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-09-21
  功能描述   : 环形链表
              使用时，先定义一个RING_LIST类型的变量，然后调用函数：ring_list_init
              以后就可以使用该变量了

  函数列表   :
  修改历史   :

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
	int            is_using;        /* 是否被使用 */
	int            user_data;       /* 可以保存一个额外的数据 */

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
