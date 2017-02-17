/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : util_list.h
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-11-08
  ����޸�   :
  ��������   : һ��ͨ�õ��б����ݳ�Աֻ��һ��voidָ�롣
             ���б��ͷ�����ӽڵ㣬�����ӵĽڵ���������ǰ��

  �����б�   :
  �޸���ʷ   :

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
