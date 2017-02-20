/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : protocol_adapter.c
  �������� : 2010-12-22

  �汾    : 1.0
  �������� : ���ݽ����������

  �޸���ʷ : ���Ӹն���޸�

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

/* TODO:֪ͨ�ͻ��˸�����Դ�ļ� */
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
 * ֹͣ���أ���ͻ��˴����ļ��������ָ��
 * ���Ҳ�������ع��̳�ͻ��send_reply_msg�Ժ���Ҫ�޸�Ϊ
 * ���Ͷ�����Ӽ�¼
 * �����л���Ҫһ���ļ�����........................
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
		/* ֹͣ���е��������� */
		fd_download_stop_all(callback_notify_pad_stop_download);
	}

	/*
		case PROTOCOL_FILE_TYPE_SO
		case PROTOCOL_FILE_TYPE_LUA
	*/
	if((file_type_mask & 0x2) || (file_type_mask & 0x4))
	{
		/* ж�ض�̬�� */
		if(!release_ui_library())
		{
			CCC_LOG_OUT("%s : release_ui_library error\n", __FUNCTION__);
			return 0;
		}
	}

	/* PROTOCOL_FILE_TYPE_IRDA */
	if(file_type_mask & 0x8)
	{
		/* ֱ�ӹرձ���������ݵ����ݿ�
		 * ��һ�ζ�ȡ��������ʱ�������⵽���ݿ��ѹرգ�
		 * ���Զ���
		 */
		close_infrared_file();
	}
	return 1;
}
static int after_update_from_upload(unsigned int file_type_mask)
{
	if((file_type_mask & 0x2) || (file_type_mask & 0x4))
	{
		/* �豸�ӿ� */
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

/* �յ�Ҫ�ϴ��ļ����ͺ��ļ��� */
static int process_protocol_upload_file_cmd_ex_01(FLXSocket sock, PDU *pdu)
{
	int send_len;
	unsigned char send_data[8];
	int b_succeeded = 0;
	unsigned char protocol_file_type = 0x0; /* ���ʱ�򻹲�֪�����ͣ���ֵΪ0 */

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

/* �յ��ϴ����� */
static int process_protocol_upload_file_cmd_ex_02(FLXSocket sock, PDU *pdu)
{
	int send_len;
	unsigned char send_data[8];
	int b_succeeded = 0;
	unsigned char protocol_file_type = 0x0; /* ���ʱ�򻹲�֪�����ͣ���ֵΪ0 */

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

/* �����ļ��ϴ���� */
static int process_protocol_upload_file_cmd_ex_03(FLXSocket sock, PDU *pdu)
{
	int send_len;
	unsigned char send_data[8];
	int b_succeeded = 0;
	unsigned char protocol_file_type = 0x0; /* ���ʱ�򻹲�֪�����ͣ���ֵΪ0 */

	if(pdu->data_len >= 2)
	{
		protocol_file_type = pdu->data[1];
		fd_upload_single_file_finish(sock);
		if(PCS_EX_OK == pdu->data[0])   /* �����ļ�ʧ�ܣ�Ӧ��ɾ������ļ��ļ�¼ */
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
/* �����ļ��ϴ���� */
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

	/* �ϴ������󣬽���һЩ���� */
	if(b_succeeded)
	{
		unsigned int file_type_mask = fd_upload_all_file_finish(sock);
		/*
		 * ����ͣ  ����Ҫֹͣ���е����ع���
		 * Ȼ������ļ�
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

/* 0x11������ϴ��ļ�  CLIENT_INFO_STRU *clientInfo */
static int process_protocol_upload_file(FLXSocket sock, PDU *pdu)
{
	switch(pdu->cmd_ex)
	{
	case 0x01 :return process_protocol_upload_file_cmd_ex_01(sock, pdu); /* �յ�Ҫ�ϴ��ļ����ͺ��ļ��� */
	case 0x02 :return process_protocol_upload_file_cmd_ex_02(sock, pdu); /* �յ��ϴ����� */
	case 0x03 :return process_protocol_upload_file_cmd_ex_03(sock, pdu);
	case 0x04 :return process_protocol_upload_file_cmd_ex_04(sock, pdu);
	}
	return 0;
}

/*
 * ����������
 * PACKAGE_SIZE
 */
static int process_protocol_download_file_cmd_ex_02(FLXSocket sock, PDU *pdu)
{
	int len, send_len;
	unsigned char send_data[PROTOCOL_DATA_LEN_MAX];
	unsigned char protocol_file_type = 0x0; /* ���ʱ�򻹲�֪�����ͣ���ֵΪ0 */

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
		if(0 == len)   /* �����Ѿ������� */
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
 * �ͻ��������ļ��汾��Ϣ
 */
static int process_protocol_download_file_cmd_ex_04(FLXSocket sock, PDU *pdu)
{
	int send_len;
	unsigned char send_data[PROTOCOL_DATA_LEN_MAX];
	int b_succeeded = 0;
	unsigned char protocol_file_type = 0x0; /* ���ʱ�򻹲�֪�����ͣ���ֵΪ0 */

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
 * �ͻ������������ļ�
 */
static int process_protocol_download_file_cmd_ex_01(FLXSocket sock, PDU *pdu)
{
	int send_len;
	unsigned char send_data[PROTOCOL_DATA_LEN_MAX];
	int b_succeeded = 0;
	unsigned char protocol_file_type = 0x0;      /* ���ʱ�򻹲�֪�����ͣ���ֵΪ0 */

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

			/* �ļ����� ռ4�ֽ�*/
			send_data[1] = (file_len >> 24) & 0xFF;
			send_data[2] = (file_len >> 16) & 0xFF;
			send_data[3] = (file_len >> 8)  & 0xFF;
			send_data[4] =  file_len & 0xFF;
			send_len += 4;

			/* �ļ��� */
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

/* 0x12 �����ļ� */
static int process_protocol_download_file(FLXSocket sock, PDU *pdu)
{
	switch(pdu->cmd_ex)
	{
	case 0x01 :return process_protocol_download_file_cmd_ex_01(sock, pdu); /* �ͻ������������ļ� */
	case 0x02 :return process_protocol_download_file_cmd_ex_02(sock, pdu); /* ���������� */
	case 0x04 :return process_protocol_download_file_cmd_ex_04(sock, pdu); /* �ͻ��������ļ��汾��Ϣ */
	}
	return 0;
}

#if 0
/* �̴߳��ݲ��� */
typedef struct UIE
{
	FLXInt32  senderId;
	FLXInt32  event;
	FLXByte   data[PACKAGE_SIZE];
	FLXInt32  dataLen;
	FLXBool   rev;       // ��¼�̲߳��������Ƿ�ɹ�
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
	puie_param->rev = TRUE;                              /* �̲߳������ݴ������ */

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
	/* �ؼ�id */
	senderId  = four_bytes_to_int(pdu->data + 4, 4);

	uie_param.event = event;
	uie_param.senderId = senderId;
	memcpy(uie_param.data, pdu->data, pdu->data_len);
	uie_param.dataLen = pdu->data_len;
	uie_param.rev = FALSE;

	iRet = thread_create(&pid, NULL, (void *)parse_control_event_thread_func, &uie_param);
	if (iRet == 0)
	{
		while (uie_param.rev == FALSE); //�ȴ��������ݸ��߳�
	}
	else
	{
		MSG_OUT("thread create error\n");
			return 0;
	}
	return 1;
}
#endif

/* 0x19 �¼����� */
static int process_protocol_client_event(PDU *pdu)
{
	FLXInt32 senderId, event;
	if(pdu->cmd_ex != 0x03)
		return 0;
	if(pdu->data_len < 8)
		return 0;

	/* event */
	event = four_bytes_to_int(pdu->data, 4);
	/* �ؼ�id */
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
/* 0x21��ȡ��������Ϣ */
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
	case PCS_UPLOAD_FILE   :return process_protocol_upload_file(clientInfo->sock, pdu);     /* 0x11 ������ϴ��ļ� */
	case PCS_DOWNLOAD_FILE :return process_protocol_download_file(clientInfo->sock, pdu);   /* 0x12 �����ļ� */
	case PCS_CLIENT_EVENT  :return process_protocol_client_event(pdu);                      /* 0x19 �¼����� */
	case PCS_SERVER_INFO   :return process_protocol_server_info(clientInfo->sock, pdu);     /* 0x21 ��ȡ��������Ϣ*/
	default:
		printf("This data packet is bad: unknown command\n");
		break;
	}
	return 1;
}

/* �����Կͻ��˵�����ת���������豸 */
int server_relay_client_data(int server_type, PDU *pdu)
{
	int iDataLen, ret;
	unsigned char pbyRevDataTemp[PROTO_DATA_LEN_MAX];

	assemble_protocol_packet(pdu, pbyRevDataTemp, &iDataLen);
	switch(server_type)
	{
	case SERVER_TYPE_SLAVE: /* �ӻ� */
		/* ������������ת��һ�� */
		ret = slave_relay_data_to_master(pbyRevDataTemp, iDataLen);
		break;

	case SERVER_TYPE_MASTER: /* ���� */
		{
			/* �����������дӻ�ת��һ�� */
			send_data_to_all_slave(pbyRevDataTemp, iDataLen);
		}
		break;
	}
	return 1;
}
/* ����������һ̨�ӻ�������ת���������ӻ� */
int relay_slave_data_to_other_slave(FLXSocket slaveClientSock, PDU *pdu)
{
	int iDataLen;
	unsigned char pbyRevDataTemp[PROTO_DATA_LEN_MAX];
	assemble_protocol_packet(pdu, pbyRevDataTemp, &iDataLen);

	send_data_to_other_slave(slaveClientSock, pbyRevDataTemp, iDataLen);
	return 1;
}
/*
 * �����������Դӻ�����Ϣ
 * ��Щ��Ϣ�������ڴӻ��������ƽ�壬�ɴӻ�ת��������
 */
static int proa_analyse_data_from_slave_server(FLXSocket slaveClientSock, PDU *pdu)
{
	if(pdu->cmd != PCS_CLIENT_EVENT) /* ֻ���� 0x19 �ͻ����¼� */
		return 0;

	relay_slave_data_to_other_slave(slaveClientSock, pdu);
	return process_protocol_pdu(NULL, pdu);
}
/*
 * �ӻ���Ϊ�ͻ��˴������������յ�������
 * Ŀǰ��Ҫ�ǿؼ��¼�
 */
static int proa_analyse_data_from_slave_client(PDU *pdu)
{
	if(pdu->cmd != PCS_CLIENT_EVENT) /* ֻ���� 0x19 �ͻ����¼� */
		return 0;

	return process_protocol_pdu(NULL, pdu);
}

//��������pad����������ȿͻ���
static int proa_analyse_data_from_pad_or_0(CLIENT_INFO_STRU *clientInfo, PDU *pdu)
{
	if(PCS_CLIENT_EVENT == pdu->cmd) /* 0x19 ����ת���������豸 */
	{
		server_relay_client_data(g_server_type, pdu);
	}

	return process_protocol_pdu(clientInfo, pdu);
}

/*
 * �������ȡ���ݵĻص�����
 * ��Щ���ݶ���Э������
 *
 * ������
 *   data_source ��ʶ������Դ
 *   *data       ���ݻ���
 *   *data_len   ���ݵĳ��ȣ��ֽ�Ϊ��λ�����ϲ㴦�������ʣ�����ݵĳ���
 *   clientInfo  �ͻ�����Ϣ
 */
void callback_recv_data_from_network(int data_source, unsigned char *buffer, int *data_len, CLIENT_INFO_STRU *clientInfo)
{
	PDU pdu;
	if(!parse_packet(buffer, data_len, &pdu))
		return;
	//print_pdu_data(clientInfo->sock, &pdu);

	switch(data_source)
	{
	case DATA_SOURCE_PAD_OR_0://��������pad����������ȿͻ���
		proa_analyse_data_from_pad_or_0(clientInfo, &pdu);
		break;

	case DATA_SOURCE_SLAVE_SERVER://�������Դӻ�
		proa_analyse_data_from_slave_server(clientInfo->sock, &pdu);
		break;
	case DATA_SOURCE_SLAVE_CLIENT://�������Դӻ��ϵĿͻ�������
		proa_analyse_data_from_slave_client(&pdu);
		break;
	}
}

/*
 * �Ӵ��ڣ�1-8����ȡ���ݵĻص�����
 *
 * ������
 *   serial_no   ���ڱ�ţ�ȡֵ��1 - 8
 *   *data       ���ݻ���
 *   *data_len   ���ݵĳ��ȣ��ֽ�Ϊ��λ�����ϲ㴦�������ʣ�����ݵĳ���
 *
 */
void callback_recv_data_from_serial(int serial_no, unsigned char *buffer, int size)
{
	printf("<s%d len %d\n", serial_no, size);
	parse_serial_recv(serial_no, buffer, size);
}


/*
 * �ص�����
 * Ϊ���Ӵ������ݻ���ڵ㡢ɾ������ڵ�
 * �����ӻ�ȡ����
 *
 * Ŀǰֻ��telnet����
 */
void callback_by_connect_manager(int reason, int connect_type, int connect_no, unsigned char *buffer, int data_len)
{
	switch(reason)
	{
		/* �������յ������� */
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

		/* ɾ�����ӵ����ݻ��� */
	case CMC_REASON_CONNECT_DELETE:
		data_pool_delete_node(connect_type, connect_no);
		break;

		/* Ϊ���ӷ���һ�����ݻ��� */
	case CMC_REASON_CONNECT_ADD:
		data_pool_add_node(connect_type, connect_no);
		break;
	}
}


/*
 * �ص�����
 * �������ݣ��������ݳ�
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
