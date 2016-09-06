/*
 * app_config.h
 *
 *  Created on: 2013-5-17
 *      Author: flx
 */

#ifndef __DB_CONFIG_H__
#define __DB_CONFIG_H__

#include "config_info.h"
#include "db_table_main.h"
#include "db_table_updown.h"

#define SQLITE_COL_LEN_MAX   100

extern int db_config_check(const char *db_name);
extern int db_config_load();
extern void db_config_close();

extern int db_config_load_main();
extern int db_config_load_updown();

extern int db_config_server_config(SERVER_CONFIG *lp_server_config);
extern int db_config_get_cfg_info_string(char *cfg_string, int size);
extern int db_config_modify_cfg_info_all(char *cfg_string, int size);
extern int db_config_get_ifconfig_script(char *buffer, int size);

extern int db_config_modify_cfg_info(const char *key_name, const char *new_value);
extern int db_config_main_update(const char *key_name, const char *new_value);
extern int db_config_updown_update(const char *key_name, const char *new_value);

extern int modify_config_table_system_id(const char *new_value);
extern int modify_config_table_device_id(const char *new_value);
extern int modify_cfg_host_name(const char *new_value);
extern int modify_cfg_ip_type(const char *new_value);
extern int modify_cfg_port_1(const char *new_value);
extern int modify_cfg_port_2(const char *new_value);
extern int modify_cfg_ip_address(const char *new_value);
extern int modify_cfg_ip_mask(const char *new_value);
extern int modify_cfg_gate_way(const char *new_value);
extern int modify_cfg_dns_suffix(const char *new_value);
extern int modify_cfg_domain_1(const char *new_value);
extern int modify_cfg_domain_2(const char *new_value);
extern int modify_cfg_server_type(const char *new_value);
extern int modify_cfg_server_ip(const char *new_value);
extern int modify_cfg_server_port(const char *new_value);
extern int modify_cfg_server_dns(const char *new_value);

extern const char *db_config_get_config(const char *key_name);
extern const char *db_config_res_filename(const char *key_name);

#endif /* __DB_CONFIG_H__ */
