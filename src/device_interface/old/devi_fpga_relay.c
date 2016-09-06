/******************************************************************************

  版权所有 (C), 2001-2020, 北京飞立信科技股份有限公司

 ******************************************************************************
  文 件 名   : devi_fpga_replay.c
  版 本 号   : 初稿
  作    者   : chen zhi tao
  生成日期   : 2011年3月30日星期三
  最近修改   :
  功能描述   : 串口设备接口
  
  修改历史   :
  1.日    期   : 2011年3月30日星期三
    作    者   : chen zhi tao
    修改内容   : 创建文件

******************************************************************************/

#include "FLXCommon/flxtypes.h"
#include "../device_controller/devc_fpga_relay.h"

FLXInt32 devi_relay_write_data(FLXChar port, FLXBool status)
{
	if (devc_fpga_relay_write_data(port, status) != 0)
		return -1;

	return 0;
}
