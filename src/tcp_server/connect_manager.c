#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "FLXCommon/flxtypes.h"
#include "FLXCommon/flxthread.h"
#include "FLXCommon/flxnettypes.h"
#include "connect_manager.h"
#include "connect_list.h"

#define EPOLL_MAX_COUNT      100
#define EPOLL_RECV_BUF_MAX   1024

static int g_epfd = -1;
static connect_manager_callback g_connect_manager_callback = NULL;

static void connect_manager_output_recv(int connect_type, int connect_no, unsigned char *buffer, int data_len)
{
	if(g_connect_manager_callback) g_connect_manager_callback(CMC_REASON_DATA, connect_type, connect_no, buffer, data_len);
}
/*
 * ���ϲ���÷���һЩ֪ͨ��Ϣ
 */
static void connect_manager_send_message(int reason, int connect_type, int connect_no)
{
	if(g_connect_manager_callback) g_connect_manager_callback(reason, connect_type, connect_no, NULL, 0);
}

static void process_connect_closed(CONNECT_NODE connect_node)
{
	int connect_no = connect_node->connect_no;
	int connect_type = connect_node->connect_type;
	if(CONNECT_TYPE_TELNET_FOR_HTTP == connect_type)   /* http ����ֱ��ɾ�� */
	{
		printf("delete http connect(%d), %s\n", connect_node->sock, connect_node->ip_address);

		connect_list_delete(connect_type, connect_no);
		/* TODO: check */
		connect_manager_send_message(CMC_REASON_CONNECT_DELETE, connect_type, connect_no);
	}
	else
	{
		connect_manager_send_message(CMC_REASON_CONNECT_CLOSE, connect_type, connect_no);
		connect_list_reconnect(connect_node->sock);
	}
}

void *thread_func_epoll_wait(void *param)
{
	int k, nfds;
	int epfd = g_epfd;
	struct epoll_event _events[EPOLL_MAX_COUNT];
	unsigned char recv_buf[EPOLL_RECV_BUF_MAX];

	printf("%s thread run ... \n", __FUNCTION__);
	while(1)
	{
		//printf("epoll_wait start ... \n");

		nfds = epoll_wait(epfd, _events, EPOLL_MAX_COUNT, -1);
		//printf("epoll_wait nfds %d \n", nfds);

		for(k = 0; k < nfds; ++k)
		{
			if(_events[k].events & EPOLLIN)
			{
				CONNECT_NODE connect_node = (CONNECT_NODE)_events[k].data.ptr;
				ssize_t len = recv(connect_node->sock, recv_buf, EPOLL_RECV_BUF_MAX, 0);

				if(len > 0)
				{
					connect_manager_output_recv(connect_node->connect_type, connect_node->connect_no, recv_buf, len);
					if(len >= EPOLL_RECV_BUF_MAX)
					{
						while(1)
						{
							len = recv(connect_node->sock, recv_buf, EPOLL_RECV_BUF_MAX, 0);
							if(len > 0) connect_manager_output_recv(connect_node->connect_type, connect_node->connect_no, recv_buf, len);
							if(len < EPOLL_RECV_BUF_MAX)
								break;
						}
					}
				}
				else
				{
					// close
					process_connect_closed(connect_node);
				}
			}
		}
	}
	printf("%s thread closed ... \n", __FUNCTION__);
	return ((void *)0);
}

int callback_by_connect_list(CONNECT_NODE connect_node)
{
	int ret, result;
	struct epoll_event ev;

	ev.data.ptr = (void *)connect_node;
	ev.events = EPOLLIN|EPOLLERR |EPOLLET;
	ret = epoll_ctl(g_epfd, EPOLL_CTL_ADD, connect_node->sock, &ev);
	if(ret != 0)
	{
		perror("epoll_ctl error:");
	}
	result = (0 == ret) ? 1:0;
	printf("new socket %d for epoll_ctl.\n", (int)connect_node->sock);

	/* TODO: check */
	if(result) connect_manager_send_message(CMC_REASON_CONNECT_OPEN, connect_node->connect_type, (int)connect_node->connect_no);

	printf("%s 222 connect_node->connect_no %d\n", __FUNCTION__, connect_node->connect_no);
	return result;
}

int connect_manager_send_data(int connect_type, int connect_no, unsigned char *buffer, int data_len)
{
	return connect_send_data(connect_type, connect_no, buffer, data_len);
}

/*  Ӧ�ü��IP�Ƿ�Ϸ� ?????????????
 * ����һ��telnet����
 * ��ͨ���ص�֪ͨ�ϲ�Ϊ���Ӵ���һ������ڵ㣬������������ӵ��շ�����
 * ͬʱ����������Ϣ�������ӹ����������ӹ�������Ϊ�����Ӵ���socket����һֱά��������ӣ�ֱ������ɾ��
 * ���ӹ��������ִ�гɹ������᷵��һ����ʶ���ӵı�ţ�connect_no
 *
 * ����ڵ㽫���ֽڼ��뵽����ڵ��б��У����ᱣ��connect_no��
 *
  *������
 *     ip_address  IP��ַ
 *     port        �˿�
 *
 * ����ֵ��
 *      -1    �����Ѿ�����
 *       0    ����
 *      >0    �������أ��Ǳ�ʶ���ӵ�һ��id����id�����ڻ����б��з���ı��
 *
 *
 *      �Ժ��ظ���ip�Ͷ˿ڽ�������� ...
 */
int connect_manager_add_telnet(int connect_no, const char *ip_address, int port)
{
	if(!connect_list_add_telnet(CONNECT_TYPE_TELNET, connect_no, ip_address, port))
	{
		printf("%s %s:%d error\n", __FUNCTION__, ip_address, port);
		return 0;
	}

	connect_manager_send_message(CMC_REASON_CONNECT_ADD, CONNECT_TYPE_TELNET, connect_no);
	fprintf(stdout, "telnet %d :%s %d\n", connect_no, ip_address, port);
	return 1;
}
int connect_manager_add_http(int connect_type, const char *ip_address, int port, void *user_data)
{
	return 0;//connect_list_add_http(connect_type, ip_address, port, user_data);
}

/*
 * ����telnetId���������ͣ���������Ҳ���������ַ�ʽ����ɾ��һ������
 */
int connect_manager_delete(int connect_type, int connect_no)
{
	// TODO ʵ�ʵ�ɾ������
	connect_list_delete(connect_type, connect_no);


	connect_manager_send_message(CMC_REASON_CONNECT_DELETE, connect_type, connect_no);
	return 0;
}
int connect_manager_init(connect_manager_callback callback)
{
	FLXThread pid;
	//����g_connecting_node_head�ڵ㴴��socket���ӣ�����socket����g_epfd
	connect_list_init(callback_by_connect_list);

	g_epfd = epoll_create(EPOLL_MAX_COUNT);
	if(g_epfd < 0)
	{
		fprintf(stderr, "epoll_create failed\n");
		return 0;
	}

	if(thread_create(&pid, NULL, (void *)thread_func_epoll_wait, NULL) != 0)
	{
		printf("%s : thread create error\n", __FUNCTION__);
		return 0;
	}

	g_connect_manager_callback = callback;
	return 1;
}

int connect_manager_close()
{
	connect_list_release();
	close(g_epfd);
	return 1;
}
