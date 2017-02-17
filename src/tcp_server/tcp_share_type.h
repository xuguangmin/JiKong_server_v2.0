/*
 * tcp_share_type.h
 *
 *  Created on: 2012-11-13
 *      Author: flx
 */

#ifndef __TCP_SHARE_TYPE_H__
#define __TCP_SHARE_TYPE_H__

#include "tps_client_manage.h"

#define DATA_SOURCE_PAD_OR_0               0           /* 数据来自pad或者设计器等客户端 */
#define DATA_SOURCE_SLAVE_SERVER           1           /* 数据来自从机 */
#define DATA_SOURCE_SLAVE_CLIENT           2           /* 数据来自从机上的客户端连接 */

#define DESCRIBE_PAD_OR_0                 "pad or designer"



#define TELNET_ACTION_ADD                  1            /* 增加了一个telnet连接 */
#define TELNET_ACTION_DELETE               2            /* 增加了一个telnet连接 */

#define CONNECT_TYPE_UNKNOWN              -1
#define CONNECT_TYPE_TELNET                0            /* 普通的从客户端到服务器的连接 */
#define CONNECT_TYPE_TELNET_FOR_HTTP       1
#define CONNECT_TYPE_CLIENT                2            /* 服务器接收到的客户端 */

/*
 * 从网络获取数据的回调函数接口
 * 参数：
 *     data_source  数据来源
 *     buffer       数据缓存
 *     data_len     缓存中数据长度
 *     clientInfo   客户端
 *
 * data_source 取值：
 *                 DATA_SOURCE_PAD_OR_0, DATA_SOURCE_SLAVE_SERVER, DATA_SOURCE_SLAVE_CLIENT
 */
typedef void (*network_recv_data)(int data_source, unsigned char *buffer, int *data_len, CLIENT_INFO_STRU *clientInfo);

#endif /* __TCP_SHARE_TYPE_H__ */
