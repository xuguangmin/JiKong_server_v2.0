/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文件名   ：device_interface.c
  创建者   ：贾延刚
  生成日期 ：2012-11-1
  功能描述 : 提供了对设备操作的接口，将编译为一个静态库。

           该静态库提供给设计器使用，会被链接到一个包含了lua脚本解析器的动态库中，
           做为操作设备的接口。

           该静态库的功能只是把从接口上获取的数据，转发到主程序提供的函数接口
           send_devi_proto_data中，不做任何处理。

******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>

#include "devi_protocol.h"
#include "protocol_adapter/protocol_adapter.h"


#define DEVI_TRUE            0
#define DEVI_FALSE          -1

/* 来自文件 : protocol_define.h */
#define DEVI_CTRL_PROPERTY_ENABLE            0x01            // 是否可用
#define DEVI_CTRL_PROPERTY_VISUAL            0x02            // 是否可见
#define DEVI_CTRL_PROPERTY_CHANGE            0x03            // 一个值（是不断变化的）
#define DEVI_CTRL_PROPERTY_CHECK             0x04            // 是否选中

static void devi_init_proto_data_struct(struct DEVI_PROTO_DATA *lp_devi_proto_data)
{
	if(!lp_devi_proto_data)
		return;

	lp_devi_proto_data->id_code      = DEVI_PROTO_DATA_ID_CODE;

	lp_devi_proto_data->data1        = 0;
	lp_devi_proto_data->data2        = 0;
	lp_devi_proto_data->data3        = 0;
	lp_devi_proto_data->data4        = 0;
	lp_devi_proto_data->data5        = 0;

	lp_devi_proto_data->char_buffer  = (void *)0;
	lp_devi_proto_data->uchar_buffer = (void *)0;
	lp_devi_proto_data->data_len     = 0;
}

int uie_sleep(int mesc)
{
	struct timeval delay;
	delay.tv_sec = 0;
	delay.tv_usec = mesc * 1000;
	select(0, NULL, NULL, NULL, &delay);
	return DEVI_TRUE;
}

/*  ------ serial  ------ */
/*
 * 0x01 指令
 * 0x01 初始化串口
 */
int devi_init_serial_port(int serial_no, int iRate, int dataBit, int stopBit, int parity)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	//printf("%s serial_no %d\n", __FUNCTION__, serial_no);

	devi_proto_data.data1  = serial_no;
	devi_proto_data.data2  = iRate;
	devi_proto_data.data3  = dataBit;
	devi_proto_data.data4  = parity;
	devi_proto_data.data5  = stopBit;
	return send_devi_proto_data(DEVI_PCS_SERIAL, 0x01, &devi_proto_data) ? DEVI_TRUE:DEVI_FALSE;
}
/*
 * 0x01 指令
 * 0x02 关闭串口
 */
int devi_release_serial_port(int serial_no)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	devi_proto_data.data1  = serial_no;
	return send_devi_proto_data(DEVI_PCS_SERIAL, 0x02, &devi_proto_data) ? DEVI_TRUE:DEVI_FALSE;
}
/*
 * 0x01 指令
 * 0x03 发送数据
 */
int devi_serial_port_write_data(int serial_no, unsigned char *pcData, int iLen)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	//printf("%s serial_no %d\n", __FUNCTION__, serial_no);

	devi_proto_data.data1        = serial_no;
	devi_proto_data.uchar_buffer = pcData;
	devi_proto_data.data_len     = iLen;
	return send_devi_proto_data(DEVI_PCS_SERIAL, 0x03, &devi_proto_data) ? DEVI_TRUE:DEVI_FALSE;
}


/*  ------ infrared  ------  */
/*
 * 0x02 指令
 * 0x01 发送红外数据
 */
int devi_irda_write_data(short infrared_no, int key)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	devi_proto_data.data1  = infrared_no;
	devi_proto_data.data2  = key;
	return send_devi_proto_data(DEVI_PCS_INFRARED, 0x01, &devi_proto_data) ? DEVI_TRUE:DEVI_FALSE;
}


/*  ------ wol  ------*/
/*
 * 0x03 指令
 * 0x01 发送继电数据
 */
int devi_wol_write_data(const char *mac_addr, const unsigned char *password, int password_len)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	devi_proto_data.char_buffer  = mac_addr;
	devi_proto_data.uchar_buffer = password;
	devi_proto_data.data_len     = password_len;
	return send_devi_proto_data(DEVI_PCS_WOL, 0x01, &devi_proto_data) ? DEVI_TRUE:DEVI_FALSE;
}

/*  ------ relay  ------*/
/*
 * 0x03 指令
 * 0x01 发送继电数据
 */
int devi_relay_write_data(int relay_no, int b_status)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	devi_proto_data.data1  = relay_no;
	devi_proto_data.data2  = b_status;
	return send_devi_proto_data(DEVI_PCS_RELAY, 0x01, &devi_proto_data) ? DEVI_TRUE:DEVI_FALSE;
}

