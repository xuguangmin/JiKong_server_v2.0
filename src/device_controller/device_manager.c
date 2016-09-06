#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>

#include "device_manager.h"
#include "fpga_serial_manager.h"
#include "devc_fpga_irda.h"
#include "devc_fpga_relay.h"
#include "ring_list.h"

#define COMM_RECV_BUFFER_SIZE     4096

static int g_b_device_initialized  = 0;                          /* �豸�������Ƿ��ѱ���ʼ�� */
static serial_port_recv_data      g_recv_data_callback = 0;
//static int g_serial_port_recv_buffer_size = 0;

/*
 * ����Ҫ�������ڵ�����
 */
static RING_LIST  g_serial_send_list;
static sem_t      g_serial_send_sem;

/*
 * ����Ҫ��������ڵ�����
 */
static RING_LIST  g_irda_send_list;
static sem_t      g_irda_send_sem;

static int device_is_initialized()
{
	if(!g_b_device_initialized)
	{
		printf("device have not be initialized\n");
		return 0;
	}
	return 1;

}
/*
 * ��ȡ���ڽ������ݵĻص�����
 */
void process_recv_data(int serial_no, unsigned char *buffer, int size)
{
	if(g_recv_data_callback) g_recv_data_callback(serial_no, buffer, size);
	else
	{
		buffer[size] = '\0';
		printf("--- debug fpga serial(%d): %s\n", serial_no, buffer);
	}
}


/*
 * ��ȡ�������ݵ��߳�
 *
 * ��ѯÿ�����ڵ������棬ȡ����
 * ��������ݣ��߳���Ϣ1��
 */
FLXInt32 thread_func_recv(void *param)
{
	int k, len;
	//int b_get_data;
	unsigned char buffer[COMM_RECV_BUFFER_SIZE];

	while(1)
	{
		//b_get_data = 0;
		for(k = 0; k < FPGA_SERIAL_COUNT; ++k)
		{
			if((len = get_fpga_serial_recv_data(k+1, buffer, COMM_RECV_BUFFER_SIZE-1)) > 0)
			{
				//b_get_data = 1;
				process_recv_data(k+1, buffer, len);
			}
		}

		//if(!b_get_data) sleep(1);
	}
	return 0;
}

/*
 * ��ش����߳�
 *
 * ���ÿ�������Ƿ�ɶ�
 * �ɶ������ȡ���ݣ����浽ÿ�����ڵ���������
 */
FLXInt32 thread_func_select(void *param)
{
	int k, result;
	int maxfd = 0;
	fd_set inputs, testfds;

	inputs = get_fpga_serial_fd_set(&maxfd);
	maxfd += 1;

	printf("start to monitor FPGA serial port ... \n");

	while(1)
	{
		testfds = inputs;
		result = select(maxfd, &testfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);//);

		if(result < 1)                                            /* 0 timeout, -1 perror("select")*/
			continue;

		for(k = 0; k < maxfd; ++k)
		{
			int fd = k;
			if(FD_ISSET(fd, &testfds))
			{
				fpga_serial_read(fd);
			}
		}
	}
	return 0;
}

/*
 * �޸Ĵ��ڵĲ����ʡ�����λ�Ȳ���
 * ����ֵ��1�ɹ���0ʧ��
 */
int comm_serial_modify_config(int serial_no, int baud_rate, int data_bits, int parity, int stop_bits)
{
	if(!device_is_initialized()) return 0;
	return modify_fpga_serial_config(serial_no, baud_rate, data_bits, parity, stop_bits);
}

int get_device_stat_info(char *buffer, int buf_size)
{
	if(!device_is_initialized()) return 0;
	return get_serial_stat_info(buffer, buf_size);
}
/*
 * ʹ���ڴ��ڲ�ʹ�õ�״̬����ʱ�����Դ��ڽ����շ�
 */
int comm_serial_stop(int serial_no)
{
	if(!device_is_initialized()) return 0;
	return enable_fpga_serial_rest(serial_no);
}


/*
 * ׷�����ݵ����ڵķ����б���
 *
 * ÿ��׷�ӻ����һ����¼
 * ������
 *     serial_no ���ڱ��
 *     buffer    ���ݻ���
 *     size      �����С
 *
 * ����ֵ���ɹ�Ϊ1������Ϊ0
 */
int append_data_to_serial_send_list(int serial_no, const unsigned char *buffer, int size)
{
	int result;
	//if(!fpga_serial_is_using(serial_no))
		//return 0;

	result = ring_list_append_data(&g_serial_send_list, buffer, size, serial_no);
	if(result)
	{
		if(sem_post(&g_serial_send_sem) != 0)
		{
			printf("%s sem_post error, %d %s\n", __FUNCTION__, errno, strerror(errno));
		}
	}
	else
	{
		printf("%s ring_list_append_data error\n", __FUNCTION__);
	}

	return result;
}
/*
 * ֻ���Ѵ򿪵ġ��Ҵ���ʹ��״̬�Ĵ��ڷ�������
 * ����ֵ��1 ���ӵ����гɹ���0Ϊ���ɹ�
 */
