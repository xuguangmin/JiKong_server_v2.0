/*
 * ccc_packet.h
 *
 *  Created on: 2013-3-28
 *      Author: flx
 */

#ifndef __CCC_PACKET_H__
#define __CCC_PACKET_H__

/* Э������ */
typedef struct __CCCPDU
{
	unsigned char   cmd;
	unsigned char   cmd_ex;
	unsigned char  *data;
	int             data_len;
	unsigned char   checksum1;
	unsigned char   checksum2;
}CCCPDU;

/* ��Э������ */
typedef struct __CCCDATA
{
	unsigned char  *data;
	int             data_len;
}CCCDATA;

/* ���ݰ� */
typedef struct __CCCPACKET
{
#define CCCPACKET_DATA_TYPE_PROTOCOL        0   /* Э������ */
#define CCCPACKET_DATA_TYPE_NORMAL          1   /* ��Э������ */
	int             data_type;                  /* �������� */

	unsigned char  *buffer;                     /* �������ݻ��� */
	int             buffer_size;
	int             data_len;
	int             using;                      /* �Ƿ���ʹ�� */

	union
	{
		CCCPDU    pdu;
		CCCDATA   normal;
	}recv_data;
}CCCPACKET;
typedef struct CCCPACKET *CCCPACKET_NODE;


extern int cccpacket_init(CCCPACKET *cccpacket);
extern int cccpacket_release(CCCPACKET *cccpacket);
extern int cccpacket_using(CCCPACKET *cccpacket);
extern int cccpacket_not_using(CCCPACKET *cccpacket);

extern int cccpacket_save_protocol_data(CCCPACKET *cccpacket, const unsigned char *buffer, int data_len);
extern int cccpacket_save_normal_data(CCCPACKET *cccpacket, const unsigned char *buffer, int data_len);

#endif /* __CCC_PACKET_H__ */
