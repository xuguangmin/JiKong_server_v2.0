#ifndef __TPS_CLIENT_MANAGE_H__
#define __TPS_CLIENT_MANAGE_H__

#include "FLXCommon/flxtypes.h"
#include "FLXCommon/flxnettypes.h"

typedef struct __CLIENT_INFO_STRU
{
	unsigned char  clientType;             /* 客户端类型 */
	FLXSocket      sock;				   /* 客户端套接字 */
}CLIENT_INFO_STRU;

typedef int (*tps_client_callback)(int data_source, unsigned char *data, int *data_len, CLIENT_INFO_STRU *clientInfo);

extern int  tps_client_manage_init(tps_client_callback callback);
extern void tps_release_all_client(void);

extern int  tps_reg_client_pad_or_0(FLXSocket sock);
extern int  tps_set_client_type(FLXSocket sock, unsigned char client_type);
extern void send_data_to_all_pad(unsigned char *data, unsigned int data_len);
extern int  send_data_to_pad(FLXSocket sock, unsigned char *data, unsigned int data_len);

extern int  tps_reg_client_slave_server(FLXSocket sock);
extern void send_data_to_all_slave(unsigned char *data, unsigned int data_len);
extern void send_data_to_other_slave(FLXSocket sock, unsigned char *data, unsigned int data_len);

#endif  /* __TPS_CLIENT_MANAGE_H__ */