/* ------  转发某个客户端产生的按钮状态消息给所有客户端  ------ */
/*
 * 0x04 指令
 * 0x01 控件属性
 */
static int devi_set_ui_ctrl_property(int ctrlId, int property, int value)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	devi_proto_data.data1  = ctrlId;
	devi_proto_data.data2  = property;
	devi_proto_data.data3  = value;
	return send_devi_proto_data(DEVI_PCS_CTRL_PROPERTY, 0x01, &devi_proto_data);
}
/* 控件使能 */
int uie_ctrl_enable(int ctrlId)   {return devi_set_ui_ctrl_property(ctrlId, DEVI_CTRL_PROPERTY_ENABLE, 0x01) ? DEVI_TRUE:DEVI_FALSE;}
/* 控件禁用 */
int uie_ctrl_disable(int ctrlId)  {return devi_set_ui_ctrl_property(ctrlId, DEVI_CTRL_PROPERTY_ENABLE, 0x00) ? DEVI_TRUE:DEVI_FALSE;}
/* 使控件可见 */
int uie_ctrl_visual(int ctrlId)   {return devi_set_ui_ctrl_property(ctrlId, DEVI_CTRL_PROPERTY_VISUAL, 0x01) ? DEVI_TRUE:DEVI_FALSE;}
/* 使控件不可见 */
int uie_ctrl_unvisual(int ctrlId) {return devi_set_ui_ctrl_property(ctrlId, DEVI_CTRL_PROPERTY_VISUAL, 0x00) ? DEVI_TRUE:DEVI_FALSE;}

/* 给控件一个值
 * 目前用于滑动条和进度条设置进度值
 */
int uie_ctrl_set_value_int(int ctrlId, int change)
{
	return devi_set_ui_ctrl_property(ctrlId, DEVI_CTRL_PROPERTY_CHANGE, change) ? DEVI_TRUE:DEVI_FALSE;
}
/* 选中控件
 * 目前用于checkbox和radiobox的选中与否
 */
int uie_ctrl_set_checked(int ctrlId)
{
	return devi_set_ui_ctrl_property(ctrlId, DEVI_CTRL_PROPERTY_CHECK, 0x01) ? DEVI_TRUE:DEVI_FALSE;
}
/* 不选中控件 */
int uie_ctrl_set_not_checked(int ctrlId)
{
	return devi_set_ui_ctrl_property(ctrlId, DEVI_CTRL_PROPERTY_CHECK, 0x00) ? DEVI_TRUE:DEVI_FALSE;
}


/*  ------ network http ------*/
/*
 * 0x05 指令
 * 0x01 发送http url
 */
int devi_network_http(const char *url_http)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	devi_proto_data.char_buffer = url_http;
	return send_devi_proto_data(DEVI_PCS_HTTP, 0x01, &devi_proto_data) ? DEVI_TRUE:DEVI_FALSE;
}

/*  ------ network onvif by http ------*/
/*
 * 0x07 指令
 * 0x01 发送onvif
 */
int devi_network_onvif(const char *uri, const char *body)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	devi_proto_data.char_buffer = uri;
	devi_proto_data.uchar_buffer = (unsigned char *)body;
	return send_devi_proto_data(DEVI_PCS_ONVIF, 0x01, &devi_proto_data) ? DEVI_TRUE:DEVI_FALSE;
}

/*  ------ network telnet ------ */
/*
 * 0x06 指令
 * 0x01 初始化
 */
int devi_init_telnet(int controlId, const char *ip_address, int port)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	devi_proto_data.data1       = controlId;
	devi_proto_data.data2       = port;
	devi_proto_data.char_buffer = ip_address;
	return send_devi_proto_data(DEVI_PCS_TELNET, 0x01, &devi_proto_data) ? DEVI_TRUE:DEVI_FALSE;
}
/*
 * 0x06 指令
 * 0x02 关闭
 */
int devi_release_telnet(int controlId)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	devi_proto_data.data1       = controlId;
	return send_devi_proto_data(DEVI_PCS_TELNET, 0x02, &devi_proto_data) ? DEVI_TRUE:DEVI_FALSE;
}
/*
 * 0x06 指令
 * 0x03 发送数据
 */
int devi_telnet_write_data(int controlId, const unsigned char *data, int data_len)
{
	struct DEVI_PROTO_DATA  devi_proto_data;
	devi_init_proto_data_struct(&devi_proto_data);

	devi_proto_data.data1        = controlId;
	devi_proto_data.uchar_buffer = data;
	devi_proto_data.data_len     = data_len;
	return send_devi_proto_data(DEVI_PCS_TELNET, 0x03, &devi_proto_data) ? DEVI_TRUE:DEVI_FALSE;
}
