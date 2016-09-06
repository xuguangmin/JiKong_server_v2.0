#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

typedef void (*serial_port_recv_data)(int serial_no, unsigned char *buffer, int size);

extern int device_manager_init(serial_port_recv_data callback);

extern int comm_serial_modify_config(int serial_no, int baud_rate, int data_bits, int parity, int stop_bits);
extern int comm_serial_stop(int serial_no);

extern int send_data_to_comm_serial(int serial_no, const unsigned char *buffer, int size);
extern int send_data_to_comm_infrared(short infrared_no, const unsigned char *buffer, int data_len);
extern int send_data_to_comm_relay(int relay_no, int b_status);

extern int get_device_stat_info(char *buffer, int buf_size);

#endif  /* __DEVICE_MANAGER_H__ */
