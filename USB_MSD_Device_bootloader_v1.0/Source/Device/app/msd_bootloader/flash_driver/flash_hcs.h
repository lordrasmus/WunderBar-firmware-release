/********************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
**************************************************************************//*!
 *
 * @file flash_hcs.h
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief flash driver header file
 *
 *****************************************************************************/
#ifndef _FLASH_HCS_H_
#define _FLASH_HCS_H_
/*********************************** Includes ***********************************/

/*********************************** Macros ************************************/

/*********************************** Defines ***********************************/
/* error code */
#define Flash_OK           0x00
#define Flash_FACCERR      0x01
#define Flash_FPVIOL       0x02
#define Flash_NOT_ERASED   0x04
#define Flash_CONTENTERR   0x08
#define Flash_NOT_INIT     0xFF

/* flash commands */
#define FlashCmd_EraseVerify    0x05
#define FlashCmd_Program        0x20
#define FlashCmd_BurstProgram   0x25
#define FlashCmd_SectorErase    0x40
#define FlashCmd_MassErase      0x41


/********************************** Constant ***********************************/

/*********************************** Variables *********************************/

/*********************************** Prototype *********************************/
extern void Flash_Init(unsigned char FlashClockDiv);
extern unsigned char Flash_SectorErase(unsigned char *FlashPtr);
extern unsigned char Flash_ByteProgram(unsigned char *FlashStartAdd,unsigned char *DataSrcPtr,unsigned short NumberOfBytes);
void SpSub(void);
void SpSubEnd(void);

#endif /* _FLASH_HCS_H_ */
/* EOF */