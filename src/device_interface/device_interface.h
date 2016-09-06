/*
 * device_interface.h
 *
 *  Created on: 2013Äê11ÔÂ20ÈÕ
 *      Author: flx
 */

#ifndef __DEVICE_INTERFACE_H__
#define __DEVICE_INTERFACE_H__


extern int devi_init_serial_port(int serial_no, int iRate, int dataBit, int stopBit, int parity);
extern int devi_release_serial_port(int serial_no);
extern int devi_serial_port_write_data(int serial_no, unsigned char *pcData, int iLen);
extern int devi_irda_write_data(short infrared_no, int key);
extern int devi_wol_write_data(const char *mac_addr, const unsigned char *password, int password_len);
extern int devi_relay_write_data(int relay_no, int b_status);

extern int devi_network_http(const char *url_http);
extern int devi_network_onvif(const char *uri, const char *body);
extern int devi_init_telnet(int controlId, const char *ip_address, int port);
extern int devi_release_telnet(int controlId);
extern int devi_telnet_write_data(int controlId, const unsigned char *data, int data_len);

extern int uie_sleep(int mesc);
extern int uie_ctrl_enable(int ctrlId);
extern int uie_ctrl_disable(int ctrlId);
extern int uie_ctrl_visual(int ctrlId);
extern int uie_ctrl_unvisual(int ctrlId);
extern int uie_ctrl_set_value_int(int ctrlId, int change);
extern int uie_ctrl_set_checked(int ctrlId);
extern int uie_ctrl_set_not_checked(int ctrlId);

#endif /* __DEVICE_INTERFACE_H__ */
