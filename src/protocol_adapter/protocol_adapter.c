/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : protocol_adapter.c
  生成日期 : 2010-12-22

  版本    : 1.0
  功能描述 : 数据解析、处理等

  修改历史 : 贾延刚多次修改

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "share_type.h"
#include "util_log.h"
#include "util_func.h"
#include "sysconfig.h"

#include "protocol/protocol.h"
#include "ui_library/ui_library_wrapper.h"
#include "file_deal/fd_file_up_download.h"

#include "data_pool/ccc_data_pool.h"
#include "device_controller/device_manager.h"
#include "device_interface/devi_protocol.h"

#include "tcp_server/tcp_share_type.h"
#include "tcp_server/connect_manager.h"
#include "tcp_server/tps_tcp_server.h"
#include "tcp_server/wol.h"
#include "devi_proto_data.h"


static int g_server_type = -1;

static void print_pdu_data(FLXSocket sock, PDU *pdu)
{
	if(!pdu) return;

	printf("<%d : %02X%02X(%d): ", (int)sock, pdu->cmd, pdu->cmd_ex, pdu->data_len);
	output_byte_array(pdu->data, pdu->data_len, pdu->data_len);//2);
}

#if 0
static void print_send_data(unsigned char cmd, unsigned char cmd_ex, const unsigned char *data, unsigned int data_len)
{
	if(!data) return;

	printf("%02X%02X(%d): ", cmd, cmd_ex, data_len);
	output_byte_array(data, data_len, 2);
}
#endif

static int send_reply_msg(FLXSocket sock, unsigned char cmd, unsigned char cmd_ex, const unsigned char *data, unsigned int data_len)
{
	unsigned char out_buffer[PROTOCOL_DATA_SIZE_MAX];
	unsigned int packet_size;

	packet_size = create_protocol_packet(cmd, cmd_ex, data, data_len, out_buffer, PROTOCOL_DATA_SIZE_MAX);
	if(packet_size <= 0)
		return 0;

	if(!network_manager_send_data(sock, out_buffer, packet_size))
	{
		CCC_LOG_OUT("%d %s failed\n", (int)sock, __FUNCTION__);
		return 0;
	}
	/* TODO:CCC_LOG_OUT(">%d : %02X%02X(%d): ", (int)sock, cmd, cmd_ex, data_len); */
	/* TODO: output_byte_array(data, data_len, 2); */
	return 1;
}

/* TODO:通知客户端更新资源文件 */
static int fd_tell_client_update_res(unsigned char protocol_file_type)
{
	unsigned char out_buffer[PROTOCOL_DATA_SIZE_MAX];
	unsigned int packet_size;
	int send_len;
	unsigned char send_data[8];

	send_data[0] = protocol_file_type;
	send_len = 1;

	packet_size = create_protocol_packet(PCS_DOWNLOAD_FILE, 0x05, send_data, send_len, out_buffer, PROTOCOL_DATA_SIZE_MAX);
	if(packet_size <= 0)
		return 0;

	send_data_to_all_pad(out_buffer, packet_size);
	return 1;
}

/* 0x00 */
static int process_protocol_heart_beat(FLXSocket sock)
{
	return send_reply_msg(sock, PCS_HEART_BEAT, 0X00, NULL, 0);
}
/* 0x10 */
static int process_protocol_client_login(FLXSocket sock, PDU *pdu)
{
	return tps_set_client_type(sock, pdu->cmd_ex);
}

/*
 * 停止下载，向客户端传送文件传输完成指令
 * 这个也许会和下载过程冲突，send_reply_msg以后需要修改为
 * 向发送队列添加记录
 * 参数中还需要一个文件类型........................
 */
void callback_notify_pad_stop_download(FLXSocket sock, unsigned char protocol_file_type)
{
	int send_len;
	unsigned char send_data[8];
	unsigned char out_buffer[PROTOCOL_DATA_SIZE_MAX];
	unsigned int packet_size;

	send_data[0] = PCS_EX_ERR;
	send_data[1] = protocol_file_type;
	send_len = 2;

	packet_size = create_protocol_packet(PCS_DOWNLOAD_FILE, 0x03, send_data, send_len, out_buffer, PROTOCOL_DATA_SIZE_MAX);
	if(packet_size <= 0)
		return;

	CCC_DEBUG_OUT("notify sock %d stop download.\n", (int)sock);
	send_data_to_pad(sock, out_buffer, packet_size);
}

