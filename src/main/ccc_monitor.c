/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : ccc_monitor.c
  作者    : chen zhi tao
  生成日期 : 2011年6月9日星期四

  版本    : 1.0
  功能描述 : 监控程序，集控服务异常自动重启

  修改历史 :
              日    期  : 2012年8月
			  作    者  : wang ning
			  修改内容   : 创建文件

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
			ret = execvp("./ccc_server", NULL);//system函数
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

