#ifndef __DEVI_SERIAL_PORT_H__
#define __DEVI_SERIAL_PORT_H__

extern FLXInt32 devi_init_serial_port(FLXInt32 iPort,FLXInt32 iRate, FLXInt32 dataBit,
		                       FLXInt32 stopBit, FLXInt32 chkBit);
extern FLXInt32 devi_release_serial_port(FLXInt32 iSerialId);
extern FLXInt32 devi_serial_port_write_data(FLXInt32 iSerialId, FLXByte *pcData, FLXInt32 iLen);

//extern void devi_serial_port_recv_callback(serial_port_recv_data callback);

#endif  /* __DEVI_SERIAL_PORT_H__ */