static int before_update_after_upload(unsigned int file_type_mask)
{
	CCC_LOG_OUT("\n");
	CCC_LOG_OUT("upload finished, start to update ......\n");

	/* PROTOCOL_FILE_TYPE_ZIP */
	if((file_type_mask & 0x1)|| (file_type_mask & 0x10))
	{
		/* 停止所有的下载任务 */
		fd_download_stop_all(callback_notify_pad_stop_download);
	}

	/*
		case PROTOCOL_FILE_TYPE_SO
		case PROTOCOL_FILE_TYPE_LUA
	*/
	if((file_type_mask & 0x2) || (file_type_mask & 0x4))
	{
		/* 卸载动态库 */
		if(!release_ui_library())
		{
			CCC_LOG_OUT("%s : release_ui_library error\n", __FUNCTION__);
			return 0;
		}
	}

	/* PROTOCOL_FILE_TYPE_IRDA */
	if(file_type_mask & 0x8)
	{
		/* 直接关闭保存红外数据的数据库
		 * 下一次读取红外数据时，如果检测到数据库已关闭，
		 * 会自动打开
		 */
		close_infrared_file();
	}
	return 1;
}
static int after_update_from_upload(unsigned int file_type_mask)
{
	if((file_type_mask & 0x2) || (file_type_mask & 0x4))
	{
		/* 设备接口 */
		const char *so_file = get_ui_event_so_filename();
		if(!initialize_ui_library(so_file))
		{
			CCC_LOG_OUT("after_update_from_upload : initialize_ui_library error\n");
			return 0;
		}
	}
	/* PROTOCOL_FILE_TYPE_ZIP */
	if((file_type_mask & 0x1)|| (file_type_mask & 0x10))
	{
		fd_tell_client_update_res(0x01);
	}

	CCC_LOG_OUT("update finished, after uploaded!\n\n");
	return 1;
}

/* 收到要上传文件类型和文件名 */
static int process_protocol_upload_file_cmd_ex_01(FLXSocket sock, PDU *pdu)
{
	int send_len;
	unsigned char send_data[8];
	int b_succeeded = 0;
	unsigned char protocol_file_type = 0x0; /* 这个时候还不知道类型，赋值为0 */

	if(pdu->data_len > 1)
	{
		protocol_file_type = pdu->data[0];
		if(fd_upload_create_file(sock, protocol_file_type, &pdu->data[1], pdu->data_len-1))
		{
			b_succeeded = 1;
		}
	}

	send_data[0] = (!b_succeeded) ? PCS_EX_ERR:PCS_EX_OK;
	send_data[1] = protocol_file_type;
	send_len = 2;

	return send_reply_msg(sock, PCS_UPLOAD_FILE, 0x01, send_data, send_len);
}

/* 收到上传数据 */
static int process_protocol_upload_file_cmd_ex_02(FLXSocket sock, PDU *pdu)
{
	int send_len;
	unsigned char send_data[8];
	int b_succeeded = 0;
	unsigned char protocol_file_type = 0x0; /* 这个时候还不知道类型，赋值为0 */

	if(pdu->data_len > 1)
	{
		protocol_file_type = pdu->data[0];
		if(fd_upload_write_file(sock, &pdu->data[1], pdu->data_len-1))
		{
			b_succeeded = 1;
		}
	}

	send_data[0] = (!b_succeeded) ? PCS_EX_ERR:PCS_EX_OK;
	send_data[1] = protocol_file_type;
	send_len = 2;

	return send_reply_msg(sock, PCS_UPLOAD_FILE, 0x02, send_data, send_len);
}

