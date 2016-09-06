/*
 * sqlite_key.h
 *
 *  Created on: 2013-5-17
 *      Author: flx
 */

#ifndef __SQLITE_KEY_H__
#define __SQLITE_KEY_H__

/* 数据表 main */
#define SQLITE_TABLE_MAIN         "main"
/* col name */
#define TABLE_MAIN_COL_KEY        "key"
#define TABLE_MAIN_COL_VALUE      "value"
/* key name */
#define SQLITE_KEY_SYSTEM_ID      "systemId"
#define SQLITE_KEY_DEVICE_ID      "deviceId"
#define SQLITE_KEY_HOST_NAME      "hostName"
#define SQLITE_KEY_IP_TYPE        "ipType"
#define SQLITE_KEY_PORT_1         "port1"
#define SQLITE_KEY_PORT_2         "port2"
#define SQLITE_KEY_IP_ADDRESS     "ipAddr"
#define SQLITE_KEY_IP_MASK        "mask"
#define SQLITE_KEY_GATE_WAY       "gateWay"
#define SQLITE_KEY_DNS_SUFFIX     "dnsSuffix"
#define SQLITE_KEY_DOMAIN_1       "domain1"
#define SQLITE_KEY_DOMAIN_2       "domain2"
#define SQLITE_KEY_SERVER_TYPE    "serverType"
#define SQLITE_KEY_SERVER_IP      "serverIp"
#define SQLITE_KEY_SERVER_PORT    "serverPort"
#define SQLITE_KEY_SERVER_DNS     "serverDns"
#define SQLITE_KEY_IP_ADDRESS2    "ipAddr2"
#define SQLITE_KEY_IP_MASK2       "mask2"
#define SQLITE_KEY_GATE_WAY2      "gateWay2"

/* 数据表 updown */
#define SQLITE_TABLE_UPDOWN       "fileNameTab" /* updown */
/* col name */
#define TABLE_UPDOWN_COL_NAME     "name"
#define TABLE_UPDOWN_COL_VALUE    "value"
/* key name */
#define SQLITE_KEY_FILE_ZIP       "1"
#define SQLITE_KEY_FILE_SO        "2"
#define SQLITE_KEY_FILE_LUA       "3"
#define SQLITE_KEY_FILE_DB        "4"

#endif /* __SQLITE_KEY_H__ */
