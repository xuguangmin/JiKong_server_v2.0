/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ���    ��tps_tcp_slave_client.h
  ������    �����Ӹ�
  ��������   ��2012-11-6
  ��������   : tcp�ͻ��ˣ����ڷ�������Ϊ�ӻ�ʱ������һ��������������
  �����б�   :
  �޸���ʷ   :

******************************************************************************/

#ifndef __TPS_TCP_SLAVE_CLIENT_H__
#define __TPS_TCP_SLAVE_CLIENT_H__

/*
 * data      ���ݻ���
 * data_len  �������ݵĳ��ȣ����ػ����л�ʣ������
 */
typedef int (*tps_tcp_slave_client_callback)(unsigned char *data, int *data_len);

extern int tps_tcp_slave_client_start(const char *serverIp, int serverPort, tps_tcp_slave_client_callback callback);
extern int tps_tcp_slave_client_send(unsigned char *data, int data_len);

#endif /* __TPS_TCP_SLAVE_CLIENT_H__ */
