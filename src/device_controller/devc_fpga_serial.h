#ifndef __DEVC_FPGA_SERIAL_H__
#define __DEVC_FPGA_SERIAL_H__

#include "FLXCommon/flxtypes.h"

FLXInt32 devc_fpga_serial_open(int serial_no, FLXInt32 *handle);
FLXInt32 devc_fpga_serial_close(FLXInt32 handle);
FLXInt32 devc_fpga_serial_modify(FLXInt32 fd, int baud_rate, unsigned char data_bits,
                                      unsigned char parity, unsigned char stop_bits);

FLXInt32 devc_fpga_serial_write_data(FLXInt32 handle, FLXByte *data, FLXInt32 len);
FLXInt32 devc_fpga_serial_read_data(FLXInt32 handle, FLXByte *data, FLXInt32 len);

#endif  /* __DEVC_FPGA_SERIAL_H__ */
