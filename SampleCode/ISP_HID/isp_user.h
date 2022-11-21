/**************************************************************************//**
 * @file     isp_user.h
 * @brief    ISP Command header file
 *
 * @note
 * @copyright SPDX-License-Identifier: Apache-2.0
 * @copyright Copyright (C) 2021 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef ISP_USER_H
#define ISP_USER_H

#define RST_ADDR_LDROM                      (0)
#define RST_ADDR_APROM                      (1)
#define RST_SEL_NVIC                        (0)
#define RST_SEL_CPU                         (1)
#define RST_SEL_CHIP                        (2)

#define LDROM_DEBUG(format, args...) 		printf("\033[1;36m" "[LDROM]" format "\033[0m", ##args)


#define FW_VERSION 0x33 // Revised Ver. Remove useless isp command to reduce code size

/*
		flash page : 2K (0x0800)
		boot loader in LDROM : 4K 
		boot loader in APROM : 2K*4 (pos : start from 0x1D000)
		
		dataflash : 4K , 0x1F000 		
		application code checksum : 0x1CFFC

*/
#define APROM_APPLICATION_SIZE      		0x0001D000UL

#include "fmc_user.h"
#include <string.h>

#define CMD_UPDATE_APROM      0x000000A0
#define CMD_UPDATE_CONFIG     0x000000A1
#define CMD_READ_CONFIG       0x000000A2
#define CMD_ERASE_ALL         0x000000A3
#define CMD_SYNC_PACKNO       0x000000A4
#define CMD_GET_FWVER         0x000000A6
#define CMD_RUN_APROM         0x000000AB
#define CMD_RUN_LDROM         0x000000AC
#define CMD_RESET             0x000000AD
#define CMD_CONNECT           0x000000AE
#define CMD_GET_DEVICEID      0x000000B1
#define CMD_UPDATE_DATAFLASH  0x000000C3
#define CMD_RESEND_PACKET     0x000000FF

#define V6M_AIRCR_VECTKEY_DATA    0x05FA0000UL
#define V6M_AIRCR_SYSRESETREQ     0x00000004UL


extern void GetDataFlashInfo(uint32_t *addr, uint32_t *size);
extern uint32_t GetApromSize(void);
extern int ParseCmd(unsigned char *buffer, uint8_t len);
extern uint32_t g_apromSize, g_dataFlashAddr, g_dataFlashSize;

uint8_t read_magic_tag(void);
void write_magic_tag(uint8_t tag);

extern __attribute__((aligned(4))) uint8_t usb_rcvbuf[64];
extern __attribute__((aligned(4))) uint8_t response_buff[64];
#endif  // #ifndef ISP_USER_H
