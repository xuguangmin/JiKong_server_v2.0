/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : util_list.h
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-11-08
  最近修改   :
  功能描述   : 一个通用的列表，数据成员只有一个void指针。
             在列表的头部增加节点，新增加的节点总是在最前边

  函数列表   :
  修改历史   :

******************************************************************************/

#ifndef __UTIL_LIST_H__
#define __UTIL_LIST_H__

#include <pthread.h>

typedef struct list_node *LIST_NODE;
struct list_node
{
	void      *data;
	LIST_NODE  prev;
	LIST_NODE  next;
};

#define UTIL_LIST_INITIALIZER   { {0, PTHREAD_MUTEX_INITIALIZER} }
struct util_list
{
	LIST_NODE       head;
	pthread_mutex_t list_mutex;
};
typedef struct util_list UTIL_LIST;


extern int  add_list_node(UTIL_LIST *lp_util_list, void *data);
extern void delete_list_node(UTIL_LIST *lp_util_list, void *data);
extern void delete_all_list_node();

#endif /* __UTIL_LIST_H__ */
