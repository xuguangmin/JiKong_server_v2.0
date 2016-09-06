/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : connecting_list.h
  ����    : ���Ӹ�
  �������� : 2012-11-19

  �汾    : 1.0
  �������� : ������Ҫ���ӵ�socket�Ķ����б�
           �̻߳����ʱ�����һ�£����ӳɹ���
           ���Զ�ɾ���ڵ�

  �޸���ʷ :

******************************************************************************/
#ifndef __CONNECTING_LIST_H__
#define __CONNECTING_LIST_H__

#include "FLXCommon/flxnettypes.h"

/*
 * ���ӳɹ��󣬻��������ص�������֪ͨ�ϲ�
 */
typedef int (*connecting_callback)(int connect_type, int connect_no, FLXSocket sock);

extern int  connecting_list_init(connecting_callback callback);
extern void connecting_list_release(void);

extern int  connecting_list_add(int connect_type, int connect_no, const char *ip_address, int port);
extern int  connecting_list_delete(int connect_type, int connect_no);

#endif  /* __CONNECTING_LIST_H__ */
