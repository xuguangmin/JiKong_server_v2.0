/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文 件 名   : telnet_info.h
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-12-07
  最近修改   :
  功能描述   : 一个保存telnet连接信息的队列
             在队列的头部增加节点，新增加的节点总是在最前边

******************************************************************************/

#ifndef __TELNET_INFO_H__
#define __TELNET_INFO_H__

extern int add_telnet_info_node(int controlId, const char *ip_address, int port, int id);
extern int delete_telnet_info_node(int controlId);
extern int get_telnet_info_node_id(int controlId);

#endif /* __TELNET_INFO_H__ */