/* 单个文件上传完成 */
static int process_protocol_upload_file_cmd_ex_03(FLXSocket sock, PDU *pdu)
{
	int send_len;
	unsigned char send_data[8];
	int b_succeeded = 0;
	unsigned char protocol_file_type = 0x0; /* 这个时候还不知道类型，赋值为0 */

	if(pdu->data_len >= 2)
	{
		protocol_file_type = pdu->data[1];
		fd_upload_single_file_finish(sock);
		if(PCS_EX_OK == pdu->data[0])   /* 单个文件失败，应该删除这个文件的记录 */
		{
			b_succeeded = 1;
		}
		else{
			printf("process_protocol_upload_file_cmd_ex_03 failed\n");
		}
	}

	send_data[0] = (!b_succeeded) ? PCS_EX_ERR:PCS_EX_OK;
	send_data[1] = protocol_file_type;
	send_len = 2;

	return send_reply_msg(sock, PCS_UPLOAD_FILE, 0x03, send_data, send_len);
}
/* 所有文件上传完成 */
static int process_protocol_upload_file_cmd_ex_04(FLXSocket sock, PDU *pdu)
{
	int result = 0;
	unsigned char send_data[8];
	int b_succeeded = 0;

	if(pdu->data_len >= 1)
	{
		if(PCS_EX_OK == pdu->data[0])
			b_succeeded = 1;
	}
	send_data[0] = (!b_succeeded) ? PCS_EX_ERR:PCS_EX_OK;
	result = send_reply_msg(sock, PCS_UPLOAD_FILE, 0x04, send_data, 1);

	/* 上传结束后，进行一些处理 */
	if(b_succeeded)
	{
		unsigned int file_type_mask = fd_upload_all_file_finish(sock);
		/*
		 * 先暂停  还需要停止所有的下载过程
		 * 然后更新文件
		 */
		before_update_after_upload(file_type_mask);
		update_config_file();
		after_update_from_upload(file_type_mask);
	}
	else{
		delete_upload_file();
	}

	return result;
}

/* 0x11设计器上传文件  CLIENT_INFO_STRU *clientInfo */
static int process_protocol_upload_file(FLXSocket sock, PDU *pdu)
{
	switch(pdu->cmd_ex)
	{
	case 0x01 :return process_protocol_upload_file_cmd_ex_01(sock, pdu); /* 收到要上传文件类型和文件名 */
	case 0x02 :return process_protocol_upload_file_cmd_ex_02(sock, pdu); /* 收到上传数据 */
	case 0x03 :return process_protocol_upload_file_cmd_ex_03(sock, pdu);
	case 0x04 :return process_protocol_upload_file_cmd_ex_04(sock, pdu);
	}
	return 0;
}

/*
 * 请求发送数据
 * PACKAGE_SIZE
 */
static int process_protocol_download_file_cmd_ex_02(FLXSocket sock, PDU *pdu)
{
	int len, send_len;
	unsigned char send_data[PROTOCOL_DATA_LEN_MAX];
	unsigned char protocol_file_type = 0x0; /* 这个时候还不知道类型，赋值为0 */

	do{
		if(pdu->data_len < 1)
			break;

		protocol_file_type = pdu->data[0];
		if(pdu->data_len >= 2)
		{
			if(pdu->data[0] != PCS_EX_OK)
				break;

			protocol_file_type = pdu->data[1];
		}

		send_data[0] = protocol_file_type;
		send_len = 1;

		len = fd_download_get_data(sock, &send_data[1], PROTOCOL_DATA_LEN_MAX-1);
		if(0 == len)   /* 数据已经发送完 */
		{
			send_data[0] = PCS_EX_OK;
			send_data[1] = protocol_file_type;
			send_len = 2;

			return send_reply_msg(sock, PCS_DOWNLOAD_FILE, 0x03, send_data, send_len);
		}
		else if(len > 0)
		{
			send_len += len;
			return send_reply_msg(sock, PCS_DOWNLOAD_FILE, 0x02, send_data, send_len);
		}
	}while(0);


	send_data[0] = PCS_EX_ERR;
	send_data[1] = protocol_file_type;
	send_len = 2;
	if(!send_reply_msg(sock, PCS_DOWNLOAD_FILE, 0x02, send_data, send_len))
		return 0;
	if(!send_reply_msg(sock, PCS_DOWNLOAD_FILE, 0x03, send_data, send_len))
		return 0;

	return 1;
}

/*
 * 客户端请求文件版本信息
 */
static int process_protocol_download_file_cmd_ex_04(FLXSocket sock, PDU *pdu)
{
	int send_len;
	unsigned char send_data[PROTOCOL_DATA_LEN_MAX];
	int b_succeeded = 0;
	unsigned char protocol_file_type = 0x0; /* 这个时候还不知道类型，赋值为0 */

	if(pdu->data_len >= 1)
	{
		const char *res_filename;
		char fileVersionBuf[256];
		protocol_file_type = pdu->data[0];

		send_data[0] = protocol_file_type;
		send_len = 1;

		res_filename = get_res_filename(protocol_file_type);
		if(fd_download_check_file_version(res_filename, fileVersionBuf, 255))
		{
			int len = strlen(fileVersionBuf);
			memcpy(&send_data[1], fileVersionBuf, len);
			send_len += len;

			b_succeeded = 1;
			printf("file version = %s  length =%d.\n",fileVersionBuf, strlen(fileVersionBuf));
		}
	}

	if(!b_succeeded)
	{
		send_data[0] = PCS_EX_ERR;
		send_data[1] = protocol_file_type;
		send_len = 2;
	}

	return send_reply_msg(sock, PCS_DOWNLOAD_FILE, 0x04, send_data, send_len);
}

