/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : cccdp_send.h
  ����    : ���Ӹ�
  �������� : 2013-04-01

  �汾    : 1.0
  �������� : ���ͻ�������
            �������еĽ������ݣ�ͨ���ص�����������ݣ�pdu����ͨ����

  �޸���ʷ :

******************************************************************************/

#ifndef __CCCDP_SEND_H__
#define __CCCDP_SEND_H__


#include "packet_pool/ccc_packet.h"

/*
 *
 */
typedef void (*cccdp_send_callback)(int connect_type, int connect_no, CCCPACKET *cccpacket);

extern int cccdp_send_init(cccdp_send_callback callback);

extern int cccdp_send_add_node(int connect_type, int connect_no);
extern int cccdp_send_delete_node(int connect_type, int connect_no);

extern int cccdp_send_save_data(int connect_type, int connect_no, const unsigned char *buffer, int data_len);

#endif  /* __CCCDP_SEND_H__ */
