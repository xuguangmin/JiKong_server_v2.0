/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : devi_proto_data.c
  生成日期 : 2013-11-21

  版本    : 1.0
  功能描述 : 处理来自外部脚本传入的数据

  修改历史 :

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util_log.h"
#include "util_func.h"
#include "configure.h"
#include "protocol/protocol.h"
#include "device_controller/device_manager.h"
#include "device_interface/devi_protocol.h"
#include "tcp_server/tps_tcp_server.h"
#include "tcp_server/connect_manager.h"

#include "data_pool/ccc_data_pool.h"
/*
 * telnet连接函数
 */
static int add_comm_telnet(int connect_no, const char *ip_address, int port)
{
	return connect_manager_add_telnet(connect_no, ip_address, port);
}
static int delete_comm_telnet(int connect_no)
{
	return connect_manager_delete(CONNECT_TYPE_TELNET, connect_no);
}
static int send_data_to_comm_telnet(int connect_no, const unsigned char *buffer, int data_len)
{
	return data_pool_save_send_data_telnet(connect_no, buffer, data_len);
}

static int send_devi_proto_data_to_serial(int cmd_ex, struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	int serial_no = lp_devi_proto_data->data1;
	switch(cmd_ex)
	{
	case 0x01 :
		{
			int baud_rate = lp_devi_proto_data->data2;
			int data_bits = lp_devi_proto_data->data3;
			int parity    = lp_devi_proto_data->data4;
			int stop_bits = lp_devi_proto_data->data5;

			//printf("%d %d %d-%d-%d\n", serial_no, baud_rate, data_bits, parity, stop_bits);
			return comm_serial_modify_config(serial_no, baud_rate, data_bits, parity, stop_bits);
		}
		break;
	case 0x02 :return comm_serial_stop(serial_no);
	case 0x03 :
		if(lp_devi_proto_data->uchar_buffer && lp_devi_proto_data->data_len > 1)
		{
			return send_data_to_comm_serial(serial_no, lp_devi_proto_data->uchar_buffer, lp_devi_proto_data->data_len);
		}
		break;
	}
	return 0;
}

static int send_data_to_infrared(short infrared_no, int key)
{
	int len;
	char *irdaData = NULL;
	unsigned char *buffer;
	if(get_irda_data(key, &irdaData) <= 0 || !irdaData || strlen(irdaData) <= 0)
	{
		CCC_LOG_OUT("%s :IR%d key %d no data.\n", __FUNCTION__, infrared_no, key);
		return 0;
	}

	len = protocol_infrared_transfer(irdaData, &buffer);
	if(irdaData) free(irdaData);
	if(len <= 0)
		return 0;

	printf(">IR%d, key %d\n", infrared_no, key);
	send_data_to_comm_infrared(infrared_no, buffer, len);
	if(buffer) free(buffer);
	return 1;
}

static int send_devi_proto_data_to_infrared(int cmd_ex, struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	switch(cmd_ex)
	{
	case 0x01 :
		if(lp_devi_proto_data)
		{
			int infrared_no = lp_devi_proto_data->data1;
			int key         = lp_devi_proto_data->data2;
			return send_data_to_infrared((short)infrared_no, key);
		}
		break;
	}
	return 0;
}

static int send_devi_proto_data_to_wol(int cmd_ex, struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	switch(cmd_ex)
	{
	case 0x01 :
		if(lp_devi_proto_data->char_buffer)
		{
			return network_manager_wol(lp_devi_proto_data->char_buffer);
		}
		break;
	}
	return 0;
}

static int send_devi_proto_data_to_relay(int cmd_ex, struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	switch(cmd_ex)
	{
	case 0x01 :
		if(lp_devi_proto_data)
		{
			int relay_no = lp_devi_proto_data->data1;
			int b_status = lp_devi_proto_data->data2;
			return send_data_to_comm_relay(relay_no, b_status);
		}
		break;
	}
	return 0;
}

static unsigned int create_protocol_packet_ctrl_property(unsigned char *out_buffer, unsigned int buffer_size,
		                                                 int ctrlId, int property, int value)
{
	unsigned char valid_data[16];
	if(!out_buffer || buffer_size < (9 + 12))
		return 0;

	/* 控件id */
	int_to_four_bytes(ctrlId, valid_data, 4);
	/* 控件属性码 */
	int_to_four_bytes(property, &valid_data[4], 4);
	/* 控件属性值 */
	int_to_four_bytes(value, &valid_data[8], 4);
	return create_protocol_packet(PCS_CTRL_PROPERTY, 0x11, valid_data, 12, out_buffer, buffer_size);
}