/*
 * 客户端请求下载文件
 */
static int process_protocol_download_file_cmd_ex_01(FLXSocket sock, PDU *pdu)
{
	int send_len;
	unsigned char send_data[PROTOCOL_DATA_LEN_MAX];
	int b_succeeded = 0;
	unsigned char protocol_file_type = 0x0;      /* 这个时候还不知道类型，赋值为0 */

	if(pdu->data_len >= 1)
	{
		long int file_len;
		const char *res_filename;
		protocol_file_type = pdu->data[0];

		send_data[0] = protocol_file_type;
		send_len = 1;

		res_filename = get_res_filename(protocol_file_type);
		if(fd_download_open_file(sock, res_filename, &file_len))
		{
			int filename_len;

			/* 文件长度 占4字节*/
			send_data[1] = (file_len >> 24) & 0xFF;
			send_data[2] = (file_len >> 16) & 0xFF;
			send_data[3] = (file_len >> 8)  & 0xFF;
			send_data[4] =  file_len & 0xFF;
			send_len += 4;

			/* 文件名 */
			filename_len = strlen(res_filename);
			memcpy(&send_data[5], res_filename, filename_len);
			send_len += filename_len;

			b_succeeded = 1;
		}
	}

	if(!b_succeeded)
	{
		send_data[0] = PCS_EX_ERR;
		send_data[1] = protocol_file_type;
		send_len = 2;
	}

	return send_reply_msg(sock, PCS_DOWNLOAD_FILE, 0x01, send_data, send_len);
}

/* 0x12 下载文件 */
static int process_protocol_download_file(FLXSocket sock, PDU *pdu)
{
	switch(pdu->cmd_ex)
	{
	case 0x01 :return process_protocol_download_file_cmd_ex_01(sock, pdu); /* 客户端请求下载文件 */
	case 0x02 :return process_protocol_download_file_cmd_ex_02(sock, pdu); /* 请求发送数据 */
	case 0x04 :return process_protocol_download_file_cmd_ex_04(sock, pdu); /* 客户端请求文件版本信息 */
	}
	return 0;
}

#if 0
/* 线程传递参数 */
typedef struct UIE
{
	FLXInt32  senderId;
	FLXInt32  event;
	FLXByte   data[PACKAGE_SIZE];
	FLXInt32  dataLen;
	FLXBool   rev;       // 记录线程参数传递是否成功
}UIE_STRU;

void *parse_control_event_thread_func(void *param)
{
	FLXInt32 dataLen;
	FLXInt32 senderId, event;
	FLXByte data[PACKAGE_SIZE];
	UIE_STRU *puie_param = (UIE_STRU *)param;
	if(!puie_param)
		return NULL;

	senderId = puie_param->senderId;
	event    = puie_param->event;
	dataLen  = puie_param->dataLen;

	memcpy(data, puie_param->data, PACKAGE_SIZE);
	puie_param->rev = TRUE;                              /* 线程参数数据传递完毕 */

	CCC_LOG_OUT("parse_control_event_thread_func senderId %d, event %d\n", senderId, event);

	parse_control_event(senderId, event, (char *)data, dataLen);
	return NULL;
}
static int process_protocol_client_event2(PDU *pdu)
{
	FLXInt32 iRet = 0;
	FLXInt32 senderId, event;
	FLXThread pid;
	UIE_STRU uie_param;

	if(pdu->cmd_ex != 0x03)  // (pbyData[2] != 0x03)
		return 0;
	if(pdu->data_len < 8)
		return 0;

	/* event */
	event = four_bytes_to_int(pdu->data, 4);
	/* 控件id */
	senderId  = four_bytes_to_int(pdu->data + 4, 4);

	uie_param.event = event;
	uie_param.senderId = senderId;
	memcpy(uie_param.data, pdu->data, pdu->data_len);
	uie_param.dataLen = pdu->data_len;
	uie_param.rev = FALSE;

	iRet = thread_create(&pid, NULL, (void *)parse_control_event_thread_func, &uie_param);
	if (iRet == 0)
	{
		while (uie_param.rev == FALSE); //等待参数传递给线程
	}
	else
	{
		MSG_OUT("thread create error\n");
			return 0;
	}
	return 1;
}
#endif

