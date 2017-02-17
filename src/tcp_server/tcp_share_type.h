/*
 * tcp_share_type.h
 *
 *  Created on: 2012-11-13
 *      Author: flx
 */

#ifndef __TCP_SHARE_TYPE_H__
#define __TCP_SHARE_TYPE_H__

#include "tps_client_manage.h"

#define DATA_SOURCE_PAD_OR_0               0           /* ��������pad����������ȿͻ��� */
#define DATA_SOURCE_SLAVE_SERVER           1           /* �������Դӻ� */
#define DATA_SOURCE_SLAVE_CLIENT           2           /* �������Դӻ��ϵĿͻ������� */

#define DESCRIBE_PAD_OR_0                 "pad or designer"



#define TELNET_ACTION_ADD                  1            /* ������һ��telnet���� */
#define TELNET_ACTION_DELETE               2            /* ������һ��telnet���� */

#define CONNECT_TYPE_UNKNOWN              -1
#define CONNECT_TYPE_TELNET                0            /* ��ͨ�Ĵӿͻ��˵������������� */
#define CONNECT_TYPE_TELNET_FOR_HTTP       1
#define CONNECT_TYPE_CLIENT                2            /* ���������յ��Ŀͻ��� */

/*
 * �������ȡ���ݵĻص������ӿ�
 * ������
 *     data_source  ������Դ
 *     buffer       ���ݻ���
 *     data_len     ���������ݳ���
 *     clientInfo   �ͻ���
 *
 * data_source ȡֵ��
 *                 DATA_SOURCE_PAD_OR_0, DATA_SOURCE_SLAVE_SERVER, DATA_SOURCE_SLAVE_CLIENT
 */
typedef void (*network_recv_data)(int data_source, unsigned char *buffer, int *data_len, CLIENT_INFO_STRU *clientInfo);

#endif /* __TCP_SHARE_TYPE_H__ */
