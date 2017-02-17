/*
 * ccc_command.h
 *
 *  Created on: 2012-10-28
 *      Author: jiayg
 */

#ifndef __CCC_COMMAND_H__
#define __CCC_COMMAND_H__


#define CCC_ID_INVALID               0
#define CCC_ID_HELP                  1
#define CCC_ID_LIST                  2
#define CCC_ID_SHELL                 3
#define CCC_ID_ETHIP                 4
#define CCC_ID_REBOOT                5
#define CCC_ID_EXIT                  6
#define CCC_ID_PS                    7
#define CCC_ID_VERSION               8
#define CCC_ID_SERIAL                9
#define CCC_ID_SERIAL_H              10
#define CCC_ID_WOL                   11
#define CCC_ID_ETHIP_1               12
#define CCC_ID_ETHIP_2               13
#define CCC_ID_RELAY                 14
#define CCC_ID_TEST                  15
#define CCC_ID_DATE                  16

extern int ccc_command(int command_id, int argc, char *argv[]);

#endif /* __CCC_COMMAND_H__ */
