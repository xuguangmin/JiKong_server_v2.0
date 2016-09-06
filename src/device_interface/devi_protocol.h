/*
 * devi_protocol.h
 *
 *  Created on: 2012-12-4
 *      Author: flx
 */

#ifndef __DEVI_PROTOCOL_H__
#define __DEVI_PROTOCOL_H__

/*
 * DEVI_PCS_SERIAL   cmd_ex : 0x01 open; 0x02 close; 0x03 data;
 */
#define DEVI_PCS_SERIAL           0x01
#define DEVI_PCS_INFRARED         0x02
#define DEVI_PCS_RELAY            0x03
#define DEVI_PCS_CTRL_PROPERTY    0x04
#define DEVI_PCS_HTTP             0x05
#define DEVI_PCS_TELNET           0x06
#define DEVI_PCS_WOL              0x07
#define DEVI_PCS_ONVIF            0x08

/*
 * 为id_code赋值，目前为1
 * 主程序会检查UI动态库传入的这个值，
 * 以用来确认UI动态库使用的静态库版本是否适用于主程序
 */
#define DEVI_DPD_ID_CODE_1              1                   /* 2012-12-06 */
#define DEVI_PROTO_DATA_ID_CODE         DEVI_DPD_ID_CODE_1
struct DEVI_PROTO_DATA
{
	int            id_code;                  /* 主要用来检查这个结构的成员是否有变化 */

	int            data1;
	int            data2;
	int            data3;
	int            data4;
	int            data5;

	const char    *char_buffer;
	const unsigned char *uchar_buffer;
	int            data_len;
};

#endif /* __DEVI_PROTOCOL_H__ */
