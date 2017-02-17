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

#ifndef __CCC_DATA_POOL_H__
#define __CCC_DATA_POOL_H__

#include "packet_pool/ccc_packet.h"

typedef void (*data_pool_callback)(int connect_type, int connect_no, CCCPACKET *dp_packet);

extern int data_pool_init(data_pool_callback callback);
extern int data_pool_add_node(int connect_type, int connect_no);
extern int data_pool_delete_node(int connect_type, int connect_no);

extern int data_pool_save_recv_data(int connect_type, int connect_no, const unsigned char *buffer, int data_len);
extern int data_pool_save_send_data(int connect_type, int connect_no, const unsigned char *buffer, int data_len);
extern int data_pool_save_send_data_telnet(int connect_no, const unsigned char *buffer, int data_len);

#endif  /* __CCC_DATA_POOL_H__ */
