#ifndef __TPS_TCP_SERVER_H__
#define __TPS_TCP_SERVER_H__

#include "FLXCommon/flxtypes.h"
#include "FLXCommon/flxnettypes.h"
#include "configure.h"
#include "tps_client_manage.h"
#include "tcp_share_type.h"




extern int network_manager_init(SERVER_CONFIG *server_config, network_recv_data callback);
extern int network_manager_close();

extern int network_manager_send_data(FLXSocket sock, unsigned char *data, unsigned int data_len);
extern int network_manager_send_url(const char *url);
extern int network_manager_wol(const char *des_mac_addr);
extern int network_manager_send_onvif(const char *http_uri, const char *http_body);



extern int slave_relay_data_to_master(FLXByte *strName, FLXInt32 iLen);
FLXInt32 tps_tcp_server_stop();
extern int add_http_connect(const char *ip_address, int port);

#endif /* __TPS_TCP_SERVER_H__ */
