/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : download_file.h
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-11-09
  最近修改   :
  功能描述   : 一个保存下载文件列表的队列
             在队列的头部增加节点，新增加的节点总是在最前边
             每一个节点对应一个客户端，节点保存的当前正在上传的文件的信息
             因为协议在下载过程的应答中未提供文件类型信息，所以一个客户端
             同一个时候只能下载一个文件

******************************************************************************/

#ifndef __DOWNLOAD_FILE_H__
#define __DOWNLOAD_FILE_H__

#include "FLXCommon/flxnettypes.h"

extern int add_download_file_node(FLXSocket sock, FILE *file_handle, const char *filename);
extern int delete_download_file_node(FLXSocket sock);
extern FILE *get_download_file_handle(FLXSocket sock);

extern int pop_download_file_node(FLXSocket *sock, unsigned char *protocol_file_type);

#endif /* __DOWNLOAD_FILE_H__ */