/* 0x19 事件处理 */
static int process_protocol_client_event(PDU *pdu)
{
	FLXInt32 senderId, event;
	if(pdu->cmd_ex != 0x03)
		return 0;
	if(pdu->data_len < 8)
		return 0;

	/* event */
	event = four_bytes_to_int(pdu->data, 4);
	/* 控件id */
	senderId  = four_bytes_to_int(pdu->data + 4, 4);
	//CCC_LOG_OUT("%s senderId %X event %X, pdu->data_len %d\n", __FUNCTION__, senderId, event, pdu->data_len);
	//output_byte_array(pdu->data, pdu->data_len, pdu->data_len);
	return parse_control_event(senderId, event, (char *)pdu->data, pdu->data_len);
}

static void reboot_funcition(void)
{
	close_configure();
	tps_tcp_server_stop();
	reboot_device();
}
/* 0x21获取服务器信息 */
static int process_protocol_server_info(FLXSocket sock, PDU *pdu)
{
	if (pdu->cmd_ex == 0x01)
	{
		char buffer[1024];
		get_cfg_info_string(buffer, 1024);
		if(!send_reply_msg(sock, PCS_SERVER_INFO, 0x01, (unsigned char *)buffer, strlen(buffer)))
			return 0;
	}
	else if (pdu->cmd_ex == 0x02)
	{
		if (!modify_cfg_info_all((char *)pdu->data, pdu->data_len))
			return 0;
	}
	else if (pdu->cmd_ex == 0x03)
	{
		reboot_funcition();
	}
	return 1;
}

static int process_protocol_pdu(CLIENT_INFO_STRU *clientInfo, PDU *pdu)
{
	switch(pdu->cmd)
	{
	case PCS_HEART_BEAT    :return process_protocol_heart_beat(clientInfo->sock);           /* 0x00 */
	case PCS_CLIENT_LOGIN  :return process_protocol_client_login(clientInfo->sock, pdu);    /* 0x10 */
	case PCS_UPLOAD_FILE   :return process_protocol_upload_file(clientInfo->sock, pdu);     /* 0x11 设计器上传文件 */
	case PCS_DOWNLOAD_FILE :return process_protocol_download_file(clientInfo->sock, pdu);   /* 0x12 下载文件 */
	case PCS_CLIENT_EVENT  :return process_protocol_client_event(pdu);                      /* 0x19 事件处理 */
	case PCS_SERVER_INFO   :return process_protocol_server_info(clientInfo->sock, pdu);     /* 0x21 获取服务器信息*/
	default:
		printf("This data packet is bad: unknown command\n");
		break;
	}
	return 1;
}

/* 把来自客户端的数据转发到其他设备 */
int server_relay_client_data(int server_type, PDU *pdu)
{
	int iDataLen, ret;
	unsigned char pbyRevDataTemp[PROTO_DATA_LEN_MAX];

	assemble_protocol_packet(pdu, pbyRevDataTemp, &iDataLen);
	switch(server_type)
	{
	case SERVER_TYPE_SLAVE: /* 从机 */
		/* 把数据往主机转发一份 */
		ret = slave_relay_data_to_master(pbyRevDataTemp, iDataLen);
		break;

	case SERVER_TYPE_MASTER: /* 主机 */
		{
			/* 把数据往所有从机转发一份 */
			send_data_to_all_slave(pbyRevDataTemp, iDataLen);
		}
		break;
	}
	return 1;
}
/* 主机把来自一台从机的数据转发到其他从机 */
int relay_slave_data_to_other_slave(FLXSocket slaveClientSock, PDU *pdu)
{
	int iDataLen;
	unsigned char pbyRevDataTemp[PROTO_DATA_LEN_MAX];
	assemble_protocol_packet(pdu, pbyRevDataTemp, &iDataLen);

	send_data_to_other_slave(slaveClientSock, pbyRevDataTemp, iDataLen);
	return 1;
}
/*
 * 主机处理来自从机的信息
 * 这些信息是来自于从机所服务的平板，由从机转发给主机
 */
