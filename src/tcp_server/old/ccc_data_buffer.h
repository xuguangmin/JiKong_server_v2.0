/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : ccc_data_buffer.c
  ����    : ���Ӹ�
  �������� : 2012-11-22

  �汾    : 1.0
  �������� : �����շ����ݵ�����
           ������ͷ��β������ͷ�ڵ�ָ��Ľڵ㣬�������淢�͸�����ڵ������
           �ϲ��ȡ�������ݣ���ô�β����ʼȡ����Ϊ�µĽڵ��׷�ӵ�β��


  �޸���ʷ :

******************************************************************************/

#ifndef __CCC_DATA_BUFFER_H__
#define __CCC_DATA_BUFFER_H__

#include <pthread.h>
#include "ring_list.h"
#include "recv_buffer.h"

/*
#define DATA_BUFFER_RECV_MAX    4096
struct data_buffer_recv_data
{
	int            id;
	unsigned char  buffer[DATA_BUFFER_RECV_MAX];
	int            buf_size;

	int            type;
};
*/
#define DATA_BUFFER_TYPE_CONN        0
#define DATA_BUFFER_TYPE_CLIENT      1

typedef struct data_buffer_node *DATA_BUFFER_NODE;
struct data_buffer_node
{
	int              id;                /* �ڵ�ı�� */
	int              connect_no;        /* �ڵ��Ӧ��ͨѶ���ӵı�� */
	int              b_connect_enable;  /* �ڵ��Ӧ��ͨѶ�����Ƿ��Ѿ����� */
	int              type;              /* ���ݻ�������͡���ͬ���͵Ļ��棬�����е����ݵĴ���ʽ����һ���Ĳ��� */

	RECV_BUFFER      recv_buf;
	int              b_rb_get;          /* �Ƿ��Ѿ��ڴ������ݣ���Ҫ����ϲ�ȡ���� */

	RING_LIST        send_buf;

	DATA_BUFFER_NODE prev;
	DATA_BUFFER_NODE next;
};

extern void free_data_buffer_node(DATA_BUFFER_NODE lp_node);
extern DATA_BUFFER_NODE create_data_buffer_node_for_telnet();
extern DATA_BUFFER_NODE create_data_buffer_node_for_client();

extern int insert_data_buffer_node(DATA_BUFFER_NODE db_node);
extern int delete_data_buffer_node(int id);
extern int get_data_buffer_node_connect_no(int id);

extern int data_buffer_save_recv_data(DATA_BUFFER_NODE db_node, const unsigned char *buffer, int data_len);
extern int data_buffer_poll_recv_data(unsigned char *buffer, int buf_size, int *id, int *type);

extern int data_buffer_save_send_data(int id, const unsigned char *buffer, int data_len);
extern int data_buffer_poll_send_data(unsigned char *buffer, int buf_size, int *connect_no);
extern int data_buffer_delete_send_data(int connect_no, int data_len);

extern int data_buffer_node_set_connect_enable(int connect_no, int b_enable);

#endif  /* __CCC_DATA_BUFFER_H__ */
