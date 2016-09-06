/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : upload_file.h
  版 本 号   : 初稿
  作   者   : 贾延刚
  生成日期   : 2012-10-26
  最近修改   :
  功能描述   : 一个保存上传文件列表的队列
             在队列的头部增加节点，新增加的节点总是在最前边
             每一个节点对应一个客户端，这个客户端本次上传的所有
             文件信息，保存在每个节点内部的一个队列中，节点保存的
             是当前正在上传的文件的信息

  函数列表   :
  修改历史   :

******************************************************************************/

#ifndef __UPLOAD_FILE_H__
#define __UPLOAD_FILE_H__

#include <stdio.h>
#include "FLXCommon/flxnettypes.h"
#include "util_queue.h"

struct ul_file_info
{
	unsigned char  protocol_file_type;      //当前正在传输的文件类型
	char          *filename;                //客户端提交的文件名
	char          *filename_tx;             //传输时使用的文件名
};

extern void delete_upload_file_node(FLXSocket sock);
extern int add_upload_file_node(FLXSocket sock, unsigned char  protocol_file_type, const char *filename);

extern FILE *get_upload_file_handle(FLXSocket sock);
extern int clear_single_upload(FLXSocket sock);

extern const char *get_upload_filename(FLXSocket sock);
extern const char *get_upload_filename_tx(FLXSocket sock);
extern UTIL_QUEUE *get_upload_file_history(FLXSocket sock);


#endif /* __UPLOAD_FILE_H__ */
