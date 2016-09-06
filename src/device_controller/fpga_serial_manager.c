#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <time.h>

#include "devc_fpga_serial.h"
#include "fpga_serial_manager.h"

#define SELECT_BUFFER_SIZE                64
#define GET_RECV_DATA_DELAY               150    /* 毫秒为单位 */
#define RECV_DATA_PACKET_INTERVAL         50     /* 毫秒为单位 */

/*
 * 保存串口句柄、状态
 * 以及从串口上接收的数据
 */
typedef struct _FPGA_SERIAL_INFO
{
	FLXInt32        iHandle;
	int             band_rate;
	unsigned char   data_bits;       //5,6,7,8
	unsigned char   parity;          // 0--no, 1--odd, 2--even, 3--force parity "1", 4--force parity "0"
	unsigned char   stop_bits;       // 1,2
	RING_LIST_BUF   recv_buffer;

	unsigned int    last_time;               /* 最后一个字符到达的时间 */
	unsigned int    packet_interval;

	int             bUsing;                  /* 是否在使用 */
	int             bOpen;                   /* 串口是否已打开 */
	pthread_mutex_t serial_mutex;
}FPGA_SERIAL_INFO;
static FPGA_SERIAL_INFO g_fpga_serial_info[FPGA_SERIAL_COUNT];

static void fpga_serial_lock(int serial_no)
{
	pthread_mutex_lock(&g_fpga_serial_info[serial_no-1].serial_mutex);
}
static void fpga_serial_unlock(int serial_no)
{
	pthread_mutex_unlock(&g_fpga_serial_info[serial_no-1].serial_mutex);
}

static int set_fpga_serial_handle(int serial_no, int handle)
{
	int k;
	if(serial_no < 1 || serial_no > FPGA_SERIAL_COUNT)
		return 0;

	fpga_serial_lock(serial_no);
	k = serial_no - 1;
	g_fpga_serial_info[k].iHandle = handle;
	g_fpga_serial_info[k].bOpen   = 1;
	fpga_serial_unlock(serial_no);
	return 1;
}

/*
 * 串口是否已经打开，1为开，0为没开
 */
static int fpga_serial_is_open(int serial_no)
{
	int result;
	if(serial_no < 1 || serial_no > FPGA_SERIAL_COUNT)
		return 0;

	fpga_serial_lock(serial_no);
	result = g_fpga_serial_info[serial_no - 1].bOpen ? 1:0;
	fpga_serial_unlock(serial_no);

	return result;
}
/*
 * 串口是否已经启用，1为启用，0为没有
 */
static int fpga_serial_is_using(int serial_no)
{
	int result;
	if(!fpga_serial_is_open(serial_no))
		return 0;

	fpga_serial_lock(serial_no);
	result = g_fpga_serial_info[serial_no - 1].bUsing ? 1:0;
	fpga_serial_unlock(serial_no);

	return result;
}

/*
 * bUsing 1 or 0
 */
static void set_fpga_serial_using(int serial_no, int bUsing, int baud_rate, unsigned char data_bits,
                                  unsigned char parity, unsigned char stop_bits)
{
	if(!fpga_serial_is_open(serial_no))
		return;

	fpga_serial_lock(serial_no);
	g_fpga_serial_info[serial_no - 1].bUsing = (0 == bUsing ? 0:1);

	g_fpga_serial_info[serial_no - 1].band_rate = baud_rate;
	g_fpga_serial_info[serial_no - 1].data_bits = data_bits;
	g_fpga_serial_info[serial_no - 1].parity    = parity;
	g_fpga_serial_info[serial_no - 1].stop_bits = stop_bits;
	fpga_serial_unlock(serial_no);
}

/*
 * 获取一个已打开且处于使用中的串口的编号
 */
static int get_serial_no(int fd)
{
	int k, result = -1;
	for(k = 0; k < FPGA_SERIAL_COUNT; ++k)
	{
		int serial_no = k + 1;

		fpga_serial_lock(serial_no);
		if(!g_fpga_serial_info[k].bOpen ||
		   !g_fpga_serial_info[k].bUsing)
		{
			fpga_serial_unlock(serial_no);
			continue;
		}

		if(fd == g_fpga_serial_info[k].iHandle)
		{
			fpga_serial_unlock(serial_no);
			result = serial_no;
			break;
		}
		fpga_serial_unlock(serial_no);
	}
	return result;
}

/*
 * 取串口句柄
 * 如果未取到，则返回-1
 */
static FLXInt32 get_fpga_serial_handle(int serial_no)
{
	FLXInt32 result;
	if(!fpga_serial_is_open(serial_no))
		return -1;

	fpga_serial_lock(serial_no);
	result = g_fpga_serial_info[serial_no - 1].iHandle;
	fpga_serial_unlock(serial_no);

	return result;
}

