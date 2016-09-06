/*
 * ipc_msg.c
 *
 *  Created on: 2013-5-24
 *      Author: jiayg
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "ipc_msg.h"

#define IPC_MSG_KEY_C_TO_S         1234             /* console to server */
static int g_msg_id_c_to_s  = -1;

static int create_msg_queue(key_t __key, int *id)
{
	int msgid = msgget(__key, 0666|IPC_CREAT);
	if(msgid == -1)
	{
		fprintf(stderr, "msgget failed. %d\n", errno);
		return 0;
	}
	if(id) *id = msgid;
	return 1;
}

static int get_msg_id_c_to_s()
{
	if(-1 == g_msg_id_c_to_s)
	{
		if(!create_msg_queue((key_t)IPC_MSG_KEY_C_TO_S, &g_msg_id_c_to_s))
		{
			g_msg_id_c_to_s = -1;
			return 0;
		}
	}
	return 1;
}
int get_msg_from_console(struct ccc_msg *lp_ccc_msg)
{
	int result;
	if(!lp_ccc_msg || !get_msg_id_c_to_s())
		return 0;

	result = msgrcv(g_msg_id_c_to_s, (void *)lp_ccc_msg, BUFSIZ, 0, 0);
	return (-1 == result) ? 0:result;
}

int send_msg_to_server(const struct ccc_msg *lp_ccc_msg)
{
	if(!lp_ccc_msg || !lp_ccc_msg->msg_data || !get_msg_id_c_to_s())
	{
		printf("get_msg_id_c_to_s ERROR\n");
		return 0;
	}

	if(msgsnd(g_msg_id_c_to_s, (void *)lp_ccc_msg, strlen(lp_ccc_msg->msg_data), 0) == -1)
		return 0;

	return 1;
}


#define IPC_MSG_KEY_S_TO_C         1235             /* server to console */
static int g_msg_id_s_to_c  = -1;

static int get_msg_id_s_to_c()
{
	if(-1 == g_msg_id_s_to_c)
	{
		if(!create_msg_queue((key_t)IPC_MSG_KEY_S_TO_C, &g_msg_id_s_to_c))
		{
			g_msg_id_s_to_c = -1;
			return 0;
		}
	}
	return 1;
}
int send_msg_to_console(const struct ccc_msg *lp_ccc_msg)
{
	if(!lp_ccc_msg || !lp_ccc_msg->msg_data || !get_msg_id_s_to_c())
	{
		printf("get_msg_id_s_to_c ERROR\n");
		return 0;
	}

	if(msgsnd(g_msg_id_s_to_c, (void *)lp_ccc_msg, strlen(lp_ccc_msg->msg_data), 0) == -1)
	{
		printf("%s failed, %s\n", __FUNCTION__, strerror(errno));
		return 0;
	}

	return 1;
}
int get_msg_from_server(struct ccc_msg *lp_ccc_msg, long int msgtyp, int msgflg)
{
	int result;
	if(!lp_ccc_msg || !get_msg_id_s_to_c())
		return 0;

	result = msgrcv(g_msg_id_s_to_c, (void *)lp_ccc_msg, BUFSIZ, msgtyp, msgflg);
	return (-1 == result) ? 0:result;
}