static int proa_analyse_data_from_slave_server(FLXSocket slaveClientSock, PDU *pdu)
{
	if(pdu->cmd != PCS_CLIENT_EVENT) /* 只处理 0x19 客户端事件 */
		return 0;

	relay_slave_data_to_other_slave(slaveClientSock, pdu);
	return process_protocol_pdu(NULL, pdu);
}
/*
 * 从机做为客户端从主机服务器收到的数据
 * 目前主要是控件事件
 */
static int proa_analyse_data_from_slave_client(PDU *pdu)
{
	if(pdu->cmd != PCS_CLIENT_EVENT) /* 只处理 0x19 客户端事件 */
		return 0;

	return process_protocol_pdu(NULL, pdu);
}

//数据来自pad或者设计器等客户端
static int proa_analyse_data_from_pad_or_0(CLIENT_INFO_STRU *clientInfo, PDU *pdu)
{
	if(PCS_CLIENT_EVENT == pdu->cmd) /* 0x19 数据转发到其他设备 */
	{
		server_relay_client_data(g_server_type, pdu);
	}

	return process_protocol_pdu(clientInfo, pdu);
}

/*
 * 从网络获取数据的回调函数
 * 这些数据都是协议数据
 *
 * 参数：
 *   data_source 标识数据来源
 *   *data       数据缓存
 *   *data_len   数据的长度（字节为单位），上层处理过后，是剩余数据的长度
 *   clientInfo  客户端信息
 */
void callback_recv_data_from_network(int data_source, unsigned char *buffer, int *data_len, CLIENT_INFO_STRU *clientInfo)
{
	PDU pdu;
	if(!parse_packet(buffer, data_len, &pdu))
		return;
	//print_pdu_data(clientInfo->sock, &pdu);

	switch(data_source)
	{
	case DATA_SOURCE_PAD_OR_0://数据来自pad或者设计器等客户端
		proa_analyse_data_from_pad_or_0(clientInfo, &pdu);
		break;

	case DATA_SOURCE_SLAVE_SERVER://数据来自从机
		proa_analyse_data_from_slave_server(clientInfo->sock, &pdu);
		break;
	case DATA_SOURCE_SLAVE_CLIENT://数据来自从机上的客户端连接
		proa_analyse_data_from_slave_client(&pdu);
		break;
	}
}

/*
 * 从串口（1-8）获取数据的回调函数
 *
 * 参数：
 *   serial_no   串口编号，取值：1 - 8
 *   *data       数据缓存
 *   *data_len   数据的长度（字节为单位），上层处理过后，是剩余数据的长度
 *
 */
void callback_recv_data_from_serial(int serial_no, unsigned char *buffer, int size)
{
	printf("<s%d len %d\n", serial_no, size);
	parse_serial_recv(serial_no, buffer, size);
}


/*
 * 回调函数
 * 为连接创建数据缓存节点、删除缓存节点
 * 从连接获取数据
 *
 * 目前只有telnet连接
 */
void callback_by_connect_manager(int reason, int connect_type, int connect_no, unsigned char *buffer, int data_len)
{
	switch(reason)
	{
		/* 从连接收到的数据 */
	case CMC_REASON_DATA:
		data_pool_save_recv_data(connect_type, connect_no, buffer, data_len);
		break;

		/*
	case CMC_REASON_CONNECT_CLOSE:
		data_buffer_node_set_connect_enable(connect_no, 0);
		break;
	case CMC_REASON_CONNECT_OPEN:
		data_buffer_node_set_connect_enable(connect_no, 1);
		break;
		*/

		/* 删除连接的数据缓存 */
	case CMC_REASON_CONNECT_DELETE:
		data_pool_delete_node(connect_type, connect_no);
		break;

		/* 为连接分配一个数据缓存 */
	case CMC_REASON_CONNECT_ADD:
		data_pool_add_node(connect_type, connect_no);
		break;
	}
}


/*
 * 回调函数
 * 接收数据，来自数据池
 */
void callback_recv_data_from_data_pool(int connect_type, int connect_no, CCCPACKET *dp_packet)
{
	printf("CCCPACKET len %d, %s\n", dp_packet->data_len, dp_packet->buffer);
	cccpacket_not_using(dp_packet);
}














int send_devi_proto_data(int cmd, int cmd_ex, struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	return devi_proto_data(cmd, cmd_ex, lp_devi_proto_data);
}

int protocol_adapter_init(int server_type)
{
	g_server_type = server_type;
	return 1;
}
