/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : telnet_info.h
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-12-07
  ����޸�   :
  ��������   : һ������telnet������Ϣ�Ķ���
             �ڶ��е�ͷ�����ӽڵ㣬�����ӵĽڵ���������ǰ��

******************************************************************************/

#ifndef __TELNET_INFO_H__
#define __TELNET_INFO_H__

extern int add_telnet_info_node(int controlId, const char *ip_address, int port, int id);
extern int delete_telnet_info_node(int controlId);
extern int get_telnet_info_node_id(int controlId);

#endif /* __TELNET_INFO_H__ */
