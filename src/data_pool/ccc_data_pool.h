/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : ccc_data_pool.c
  ����    : ���Ӹ�
  �������� : 2013-03-26

  �汾    : 1.0
  �������� : ���ݳ�

  �޸���ʷ :

******************************************************************************/

#ifndef __CCC_DATA_POOL_H__
#define __CCC_DATA_POOL_H__

#include "packet_pool/ccc_packet.h"

typedef void (*data_pool_callback)(int connect_type, int connect_no, CCCPACKET *dp_packet);

extern int data_pool_init(data_pool_callback callback);
extern int data_pool_add_node(int connect_type, int connect_no);
extern int data_pool_delete_node(int connect_type, int connect_no);

extern int data_pool_save_recv_data(int connect_type, int connect_no, const unsigned char *buffer, int data_len);
extern int data_pool_save_send_data(int connect_type, int connect_no, const unsigned char *buffer, int data_len);
extern int data_pool_save_send_data_telnet(int connect_no, const unsigned char *buffer, int data_len);

#endif  /* __CCC_DATA_POOL_H__ */
