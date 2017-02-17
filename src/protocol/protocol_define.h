/*
 * protocol_define.h
 *
 *  Created on: 2012-10-26
 *      Author: flx
 */

#ifndef __PROTOCOL_DEFINE_H__
#define __PROTOCOL_DEFINE_H__

/* protocol command set */
#define PCS_HEART_BEAT                  0x00
#define PCS_CLIENT_LOGIN                0x10
#define PCS_UPLOAD_FILE                 0x11
#define PCS_DOWNLOAD_FILE               0x12
#define PCS_CLIENT_EVENT                0x19
#define PCS_SERVER_INFO                 0x21
#define PCS_CTRL_PROPERTY               0x39

/* protocol command set extend */
#define PCS_EX_OK 		                0xFE
#define PCS_EX_ERR		                0xFF

#define PROTOCOL_FILE_TYPE_ZIP          0X01
#define PROTOCOL_FILE_TYPE_SO           0X02
#define PROTOCOL_FILE_TYPE_LUA          0X03
#define PROTOCOL_FILE_TYPE_IRDA         0x04
#define PROTOCOL_FILE_TYPE_ZIP_IOS      0x05

#define PROTOCOL_DATA_SIZE_MAX          1024           /* 集控协议包的最大长度 */
#define PROTOCOL_DATA_LEN_MAX           (PROTOCOL_DATA_SIZE_MAX - 16)

#define CTRL_PROPERTY_ENABLE            0x01
#define CTRL_PROPERTY_VISUAL            0x02

#endif /* __PROTOCOL_DEFINE_H__ */
