/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : ccc_data_buffer.c
  作者    : 贾延刚
  生成日期 : 2012-11-22

  版本    : 1.0
  功能描述 : 保存收发数据的链表
           链表有头有尾，链表头节点指向的节点，用来保存发送给多个节点的数据
           上层获取接收数据，最好从尾部开始取，因为新的节点会追加到尾部


  修改历史 :

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
	int              id;                /* 节点的编号 */
	int              connect_no;        /* 节点对应的通讯连接的编号 */
	int              b_connect_enable;  /* 节点对应的通讯连接是否已经连接 */
	int              type;              /* 数据缓存的类型。不同类型的缓存，对其中的数据的处理方式会有一定的差异 */

	RECV_BUFFER      recv_buf;
	int              b_rb_get;          /* 是否已经在处理数据，主要针对上层取数据 */

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
