#ifndef __CONNECT_MANAGER_H__
#define __CONNECT_MANAGER_H__

#include "tcp_share_type.h"


#define CMC_REASON_DATA                    0
#define CMC_REASON_CONNECT_CLOSE           1
#define CMC_REASON_CONNECT_OPEN            2
#define CMC_REASON_CONNECT_DELETE          3
#define CMC_REASON_CONNECT_ADD             4

/*
 * reason 取值：
 *             CMC_REASON_DATA
 *             CMC_REASON_CONNECT_CLOSE
 *             CMC_REASON_CONNECT_OPEN
 *             CMC_REASON_CONNECT_DELETE
 *             CMC_REASON_CONNECT_ADD
 *
 * connect_type 取值：
 *             CONNECT_TYPE_TELNET
 *             ...
 */
typedef void (*connect_manager_callback)(int reason, int connect_type, int connect_no, unsigned char *buffer, int data_len);

extern int connect_manager_init(connect_manager_callback callback);
extern int connect_manager_close();

extern int connect_manager_add_telnet(int connect_no, const char *ip_address, int port);
extern int connect_manager_add_http(int connect_type, const char *ip_address, int port, void *user_data);

extern int connect_manager_delete(int connect_type, int connect_no);

extern int connect_manager_send_data(int connect_type, int connect_no, unsigned char *buffer, int data_len);

#endif  /* __CONNECT_MANAGER_H__ */
