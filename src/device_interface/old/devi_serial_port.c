/******************************************************************************

                  ��Ȩ���� (C), 2001-2020, ���������ſƼ��ɷ����޹�˾

 ******************************************************************************
  �� �� ��   : devi_serial_port.c
  �� �� ��   : ����
  ��    ��   : chen zhi tao
  ��������   : 2011��3��30��������
  ����޸�   :
  ��������   : �����豸�ӿ�
  �����б�   :
              devi_init_serial_port
              devi_release_serial_port
              devi_serial_port_write_data
  �޸���ʷ   :
  1.��    ��   : 2011��3��30��������
    ��    ��   : chen zhi tao
    �޸�����   : �����ļ�

******************************************************************************/

#include <stdio.h>
#include "device_controller/comm_manager.h"


static int bcomm_manager = 1;       /* �Ƿ����˳�ʼ�� */

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