static int set_ui_ctrl_property(struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	unsigned int packet_size;
	unsigned char out_buffer[PROTOCOL_DATA_SIZE_MAX];
	if(!lp_devi_proto_data)
		return 0;

	packet_size = create_protocol_packet_ctrl_property(out_buffer, PROTOCOL_DATA_SIZE_MAX,
			                             lp_devi_proto_data->data1, lp_devi_proto_data->data2, lp_devi_proto_data->data3);
	if(packet_size <= 0)
		return 0;

	send_data_to_all_pad(out_buffer, packet_size);
	return 1;
}
static int send_devi_proto_data_to_ctrl_propery(int cmd_ex, struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	switch(cmd_ex)
	{
	case 0x01 :return set_ui_ctrl_property(lp_devi_proto_data);
	}
	return 0;
}

static int send_devi_proto_data_to_http(int cmd_ex, struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	switch(cmd_ex)
	{
	case 0x01 :
		if(lp_devi_proto_data->char_buffer)
		{
			return network_manager_send_url(lp_devi_proto_data->char_buffer);
		}
		break;
	}
	return 0;
}
static int send_devi_proto_data_to_onvif(int cmd_ex, struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	switch(cmd_ex)
	{
	case 0x01 :
		if(lp_devi_proto_data->char_buffer && lp_devi_proto_data->uchar_buffer)
		{
			return network_manager_send_onvif(lp_devi_proto_data->char_buffer, (char *)lp_devi_proto_data->uchar_buffer);
		}
		break;
	}
	return 0;
}

static int send_devi_proto_data_to_telnet(int cmd_ex, struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	int telnetId = lp_devi_proto_data->data1;
	switch(cmd_ex)
	{
	case 0x01 :  /* 增加telnet连接 */
		{
			int port = lp_devi_proto_data->data2;
			const char *ip = lp_devi_proto_data->char_buffer;

			return add_comm_telnet(telnetId, ip, port);
		}
		break;
	case 0x02 :  /* 关闭telnet连接 */
		{
			//int id = get_telnet_info_node_id(telnetId);
			delete_comm_telnet(telnetId);
			//delete_telnet_info_node(telnetId);
			return 1;
		}
		break;

	case 0x03 :  /* 发送数据 */
		if(lp_devi_proto_data->uchar_buffer && lp_devi_proto_data->data_len > 1)
		{
			//int id = get_telnet_info_node_id(telnetId);
			//printf("%s, id %d\n", __FUNCTION__, id);
			return send_data_to_comm_telnet(telnetId, lp_devi_proto_data->uchar_buffer, lp_devi_proto_data->data_len);
		}
		break;
	}
	return 0;
}

int devi_proto_data(int cmd, int cmd_ex, struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	if(!lp_devi_proto_data)
		return 0;
	if(lp_devi_proto_data->id_code != DEVI_PROTO_DATA_ID_CODE)
	{
		CCC_LOG_OUT("%s : libhmui_event.so is mismatch.\n", __FUNCTION__);
		return 0;
	}

	switch(cmd)
	{
	case DEVI_PCS_SERIAL        :return send_devi_proto_data_to_serial      (cmd_ex, lp_devi_proto_data);
	case DEVI_PCS_INFRARED      :return send_devi_proto_data_to_infrared    (cmd_ex, lp_devi_proto_data);
	case DEVI_PCS_RELAY         :return send_devi_proto_data_to_relay       (cmd_ex, lp_devi_proto_data);
	case DEVI_PCS_CTRL_PROPERTY :return send_devi_proto_data_to_ctrl_propery(cmd_ex, lp_devi_proto_data);
	case DEVI_PCS_HTTP          :return send_devi_proto_data_to_http        (cmd_ex, lp_devi_proto_data);
	case DEVI_PCS_TELNET        :return send_devi_proto_data_to_telnet      (cmd_ex, lp_devi_proto_data);
	case DEVI_PCS_WOL           :return send_devi_proto_data_to_wol         (cmd_ex, lp_devi_proto_data);
	case DEVI_PCS_ONVIF         :return send_devi_proto_data_to_onvif       (cmd_ex, lp_devi_proto_data);
	}
	return 0;
}
