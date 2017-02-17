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
#define GET_RECV_DATA_DELAY               150    /* ����Ϊ��λ */
#define RECV_DATA_PACKET_INTERVAL         50     /* ����Ϊ��λ */

/*
 * ���洮�ھ����״̬
 * �Լ��Ӵ����Ͻ��յ�����
 */
typedef struct _FPGA_SERIAL_INFO
{
	FLXInt32        iHandle;
	int             band_rate;
	unsigned char   data_bits;       //5,6,7,8
	unsigned char   parity;          // 0--no, 1--odd, 2--even, 3--force parity "1", 4--force parity "0"
	unsigned char   stop_bits;       // 1,2
	RING_LIST_BUF   recv_buffer;

	unsigned int    last_time;               /* ���һ���ַ������ʱ�� */
	unsigned int    packet_interval;

	int             bUsing;                  /* �Ƿ���ʹ�� */
	int             bOpen;                   /* �����Ƿ��Ѵ� */
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
 * �����Ƿ��Ѿ��򿪣�1Ϊ����0Ϊû��
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
 * �����Ƿ��Ѿ����ã�1Ϊ���ã�0Ϊû��
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
 * ��ȡһ���Ѵ��Ҵ���ʹ���еĴ��ڵı��
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
 * ȡ���ھ��
 * ���δȡ�����򷵻�-1
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
 * ��ȡselect����ʱʹ�õĴ��ھ���б�
 *
 * ȡ�򿪵Ĵ��ھ��
 * ������
 *     maxfd  ���������
 *
 * ����ֵ������б�
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
 * �������һ���ַ������ʱ��
 * δ������ʹ�øú���ʱ��Ҫ����
 */
static void set_fpga_serial_recv_last_time(int serial_no, unsigned int itime)
{
	if(serial_no < 1 || serial_no > FPGA_SERIAL_COUNT)
		return;

	g_fpga_serial_info[serial_no - 1].last_time = itime;
}

/*
 * �����ӳ�ʱ���Ƿ��ѵ�
 * δ������ʹ�øú���ʱ��Ҫ����
 * ����ֵ��0δ���������ӳ�ʱ���ѵ�
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
 * ����ÿ�����ڵİ��ӳ�ʱ��
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

/* �õ�����ʱ�� */
static unsigned int get_m_second()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec/1000);
}
/*
 * ���մ��ڵ����ݣ����浽��Ӧ���ڵ����ݻ�����
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
 * ��ȡָ�����ڵ�����
 *
 * ֻ���Ѵ򿪵ġ��Ҵ���ʹ��״̬�Ĵ��ڻ�ȡ����
 * ������
 *     buffer �������ݵĻ���
 *     size   �����С
 *
 * ����ֵ������ȡ���ĳ��ȣ���ֵ <= size
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
 * ��ȡָ�����ھ���ϵ����ݣ������浽����
 *
 * ֻ�����Ѵ򿪵ġ��Ҵ���ʹ��״̬�Ĵ�������
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
 * �Ӵ��ڷ�������
 *
 * ������
 *     serial_no ���ڱ��
 *     buffer    ���ݻ���
 *     size      �����С
 *
 * ����ֵ���ɹ�Ϊ1������Ϊ0
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
 * �޸��Ѵ򿪴��ڵĲ����ʡ�����λ�Ȳ���
 * ����ֵ��1�ɹ���0ʧ��
 * ���ڻ�һ����ȫ���򿪣�����޸����õ�ͬʱ�����ô��ڴ���ʹ��״̬
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
 * ���ô��ڴ��ڷ�ʹ��״̬
 *
 * ����ֵ��1�ɹ���0ʧ�ܣ������ע
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
 * �����еĴ���
 *
 * �򿪵Ĵ��ڲ�δʹ�á�ֻ������Ϊʹ��״̬���Ż�����ʹ��
 * ����ֵ���Ѵ򿪵Ĵ�����
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

		/* ��Ҫ��fd��¼���������ڶ��յ������ݽ��з��ࡣ
		 * ÿ������Ӧ���и�����
		 * ���ص�ƽ����ն˵Ĵ������ݣ�Ӧ�ð����ն˴����Ŀؼ�ID */
		set_fpga_serial_handle(serial_no, handle);
		fprintf(stdout, "fpga serial %d open\n", serial_no);
		result++;
	}
	return result;
}

/*
 * ��ʼ�����е����ݽṹ
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