static RING_LIST_BUF *get_serial_recv_buffer(int serial_no)
{
	if(serial_no < 1 || serial_no > FPGA_SERIAL_COUNT)
		return NULL;

	int k = serial_no - 1;
	return &g_fpga_serial_info[k].recv_buffer;
}

/*
 * 获取select监听时使用的串口句柄列表
 *
 * 取打开的串口句柄
 * 参数：
 *     maxfd  返回最大句柄
 *
 * 返回值：句柄列表
 */
fd_set get_fpga_serial_fd_set(int *maxfd)
{
	int k;
	fd_set result;
	FD_ZERO(&result);
	*maxfd = 0;

	for(k = 0; k < FPGA_SERIAL_COUNT; ++k)
	{
		FLXInt32 iHandle = get_fpga_serial_handle(k+1);
		if(iHandle < 0)
			continue;

		FD_SET(iHandle, &result);

		if(*maxfd < iHandle)
			*maxfd = iHandle;

	}
	return result;
}

/*
 * 设置最后一个字符到达的时间
 * 未加锁，使用该函数时需要加锁
 */
static void set_fpga_serial_recv_last_time(int serial_no, unsigned int itime)
{
	if(serial_no < 1 || serial_no > FPGA_SERIAL_COUNT)
		return;

	g_fpga_serial_info[serial_no - 1].last_time = itime;
}

/*
 * 检查包延迟时间是否已到
 * 未加锁，使用该函数时需要加锁
 * 返回值：0未到，否则延迟时间已到
 */
static int check_fpga_serial_recv_packet_delay(int serial_no, unsigned int current_time)
{
	if(serial_no < 1 || serial_no > FPGA_SERIAL_COUNT)
		return 0;

	if(labs(current_time - g_fpga_serial_info[serial_no - 1].last_time) < g_fpga_serial_info[serial_no - 1].packet_interval)
		return 0;

	return 1;
}

/*
 * 设置每个串口的包延迟时间
 */
void set_fpga_serial_recv_packet_delay(int serial_no, unsigned int delay)
{
	if(!fpga_serial_is_open(serial_no))
		return;
	if(delay <= 0)
		return;

	fpga_serial_lock(serial_no);
	g_fpga_serial_info[serial_no - 1].packet_interval = delay;
	fpga_serial_unlock(serial_no);
}

/* 得到毫秒时间 */
static unsigned int get_m_second()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec/1000);
}
/*
 * 接收串口的数据，保存到相应串口的数据缓存中
 */
static int save_recv_data_to_buffer(int fd, unsigned char *buffer, int buf_size)
{
	int serial_no = get_serial_no(fd);
	RING_LIST_BUF *recv_buffer = get_serial_recv_buffer(serial_no);
	if(recv_buffer)
	{
		int s;
		fpga_serial_lock(serial_no);
		for(s = 0; s < buf_size; ++s)
		{
			ring_list_buf_append(recv_buffer, buffer[s]);
		}
		set_fpga_serial_recv_last_time(serial_no, get_m_second());
		fpga_serial_unlock(serial_no);
	}
	return 0;
}
/*
 * 获取指定串口的数据
 *
 * 只向已打开的、且处于使用状态的串口获取数据
 * 参数：
 *     buffer 接收数据的缓存
 *     size   缓存大小
 *
 * 返回值：返回取到的长度，该值 <= size
 */
int get_fpga_serial_recv_data(int serial_no, unsigned char *buffer, int size)
{
	int result;
	RING_LIST_BUF *ring_list_buf;
	if(!fpga_serial_is_using(serial_no))
		return 0;

	ring_list_buf = get_serial_recv_buffer(serial_no);
	if(!ring_list_buf)
		return 0;

	fpga_serial_lock(serial_no);
	if(ring_list_buf_data_length(ring_list_buf) <= 0)
	{
		fpga_serial_unlock(serial_no);
		return 0;
	}
	if(!check_fpga_serial_recv_packet_delay(serial_no, get_m_second()))
	{
		fpga_serial_unlock(serial_no);
		return 0;
	}

	result = ring_list_buf_get_data(ring_list_buf, buffer, size);
	fpga_serial_unlock(serial_no);

	return result;
}

/*
 * 读取指定串口句柄上的数据，并保存到缓存
 *
 * 只保存已打开的、且处于使用状态的串口数据
 */
void fpga_serial_read(int fd)
{
	int nread = 0;
	unsigned char buffer[SELECT_BUFFER_SIZE];
	ioctl(fd, FIONREAD, &nread);
	if(nread > 0)
	{
		while(1)
		{
			nread = devc_fpga_serial_read_data(fd, buffer, SELECT_BUFFER_SIZE);
			if(nread <= 0)
				break;

			buffer[nread] = 0;
			save_recv_data_to_buffer(fd, buffer, nread);
			if(nread < SELECT_BUFFER_SIZE)
				break;
		}
	}
}

