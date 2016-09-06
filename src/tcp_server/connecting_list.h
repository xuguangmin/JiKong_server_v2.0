/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : connecting_list.h
  作者    : 贾延刚
  生成日期 : 2012-11-19

  版本    : 1.0
  功能描述 : 保存需要连接的socket的队列列表
           线程会隔段时间调用一下，连接成功后，
           将自动删除节点

  修改历史 :

******************************************************************************/
#ifndef __CONNECTING_LIST_H__
#define __CONNECTING_LIST_H__

#include "FLXCommon/flxnettypes.h"

/*
 * 连接成功后，会调用这个回调函数，通知上层
 */
typedef int (*connecting_callback)(int connect_type, int connect_no, FLXSocket sock);

extern int  connecting_list_init(connecting_callback callback);
extern void connecting_list_release(void);

extern int  connecting_list_add(int connect_type, int connect_no, const char *ip_address, int port);
extern int  connecting_list_delete(int connect_type, int connect_no);

#endif  /* __CONNECTING_LIST_H__ */
