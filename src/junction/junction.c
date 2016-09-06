/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �ļ����� : junction.c
  ����    : ���Ӹ�
  �������� : 2013-4-23

  �汾    : 1.0
  �������� : ���ļ���Ϊ��������ĺ��ģ����������������еĹ���ģ�顣

  �޸���ʷ :

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ccc_config.h"
#include "util_log.h"

#include "data_pool/ccc_data_pool.h"
#include "tcp_server/connect_manager.h"
#include "ui_library/ui_library_wrapper.h"
#include "device_controller/device_manager.h"
#include "protocol_adapter/protocol_adapter.h"

static SERVER_CONFIG  g_server_config = {0, 0, 0, NULL, 0};

int stop_ccc()
{
	network_manager_close();
	release_ui_library();
	if(g_server_config.dst_server_ip) free(g_server_config.dst_server_ip);
	return 1;
}

int start_ccc()
{
	if(!get_cfg_server(&g_server_config))
	{
		printf("get server configure error.\n");
		return 0;
	}

	if(!protocol_adapter_init(g_server_config.server_type))
		return 0;

	if(!data_pool_init(callback_recv_data_from_data_pool))
		return 0;

	if(!connect_manager_init(callback_by_connect_manager))
		return 0;

	if(!network_manager_init(&g_server_config, callback_recv_data_from_network))
		return 0;



#ifndef CCC_COMPILE_VERSION_X86
	if(!device_manager_init(callback_recv_data_from_serial))
		return 0;

	if (!initialize_ui_library(get_ui_event_so_filename()))
	{
		CCC_LOG_OUT("initialize_ui_library() error\n");
	}
#endif
	return 1;
}

