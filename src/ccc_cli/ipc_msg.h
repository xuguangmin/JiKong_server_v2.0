/*
 * ipc_msg.h
 *
 *  Created on: 2013-5-24
 *      Author: jiayg
 */

#ifndef __IPC_MSG_H__
#define __IPC_MSG_H__

struct ccc_msg
{
	long int msg_type;
	char     msg_data[BUFSIZ];
};

/* console to server */
extern int send_msg_to_server(const struct ccc_msg *lp_ccc_msg);
extern int get_msg_from_console(struct ccc_msg *lp_ccc_msg);

/* server to console */
extern int get_msg_from_server(struct ccc_msg *lp_ccc_msg, long int msgtyp, int msgflg);
extern int send_msg_to_console(const struct ccc_msg *lp_ccc_msg);

#endif /* __IPC_MSG_H__ */
