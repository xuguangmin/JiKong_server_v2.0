//*************************************************************************
//
// Copyright (C) 2010-2100, PHILISENSE TECHNOLOGY CO
//
//
//*************************************************************************
////////////////////////////////////////////////////////////////////////////////////
//文件名称：dec_serial_port.h
//
//文件说明：
//
//版本：	v 1.0
//
//创建时间: 2010/12/20 17:36
//
//创建人：	chen zhi tao
//
//修改人：	XX
//
//修改时间：XXXX年x月X日
//
//修改内容：
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