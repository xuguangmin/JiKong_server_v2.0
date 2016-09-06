#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


static int g_fd_irda = -1;

static int devc_fpga_irda_open()
{
	int fd = open("/dev/FPGA_Jikong11", O_RDWR);
	if(fd < 0)
	{
		perror("cannot open irda!\n");
		return -1;
	}
	printf("FPGA IRDA opened!\n");
	return fd;
}

static void devc_fpga_irda_close()
{
	close(g_fd_irda);
	g_fd_irda = -1;
}

int devc_fpga_irda_write_data(short infrared_no, unsigned char *buffer, int data_len)
{
	if(!buffer || data_len <= 0)
		return 0;

	if(g_fd_irda < 0) g_fd_irda = devc_fpga_irda_open();
	if(g_fd_irda < 0)
		return 0;

	if (ioctl(g_fd_irda, infrared_no, NULL) < 0)
	{
		printf("%s ioctl cmd IOCPRINT fail.\n", __FUNCTION__);
		devc_fpga_irda_close();
		return 0;
	}

	/*write(g_fd_irda, buffer, data_len);*/
	if(write(g_fd_irda, buffer, data_len) < 0)
	{
		printf("%s write failed. %s\n", __FUNCTION__, strerror(errno));
		devc_fpga_irda_close();
		return 0;
	}
	return 1;
}
