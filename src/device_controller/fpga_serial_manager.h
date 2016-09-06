#ifndef __FPGA_SERIAL_MANAGER_H__
#define __FPGA_SERIAL_MANAGER_H__

#include "FLXCommon/flxtypes.h"
#include "FLXCommon/flxthread.h"
#include "ring_list_buf.h"


#define FPGA_SERIAL_COUNT      8

extern void fpga_serial_info_init();
extern int  fpga_serial_open_all();
extern int  enable_fpga_serial_rest(int serial_no);
extern int  modify_fpga_serial_config(int serial_no, int baud_rate, unsigned char data_bits, unsigned char parity, unsigned char stop_bits);
extern fd_set get_fpga_serial_fd_set(int *maxfd);
extern void set_fpga_serial_recv_packet_delay(int serial_no, unsigned int delay);
extern int get_serial_stat_info(char *buffer, int buf_size);

/* send data */
//extern int  append_data_to_send_list(int serial_no, const unsigned char *buffer, int size);
//extern int  get_fpga_serial_send_data(unsigned char *buffer, int size, int *serial_no);
extern int  fpga_serial_write(int serial_no, unsigned char *buffer, int size);

/* recv data */
extern void fpga_serial_read(int fd);
extern int  get_fpga_serial_recv_data(int serial_no, unsigned char *buffer, int size);


#endif  /* __FPGA_SERIAL_MANAGER_H__ */
