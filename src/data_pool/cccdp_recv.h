/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : cccdp_recv.h
  ����    : ���Ӹ�
  �������� : 2013-03-26

  �汾    : 1.0
  �������� : ���ջ�������
            �������еĽ������ݣ�ͨ���ص�����������ݣ�pdu����ͨ����

  �޸���ʷ :

******************************************************************************/

#ifndef __CCCDP_RECV_H__
#define __CCCDP_RECV_H__


#include "packet_pool/ccc_packet.h"

/*
 *
 */
typedef void (*cccdp_recv_callback)(int connect_type, int connect_no, CCCPACKET *dp_packet);

extern int cccdp_recv_init(cccdp_recv_callback callback);

extern int cccdp_recv_add_node(int connect_type, int connect_no);
extern int cccdp_recv_delete_node(int connect_type, int connect_no);

extern int cccdp_recv_save_data(int connect_type, int connect_no, const unsigned char *buffer, int data_len);

#endif  /* __CCCDP_RECV_H__ */
