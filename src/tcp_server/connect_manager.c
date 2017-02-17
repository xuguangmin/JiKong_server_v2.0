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
 * 向上层调用发送一些通知信息
 */
static void connect_manager_send_message(int reason, int connect_type, int connect_no)
{
	if(g_connect_manager_callback) g_connect_manager_callback(reason, connect_type, connect_no, NULL, 0);
}

static void process_connect_closed(CONNECT_NODE connect_node)
{
	int connect_no = connect_node->connect_no;
	int connect_type = connect_node->connect_type;
	if(CONNECT_TYPE_TELNET_FOR_HTTP == connect_type)   /* http 连接直接删掉 */
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

/*  应该检测IP是否合法 ?????????????
 * 增加一个telnet连接
 * 会通过回调通知上层为连接创建一个缓存节点，用来保存该连接的收发数据
 * 同时，把连接信息传给连接管理器，连接管理器将为该连接创建socket，并一直维护这个连接，直到它被删除
 * 连接管理器如果执行成功，将会返回一个标识连接的编号：connect_no
 *
 * 缓存节点将把字节加入到缓存节点列表中，并会保存connect_no，
 *
  *参数：
 *     ip_address  IP地址
 *     port        端口
 *
 * 返回值：
 *      -1    连接已经存在
 *       0    错误
 *      >0    正常返回，是标识连接的一个id，该id来自于缓存列表中分配的编号
 *
 *
 *      以后重复的ip和端口将是允许的 ...
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
 * 根据telnetId和连接类型（其他连接也类似于这种方式）来删除一个连接
 */
int connect_manager_delete(int connect_type, int connect_no)
{
	// TODO 实际的删除过程
	connect_list_delete(connect_type, connect_no);


	connect_manager_send_message(CMC_REASON_CONNECT_DELETE, connect_type, connect_no);
	return 0;
}
int connect_manager_init(connect_manager_callback callback)
{
	FLXThread pid;
	//根据g_connecting_node_head节点创建socket连接，并将socket加入g_epfd
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
