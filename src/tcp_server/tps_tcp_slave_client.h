/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文件名    ：tps_tcp_slave_client.h
  创建者    ：贾延刚
  生成日期   ：2012-11-6
  功能描述   : tcp客户端，用于服务器做为从机时，建立一个到主机的连接
  函数列表   :
  修改历史   :

******************************************************************************/

#ifndef __TPS_TCP_SLAVE_CLIENT_H__
#define __TPS_TCP_SLAVE_CLIENT_H__

/*
 * data      数据缓存
 * data_len  传入数据的长度；返回缓存中还剩的数据
 */
typedef int (*tps_tcp_slave_client_callback)(unsigned char *data, int *data_len);

extern int tps_tcp_slave_client_start(const char *serverIp, int serverPort, tps_tcp_slave_client_callback callback);
extern int tps_tcp_slave_client_send(unsigned char *data, int data_len);

#endif /* __TPS_TCP_SLAVE_CLIENT_H__ */
