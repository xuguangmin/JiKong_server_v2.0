/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : protocol_adapter.h
  作者    : chen zhi tao
  生成日期 : 2010-12-22

  版本    : 1.0
  功能描述 : 数据解析、处理等

  修改历史 : 贾延刚多次修改
           现在，该文件做为整个程序的核心，连接起来基本所有的功能模块。

******************************************************************************/

#ifndef __PROTOCOL_ADAPTER_H__
#define __PROTOCOL_ADAPTER_H__

#include "device_interface/devi_protocol.h"
#include "tcp_server/tps_tcp_server.h"
#include "data_pool/ccc_data_pool.h"

extern int  protocol_adapter_init(int server_type);
extern int  send_devi_proto_data(int cmd, int cmd_ex, struct DEVI_PROTO_DATA *lp_devi_proto_data);

extern void callback_recv_data_from_serial(int serial_no, unsigned char *buffer, int size);
extern void callback_recv_data_from_network(int data_source, unsigned char *buffer, int *data_len, CLIENT_INFO_STRU *clientInfo);
extern void callback_recv_data_from_data_pool(int connect_type, int connect_no, CCCPACKET *dp_packet);
extern void callback_by_connect_manager(int reason, int connect_type, int connect_no, unsigned char *buffer, int data_len);


#endif // __PROTOCOL_ADAPTER_H__
