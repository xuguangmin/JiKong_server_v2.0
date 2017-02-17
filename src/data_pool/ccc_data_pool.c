/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : ccc_data_pool.c
  作者    : 贾延刚
  生成日期 : 2013-03-26

  版本    : 1.0
  功能描述 : 数据池

  修改历史 :

******************************************************************************/

#include "ccc_data_pool.h"
#include "tcp_server/connect_manager.h"
#include "packet_pool/ccc_packet.h"
#include "packet_pool/packet_pool.h"
#include "cccdp_recv.h"
#include "cccdp_send.h"

static data_pool_callback  g_data_pool_callback = (void *)0;

static void invoke_data_pool_callback(int connect_type, int connect_no, CCCPACKET *dp_packet)
{
	if(g_data_pool_callback && dp_packet)
		g_data_pool_callback(connect_type, connect_no, dp_packet);
}

void append_to_telnet_recv_deque(int connect_no, CCCPACKET *dp_packet)
{
	/* 应该进入telnet处理链表，然后由一个线程处理 */
	invoke_data_pool_callback(CONNECT_TYPE_TELNET, connect_no, dp_packet);
}
void append_to_serial_recv_deque(int connect_no, CCCPACKET *dp_packet)
{
	/* 应该进入serial处理链表，然后由一个线程处理 */
}

/*
 * 回调函数
 * 来自接收数据池的数据
 */
// TODO: check
void callback_cccdp_recv(int connect_type, int connect_no, CCCPACKET *dp_packet)
{
	if(!dp_packet)
		return;

	switch(connect_type)
	{
	case CONNECT_TYPE_TELNET:
		append_to_telnet_recv_deque(connect_no, dp_packet);
		break;
	}
}

void append_to_telnet_send_deque(int connect_type, int connect_no, CCCPACKET *cccpacket)
{
	if(!cccpacket)
		return;

	/* 将来应该进入telnet发送链表，然后由一个线程处理 */
	connect_manager_send_data(connect_type, connect_no, cccpacket->buffer, cccpacket->data_len);
	cccpacket_not_using(cccpacket);
}

/*
 * 回调函数
 * 来自发送数据池的数据
 */
void callback_cccdp_send(int connect_type, int connect_no, CCCPACKET *cccpacket)
{
	if(!cccpacket)
		return;

	switch(connect_type)
	{
	case CONNECT_TYPE_TELNET:
		append_to_telnet_send_deque(connect_type, connect_no, cccpacket);
		break;
	}

}

/* 追加接收数据 */
int data_pool_save_recv_data(int connect_type, int connect_no, const unsigned char *buffer, int data_len)
{
	return cccdp_recv_save_data(connect_type, connect_no, buffer, data_len);
}

int data_pool_save_send_data_telnet(int connect_no, const unsigned char *buffer, int data_len)
{
	return data_pool_save_send_data(CONNECT_TYPE_TELNET, connect_no, buffer, data_len);
}
/* 保存发送数据到发送数据池 */
int data_pool_save_send_data(int connect_type, int connect_no, const unsigned char *buffer, int data_len)
{
	return cccdp_send_save_data(connect_type, connect_no, buffer, data_len);
}
/* 删除节点 */
int data_pool_delete_node(int connect_type, int connect_no)
{
	if(!cccdp_recv_delete_node(connect_type, connect_no))
		return 0;

	return cccdp_send_delete_node(connect_type, connect_no);
}
/* 增加节点 */
int data_pool_add_node(int connect_type, int connect_no)
{
	if(!cccdp_recv_add_node(connect_type, connect_no))
		return 0;

	return cccdp_send_add_node(connect_type, connect_no);
}

int data_pool_init(data_pool_callback callback)
{	// do nothing
	if(!packet_pool_init())
		return 0;
	if(!cccdp_recv_init(callback_cccdp_recv))
		return 0;
	if(!cccdp_send_init(callback_cccdp_send))
		return 0;

	g_data_pool_callback = callback;
	return 1;
}
