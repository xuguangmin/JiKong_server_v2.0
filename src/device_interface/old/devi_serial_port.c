/******************************************************************************

                  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : devi_serial_port.c
  版 本 号   : 初稿
  作    者   : chen zhi tao
  生成日期   : 2011年3月30日星期三
  最近修改   :
  功能描述   : 串口设备接口
  函数列表   :
              devi_init_serial_port
              devi_release_serial_port
              devi_serial_port_write_data
  修改历史   :
  1.日    期   : 2011年3月30日星期三
    作    者   : chen zhi tao
    修改内容   : 创建文件

******************************************************************************/

#include <stdio.h>
#include "device_controller/comm_manager.h"


static int bcomm_manager = 1;       /* 是否做了初始化 */

FLXInt32 devi_init_serial_port(FLXInt32 iPort,FLXInt32 iRate, FLXInt32 dataBit,
		                       FLXInt32 stopBit, FLXInt32 chkBit)
{
	/*
	static int binitialize = 0;
	if(!binitialize)
	{
		binitialize = 1;
		if(!comm_manager_init())
		{
			printf("devi_init_serial_port() comm_manager_init error\n");
			return -1;
		}
		bcomm_manager = 1;
	}
*/
	if(!bcomm_manager)
		return -1;
	if(!modify_comm_config(iPort, iRate, dataBit, chkBit, stopBit))
		return -1;

	return 0;
}

FLXInt32 devi_serial_port_write_data(FLXInt32 iSerialId, FLXByte *pcData, FLXInt32 iLen)
{
	if(!bcomm_manager)
		return -1;
	if(!send_data_to_comm(iSerialId, pcData, iLen))
		return -1;

	return 0;
}
/*
void devi_serial_port_recv_callback(serial_port_recv_data callback)
{
	set_comm_recv_data_callback(callback);
}
*/
FLXInt32 devi_release_serial_port(FLXInt32 iSerialId)
{
	if(!bcomm_manager)
		return -1;
	///if(!enable_comm_rest(iSerialId))
		//return -1;

	return 0;
}
