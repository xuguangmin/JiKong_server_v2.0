#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

#include "config_info.h"

#define SQLITE_DB_APP_CONFIG    "./test.db"

extern int  load_configure(const char *config_name);
extern void close_configure();

/* test db */
extern int get_cfg_server(SERVER_CONFIG *lp_server_config);
extern int get_cfg_info_string(char *cfg_string, int size);
extern int modify_cfg_info_all(char *cfg_string, int size);
extern int modify_cfg_ip_address_ex(int lan_no, const char *ip_addr, const char *net_mask);
extern void print_server_info();

/* test db res file */
extern int new_config_file(int protocol_file_type, const char *filename_tx, const char *filename);
extern int update_config_file();
extern int delete_upload_file();
extern const char *get_res_filename(unsigned char protocol_file_type);
extern const char *get_ui_event_so_filename();

/* irda db */
extern void close_infrared_file();
extern int get_irda_data(int key, char **irda);

#endif  /* __CONFIGURE_H__ */
