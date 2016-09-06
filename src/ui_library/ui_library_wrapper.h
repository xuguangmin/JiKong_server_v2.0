#ifndef __UI_LIBRARY_WRAPPER_H__
#define __UI_LIBRARY_WRAPPER_H__

extern int initialize_ui_library(const char *ui_event_so_filename);
extern int release_ui_library();
extern int parse_control_event(int senderId, int event, char *data, int dataLen);
extern int parse_serial_recv(int serial_no, unsigned char *data, int dataLen);
extern int parse_connect_recv(int controlId, unsigned char *data, int dataLen);

#endif /* __UI_LIBRARY_WRAPPER_H__ */

