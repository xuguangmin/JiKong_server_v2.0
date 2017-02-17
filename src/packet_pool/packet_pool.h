/*
 * packet_pool.h
 *
 *  Created on: 2013-3-27
 *      Author: flx
 */

#ifndef __PACKET_POOL_H__
#define __PACKET_POOL_H__

#include "ccc_packet.h"

extern int packet_pool_init();
extern void packet_pool_release();
extern CCCPACKET *get_packet_from_packet_pool();

#endif /* __PACKET_POOL_H__ */
