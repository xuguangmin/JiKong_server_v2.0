/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞利信科技股份有限公司

 ******************************************************************************
  文件名称 : ccc_main.c
  作者    :
  生成日期 :

  版本    : 1.0
  功能描述 : 服务程序入口

  修改历史 : 来自2013-07-18的alpha版

******************************************************************************/

/* /home/flx/developer/workspace_c/ccc_server */
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "ccc_config.h"
#include "configure.h"
#include "indicator_lamp.h"
#include "util_log.h"
#include "app_console.h"
#include "junction/junction.h"


static void print_trace(void)
{
	int size;
	char **symbols;
	void *array[1024];

	size = backtrace(array, 1024);
	fprintf(stderr, "segmentation fault\n backtrace stack deep %d\n", size);

	symbols = backtrace_symbols(array, size);
	if(symbols)
	{
		int k;
		for(k = 0; k < size; ++k)
		{
			fprintf(stderr, "%d: %s\n", k, symbols[k]);
		}
		free(symbols);
	}
	fprintf(stderr, "\n");
	exit(-1);
}

static void debug_backtrace(void)
{
	panel_led_off();
	print_trace();
}

static void close_release_res(void)
{
	stop_ccc();
	close_configure();
	panel_led_off();

	// kill(getpid(), SIGINT);
	exit(0);
}


int main(int argc, char *argv[])
{
	signal(SIGSEGV, (void *)debug_backtrace);    /* catch Segmentation fault signal */
	signal(SIGINT,  (void *)close_release_res);  /* reg ctrl+c signal */

	if(!load_configure(SQLITE_DB_APP_CONFIG))
	{
		CCC_LOG_OUT("%s load_configure() error.\n", __FUNCTION__);
		return -1;
	}

	if(!start_ccc())
	{
		CCC_LOG_OUT("%s start_ccc error!\n", __FUNCTION__);

		if(system("./cpTest.sh") < 0)
		{
			printf("reboot system error.\n");
		}
		printf("SYSTEM ERROR. PLEASE CHEAK IT!\n");

		return -1;
	}

#ifndef CCC_COMPILE_VERSION_X86
	if(panel_led_on()) CCC_LOG_OUT("the front panel led on.\n");
#endif
	while(1)
	{
		app_console_start();
	}
	close_release_res();                        /* 释放链接,关闭数据库 */
	return 0;
}
