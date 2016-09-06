/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : connect_list.h
  作者    : 贾延刚
  生成日期 : 2012-11-16

  版本    : 1.0
  功能描述 : 保存所有的连接


  修改历史 :

******************************************************************************/
#ifndef __CONNECT_LIST_H__
#define __CONNECT_LIST_H__

#include "FLXCommon/flxnettypes.h"


typedef struct connect_node *CONNECT_NODE;
struct connect_node
{
	int            connect_type;         /* 连接类型 */
	int            connect_no;

	FLXSocket      sock;			     /* 套接字 */
	char          *ip_address;
	int            ip_port;
	//void          *user_data;            /* 保存一个上层传进来的数据，回调时传出去 */

	CONNECT_NODE   prev;
	CONNECT_NODE   next;
};

typedef int (*connect_list_callback)(CONNECT_NODE connect_node);


extern int  connect_list_init(connect_list_callback callback);
extern void connect_list_release(void);

extern int  connect_list_delete(int connect_type, int connect_no);
extern int  connect_list_add_telnet(int connect_type, int connect_no, const char *ip_address, int port);
extern int  connect_list_add_http(int connect_type, int connect_no, const char *ip_address, int port);

extern int  connect_list_reconnect(FLXSocket sock);
extern int  connect_send_data(int connect_type, int connect_no, unsigned char *buffer, int data_len);


#endif  /* __CONNECT_LIST_H__ */
