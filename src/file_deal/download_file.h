/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : download_file.h
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-11-09
  ����޸�   :
  ��������   : һ�����������ļ��б�Ķ���
             �ڶ��е�ͷ�����ӽڵ㣬�����ӵĽڵ���������ǰ��
             ÿһ���ڵ��Ӧһ���ͻ��ˣ��ڵ㱣��ĵ�ǰ�����ϴ����ļ�����Ϣ
             ��ΪЭ�������ع��̵�Ӧ����δ�ṩ�ļ�������Ϣ������һ���ͻ���
             ͬһ��ʱ��ֻ������һ���ļ�

******************************************************************************/

#ifndef __DOWNLOAD_FILE_H__
#define __DOWNLOAD_FILE_H__

#include "FLXCommon/flxnettypes.h"

extern int add_download_file_node(FLXSocket sock, FILE *file_handle, const char *filename);
extern int delete_download_file_node(FLXSocket sock);
extern FILE *get_download_file_handle(FLXSocket sock);

extern int pop_download_file_node(FLXSocket *sock, unsigned char *protocol_file_type);

#endif /* __DOWNLOAD_FILE_H__ */
