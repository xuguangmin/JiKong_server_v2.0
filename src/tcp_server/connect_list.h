/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : connect_list.h
  ����    : ���Ӹ�
  �������� : 2012-11-16

  �汾    : 1.0
  �������� : �������е�����


  �޸���ʷ :

******************************************************************************/
#ifndef __CONNECT_LIST_H__
#define __CONNECT_LIST_H__

#include "FLXCommon/flxnettypes.h"


typedef struct connect_node *CONNECT_NODE;
struct connect_node
{
	int            connect_type;         /* �������� */
	int            connect_no;

	FLXSocket      sock;			     /* �׽��� */
	char          *ip_address;
	int            ip_port;
	//void          *user_data;            /* ����һ���ϲ㴫���������ݣ��ص�ʱ����ȥ */

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