int send_data_to_comm_serial(int serial_no, const unsigned char *buffer, int size)
{
	if(!device_is_initialized()) return 0;
	return append_data_to_serial_send_list(serial_no, buffer, size);
}

/*
 * �ӷ����б���ȡһ����¼
 *
 * ���buffer�Ĵ�СС���б���һ����¼�����ݵĳ��ȣ�
 * ��ֻȡǰ�ߵĲ������ݣ�ʣ�µ����ݣ�������Ϊһ����¼���´���ȡ
 * ������
 *     buffer    ���ݻ���
 *     size      �����С
 *     serial_no ���ڱ��
 *
 * ����ֵ��ȡ�������ݳ��ȡ����ֽ�Ϊ��λ��<=0�ķ���ֵ����Чֵ
 */
int get_fpga_serial_send_data(unsigned char *buffer, int size, int *serial_no)
{
	sem_wait(&g_serial_send_sem);
	return ring_list_get_data(&g_serial_send_list, buffer, size, serial_no);
}

/*
 * �����߳�
 *
 * �ӷ����б���ȡ���ݺͶ�Ӧ�Ĵ��ڱ��
 * ��������
 */
FLXInt32 thread_func_serial_send(void *param)
{
	int len, serial_no;
	unsigned char buffer[256];

	while(1)
	{
		len = get_fpga_serial_send_data(buffer, 256, &serial_no);
		if(len > 0)
		{
			if(fpga_serial_write(serial_no, buffer, len))
			{
				printf(">S%d = %d\n", serial_no, len);
			}
			else
			{
				printf("%s >S%d send failed.\n", __FUNCTION__, serial_no);
			}
		}
	}
	return 0;
}

int append_data_to_infrared_send_list(short infrared_no, const unsigned char *buffer, int size)
{
	int result;
	result = ring_list_append_data(&g_irda_send_list, buffer, size, infrared_no);
	if(result)
	{
		if(sem_post(&g_irda_send_sem) != 0)
		{
			printf("%s sem_post error, %d %s\n", __FUNCTION__, errno, strerror(errno));
		}
	}
	else
	{
		printf("%s ring_list_append_data error\n", __FUNCTION__);
	}

	return result;
}
int send_data_to_comm_infrared(short infrared_no, const unsigned char *buffer, int data_len)
{
	if(!device_is_initialized()) return 0;
	return append_data_to_infrared_send_list(infrared_no, buffer, data_len);
}

void *thread_func_infrared_send(void *param)
{
	int len, infrared_no;
	unsigned char buffer[4096];

	while(1)
	{
		sem_wait(&g_irda_send_sem);
		len = ring_list_get_data(&g_irda_send_list, buffer, 4096, &infrared_no);
		if(len > 0)
		{
			if(devc_fpga_irda_write_data((short)infrared_no, buffer, len))
			{
				//printf(">IR%d = %d\n", infrared_no, len);

				//sleep(1);
				//output_byte_array(buffer, len, len);
			}
			else
			{
				printf("%s >IR%d send failed.\n", __FUNCTION__, infrared_no);
			}
		}
	}
	return NULL;
}

int send_data_to_comm_relay(int relay_no, int b_status)
{
	if(!device_is_initialized()) return 0;
	return devc_fpga_relay_write_data(relay_no, b_status);
}



/*
 * ��ʼ������
 * �����е�8�����ڣ���Ĭ�ϵĲ����ʵȲ�����ʼ��
 * ��ʼ�����մ������ݵĻ���
 * ��8������ȫ������select�ļ�������
 */
int device_manager_init(serial_port_recv_data callback)
{
	FLXThread pid;

	g_recv_data_callback = callback;
	fpga_serial_info_init();
	if(fpga_serial_open_all() < 1)
	{
		fprintf(stdout, "%s : no fpga serial port be opened\n", __FUNCTION__);
		return 0;
	}

	ring_list_init(&g_serial_send_list);
	if(sem_init(&g_serial_send_sem, 0, 0) != 0)
	{
		printf("sem_init error. %s\n", strerror(errno));
		return 0;
	}
	ring_list_init(&g_irda_send_list);
	ring_list_check_point(&g_irda_send_list, 5);
	if(sem_init(&g_irda_send_sem, 0, 0) != 0)
	{
		printf("sem_init error. %s\n", strerror(errno));
		return 0;
	}


	if (0 ==thread_create(&pid, NULL, (void *)thread_func_select, NULL))
	{
		if(thread_create(&pid, NULL, (void *)thread_func_serial_send, NULL) != 0)
		{
			printf("%s : thread thread_func_serial_send create error\n", __FUNCTION__);
			return 0;
		}

		if(thread_create(&pid, NULL, (void *)thread_func_recv, NULL) != 0)
		{
			printf("%s : thread thread_func_recv create error\n", __FUNCTION__);
			return 0;
		}

		if(thread_create(&pid, NULL, (void *)thread_func_infrared_send, NULL) != 0)
		{
			printf("%s : thread thread_func_infrared_send create error\n", __FUNCTION__);
			return 0;
		}
	}
	else
	{
		printf("%s : thread thread_func_select create error\n", __FUNCTION__);
		return 0;
	}

	g_b_device_initialized = 1;
	printf("%s OK\n", __FUNCTION__);
	return 1;
}
