/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : ccc_monitor.c
  ����    : chen zhi tao
  �������� : 2011��6��9��������

  �汾    : 1.0
  �������� : ��س��򣬼��ط����쳣�Զ�����

  �޸���ʷ :
              ��    ��  : 2012��8��
			  ��    ��  : wang ning
			  �޸�����   : �����ļ�

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	pid_t pid;
	int status, ret;

	while (1)
	{
		pid = fork();
		if (pid == -1)
		{
			continue;
		}
		else if (pid == 0)
		{
			ret = execvp("./ccc_server", NULL);//system����
			if(ret < 0)
			{
				printf("execvp error, errno:%d error:%s\n", errno, strerror(errno));
				continue;
			}
			exit(0);
		}
		else if (pid > 0)
		{
			pid = wait(&status);
			printf("hysbjks_server exit\n");
		}
	}
}

