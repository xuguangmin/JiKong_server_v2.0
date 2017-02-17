/*
 * sysconfig.h
 *
 *  Created on: 2012-11-5
 *      Author: flx
 */

#ifndef __SYS_CONFIG_H__
#define __SYS_CONFIG_H__

extern unsigned int get_ipaddr(const char *str_eth);
extern unsigned int get_netaddr(const char *str_eth);
extern int set_ipaddr_netmask(const char *ethname, const char* iphost, const char* netmask);
extern int set_ipaddr_netmask2(int lan_no, const char* ipaddr, const char* netmask);
extern int modify_ip_address_script(const char *ip_addr, const char *net_mask);
extern int modify_ifconfig_eth_script(const char *content);
extern int set_system_time(int year, int month, int day, int hour, int minute, int second);

extern void reboot_device();
extern int str_to_ip_address(char *srcstr);

#endif /* __SYS_CONFIG_H__ */