/*
 * 从串口发送数据
 *
 * 参数：
 *     serial_no 串口编号
 *     buffer    数据缓存
 *     size      缓存大小
 *
 * 返回值：成功为1，否则为0
 */
int fpga_serial_write(int serial_no, unsigned char *buffer, int size)
{
	FLXInt32 iHandle;
	if(!fpga_serial_is_using(serial_no))
	{
		return 0;
	}

	iHandle = get_fpga_serial_handle(serial_no);
	if(iHandle < 0)
	{
		return 0;
	}

	return (0 == devc_fpga_serial_write_data(iHandle, buffer, size)) ? 1:0;
}

/*
 * 修改已打开串口的波特率、数据位等参数
 * 返回值：1成功，0失败
 * 串口会一次性全部打开，这儿修改配置的同时，会让串口处于使用状态
 */
int modify_fpga_serial_config(int serial_no, int baud_rate, unsigned char data_bits,
                                unsigned char parity, unsigned char stop_bits)
{
	FLXInt32 iHandle = get_fpga_serial_handle(serial_no);
	if(iHandle < 0)
		return 0;

	set_fpga_serial_using(serial_no, 0, 0, 0, 0, 0);
	if(devc_fpga_serial_modify(iHandle, baud_rate, data_bits, parity, stop_bits) != 0)
		return 0;

	printf("fpga serial %d: %d %d-%d-%d\n", serial_no, baud_rate, data_bits, parity, stop_bits);
	set_fpga_serial_using(serial_no, 1, baud_rate, data_bits, parity, stop_bits);
	return 1;
}

/*
 * 设置串口处于非使用状态
 *
 * 返回值：1成功，0失败，不需关注
 */
int enable_fpga_serial_rest(int serial_no)
{
	if(!fpga_serial_is_open(serial_no))
		return 0;

	set_fpga_serial_using(serial_no, 0, 0, 0, 0, 0);
	return 1;
}


int get_serial_stat_info(char *buffer, int buf_size)
{
	int k;
	char aline[256];
	for(k = 0; k < FPGA_SERIAL_COUNT; ++k)
	{
		if(!fpga_serial_is_using(k+1))
		{
			sprintf(aline, "%d OFF\n", k+1);
			strcat(buffer, aline);
			continue;
		}
		sprintf(aline, "%d %d %d-%d-%d\n", k+1, g_fpga_serial_info[k].band_rate, g_fpga_serial_info[k].data_bits, g_fpga_serial_info[k].parity, g_fpga_serial_info[k].stop_bits);
		strcat(buffer, aline);
	}
	return 1;
}
/*
 * 打开所有的串口
 *
 * 打开的串口并未使用。只有设置为使用状态，才会真正使用
 * 返回值：已打开的串口数
 */
int fpga_serial_open_all()
{
	int k, result = 0;
	FLXInt32 handle;
	for(k = 0; k < FPGA_SERIAL_COUNT; ++k)
	{
		int serial_no = k + 1;
		if(devc_fpga_serial_open(serial_no, &handle) != 0)
		{
			fprintf(stdout, "error %d\n", handle);
			continue;
		}

		/* 需要把fd记录下来，用于对收到的数据进行分类。
		 * 每个串口应该有个队列
		 * 返回到平板等终端的串口数据，应该包含终端传来的控件ID */
		set_fpga_serial_handle(serial_no, handle);
		fprintf(stdout, "fpga serial %d open\n", serial_no);
		result++;
	}
	return result;
}

/*
 * 初始化所有的数据结构
 */
void fpga_serial_info_init()
{
	int k;
	int packet_interval = RECV_DATA_PACKET_INTERVAL;
	for(k = 0; k < FPGA_SERIAL_COUNT; ++k)
	{
		g_fpga_serial_info[k].iHandle   = -1;
		g_fpga_serial_info[k].bOpen     = 0;
		g_fpga_serial_info[k].bUsing    = 0;
		g_fpga_serial_info[k].band_rate = 0;
		g_fpga_serial_info[k].data_bits = 0;
		g_fpga_serial_info[k].parity    = 0;
		g_fpga_serial_info[k].stop_bits = 0;

		g_fpga_serial_info[k].last_time = 0;
		g_fpga_serial_info[k].packet_interval = packet_interval;

		pthread_mutex_init(&g_fpga_serial_info[k].serial_mutex, NULL);
		ring_list_buf_init(&g_fpga_serial_info[k].recv_buffer);
	}
}
