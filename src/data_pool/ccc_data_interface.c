#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>

#include "ccc_data_interface.h"
/*
static sem_t      g_irda_send_sem;

int append_data_to_infrared_send_list(short infrared_no, const unsigned char *buffer, int size)
{
	int result = 1;
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
	return append_data_to_infrared_send_list(infrared_no, buffer, data_len);
}

void *thread_func_infrared_send(void *param)
{
	while(1)
	{
		sem_wait(&g_irda_send_sem);
	}
	return NULL;
}
*/
int data_interface_init(data_interface_callback callback)
{
	/*
	FLXThread pid;

	if(sem_init(&g_irda_send_sem, 0, 0) != 0)
	{
		printf("sem_init error. %s\n", strerror(errno));
		return 0;
	}

	if(thread_create(&pid, NULL, (void *)thread_func_infrared_send, NULL) != 0)
	{
		printf("%s : thread thread_func_infrared_send create error\n", __FUNCTION__);
		return 0;
	}
*/
	//g_recv_data_callback = callback;
	return 1;
}
