/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : cccdp_recv.h
  作者    : 贾延刚
  生成日期 : 2013-03-26

  版本    : 1.0
  功能描述 : 接收缓存链表
            保存所有的接收数据，通过回调函数输出数据：pdu或普通数据

  修改历史 :

******************************************************************************/

#ifndef __CCCDP_RECV_H__
#define __CCCDP_RECV_H__


#include "packet_pool/ccc_packet.h"

/*
 *
 */
typedef void (*cccdp_recv_callback)(int connect_type, int connect_no, CCCPACKET *dp_packet);

extern int cccdp_recv_init(cccdp_recv_callback callback);

extern int cccdp_recv_add_node(int connect_type, int connect_no);
extern int cccdp_recv_delete_node(int connect_type, int connect_no);

extern int cccdp_recv_save_data(int connect_type, int connect_no, const unsigned char *buffer, int data_len);

#endif  /* __CCCDP_RECV_H__ */
