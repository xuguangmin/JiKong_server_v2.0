/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : upload_file.h
  �� �� ��   : ����
  ��   ��   : ���Ӹ�
  ��������   : 2012-10-26
  ����޸�   :
  ��������   : һ�������ϴ��ļ��б�Ķ���
             �ڶ��е�ͷ�����ӽڵ㣬�����ӵĽڵ���������ǰ��
             ÿһ���ڵ��Ӧһ���ͻ��ˣ�����ͻ��˱����ϴ�������
             �ļ���Ϣ��������ÿ���ڵ��ڲ���һ�������У��ڵ㱣���
             �ǵ�ǰ�����ϴ����ļ�����Ϣ

  �����б�   :
  �޸���ʷ   :

******************************************************************************/

#ifndef __UPLOAD_FILE_H__
#define __UPLOAD_FILE_H__

#include <stdio.h>
#include "FLXCommon/flxnettypes.h"
#include "util_queue.h"

struct ul_file_info
{
	unsigned char  protocol_file_type;      //��ǰ���ڴ�����ļ�����
	char          *filename;                //�ͻ����ύ���ļ���
	char          *filename_tx;             //����ʱʹ�õ��ļ���
};

extern void delete_upload_file_node(FLXSocket sock);
extern int add_upload_file_node(FLXSocket sock, unsigned char  protocol_file_type, const char *filename);

extern FILE *get_upload_file_handle(FLXSocket sock);
extern int clear_single_upload(FLXSocket sock);

extern const char *get_upload_filename(FLXSocket sock);
extern const char *get_upload_filename_tx(FLXSocket sock);
extern UTIL_QUEUE *get_upload_file_history(FLXSocket sock);


#endif /* __UPLOAD_FILE_H__ */
