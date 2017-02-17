//*************************************************************************
//
// Copyright (C) 2010-2100, PHILISENSE TECHNOLOGY CO
//
//
//*************************************************************************
////////////////////////////////////////////////////////////////////////////////////
//�ļ����ƣ�dec_serial_port.h
//
//�ļ�˵����
//
//�汾��	v 1.0
//
//����ʱ��: 2010/12/20 17:36
//
//�����ˣ�	chen zhi tao
//
//�޸��ˣ�	XX
//
//�޸�ʱ�䣺XXXX��x��X��
//
//�޸����ݣ�
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef dec_serial_port_h__
#define dec_serial_port_h__

#include "sys_types/flxtypes.h"

#ifdef DEBUG
#define SERIAL_PORT_DEBUG(args)	(printf("[dec] "), printf args)
#else
#define SERIAL_PORT_DEBUG(args)
#endif

FLXInt32 dec_init_serial_port(FLXChar *cDev, FLXInt32 iRate, FLXInt32 *iHandle);
FLXInt32 dec_serial_port_write_data(FLXInt32 iHandle, FLXChar *pcData, FLXInt32 iLen);
FLXInt32 dec_serial_port_read_data(FLXInt32 iHandle, FLXChar *pcData, FLXInt32 iLen);
FLXInt32 dec_serial_port_close(FLXInt32 iHandle);

#endif // dec_serial_port_h__