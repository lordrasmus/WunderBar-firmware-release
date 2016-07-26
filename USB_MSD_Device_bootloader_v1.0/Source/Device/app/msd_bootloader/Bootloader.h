/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************//*!
 *
 * @file Bootloader.h
 *
 * @author
 *
 * @version
 *
 * @date 
 *
 * @brief This file is for a USB Mass-Storage Device bootloader.  This file 
 *           has the structures and definitions for the bootloader
 *
 *****************************************************************************/
#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include "types.h"
#include "derivative.h"
#if (defined __MCF52259_H__)
#define MIN_RAM1_ADDRESS        0x20000000
#define MAX_RAM1_ADDRESS        0x2000FFFF
#define MIN_FLASH1_ADDRESS      0x00000000
#define MAX_FLASH1_ADDRESS      0x0007FFFF
#define IMAGE_ADDR              ((uint_32_ptr)0xC000)
#define PROT_VALUE				0x7	  	  // Protects 0x0 - 0xBFFF
#define ERASE_SECTOR_SIZE       (0x1000)  /* 4K bytes*/
#elif (defined _MCF51JM128_H)
#define MIN_RAM1_ADDRESS        0x00800000
#define MAX_RAM1_ADDRESS        0x00803FFF
#define MIN_FLASH1_ADDRESS      0x00000000
#define MAX_FLASH1_ADDRESS      0x0001FFFF
#define IMAGE_ADDR              ((uint_32_ptr)0x0A000)
#define PROT_VALUE				0xD7	  // Protects 0x0 - 0x9FFF
#define ERASE_SECTOR_SIZE       (0x0400)  /* 1K bytes*/
#elif (defined MCU_MK60N512VMD100)
#define MIN_RAM1_ADDRESS        0x1FFF0000
#define MAX_RAM1_ADDRESS        0x20010000
#define MIN_FLASH1_ADDRESS      0x00000000
#define MAX_FLASH1_ADDRESS      0x0007FFFF
#define IMAGE_ADDR              ((uint_32_ptr)0xC000)
#define PROT_VALUE0				0xFF 	  // Protects 0x0 - 0xBFFF
#define PROT_VALUE1				0xFF 	  // Protects 0x0 - 0xBFFF
#define PROT_VALUE2				0xFF 	  // Protects 0x0 - 0xBFFF
#define PROT_VALUE3				0xF8 	  // Protects 0x0 - 0xBFFF
#define ERASE_SECTOR_SIZE       (0x800)  /* 2K bytes*/
#elif (defined MCU_MK64F12)
#define MIN_RAM1_ADDRESS        0x1FFF0000
#define MAX_RAM1_ADDRESS        0x20030000
#define MIN_FLASH1_ADDRESS      0x00000000
#define MAX_FLASH1_ADDRESS      0x00100000
#define IMAGE_ADDR              ((uint_32_ptr)0x18000)
#define PROT_VALUE0				0xFF 	  // Protects 0x0 - 0x10000
#define PROT_VALUE1				0xFF 	  // Protects 0x0 - 0x10000
#define PROT_VALUE2				0xFF 	  // Protects 0x0 - 0x10000
#define PROT_VALUE3				0xFC 	  // Protects 0x0 - 0x10000
#define ERASE_SECTOR_SIZE       (0x1000)  /* 4K bytes*/
#define CONFIG_ADDR             ((uint_32_ptr)0x10000)
#elif (defined MCU_MK24F12)
#define MIN_RAM1_ADDRESS        0x1FFF0000
#define MAX_RAM1_ADDRESS        0x20030000
#define MIN_FLASH1_ADDRESS      0x00000000
#define MAX_FLASH1_ADDRESS      0x00100000
#define IMAGE_ADDR              ((uint_32_ptr)0x18000)
#define PROT_VALUE0				0xFF 	  
#define PROT_VALUE1				0xFF 	  
#define PROT_VALUE2				0xFF 	  
#define PROT_VALUE3				0xFF 	  // Protects 0x0 - 0x10000
#define ERASE_SECTOR_SIZE       (0x1000)  /* 4K bytes*/
#define CONFIG_ADDR             ((uint_32_ptr)0x10000)
#endif

#define S19_RECORD_HEADER       0x53300000
/* DES space less than this is protected */
#define FLASH_PROTECTED_ADDRESS (int)&IMAGE_ADDR[0]
#define FLASH_ADDR_OFFSET       0x44000000
#define First4Bytes             4

/* RAM locations for USB Buffers */
#define USB_BUFFER_START        0x20000400
#define MSD_BUFFER_SIZE         512
#define BDT_SIZE                16
#define ICP_BUFFER_SIZE         64

/* File types */
#define UNKNOWN                 0
#define RAW_BINARY              1
#define CODE_WARRIOR_BINARY     2
#define S19_RECORD              3

void _Entry(void) ;
#if (defined __MCF52259_H__) || (defined _MCF51JM128_H)
	extern unsigned long far __SP_INIT[];
#elif (defined MCU_MK60N512VMD100)	
	extern long __SP_INIT;
#elif (defined MCU_MK64F12)	
	extern long __SP_INIT;
#elif (defined MCU_MK24F12)	
	extern long __SP_INIT;
#endif
	
/* Bootloader Status */
#define BootloaderReady         0
#define BootloaderS19Error      1
#define BootloaderFlashError    2
#define BootloaderSuccess       3
#define BootloaderStarted       4

#define  FLASH_IMAGE_SUCCESS    0
#define  FLASH_IMAGE_ERROR      1

#define BUFFER_LENGTH           (1024)    /* 1K bytes*/

uint_8 FlashApplication(uint_8* arr,uint_32 length);
#if (defined __MCF52259_H__) || (defined _MCF51JM128_H)
	asm __declspec(register_abi) void _startup(void);
#elif (defined MCU_MK60N512VMD100)	
	extern int _startup(void);
#elif (defined MCU_MK64F12)	
	extern int _startup(void);
#elif (defined MCU_MK24F12)	
	extern int _startup(void);
#endif
	
#endif
/* EOF */
