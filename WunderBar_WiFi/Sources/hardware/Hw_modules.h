
#ifndef HARDWARE_MODULES_H_
#define HARDWARE_MODULES_H_


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#include "Events.h"
#include <Cpu.h>
#include <stdbool.h>
#include <stdint.h>


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// MSTImer functions

unsigned long long int MSTimerGet ();
void MSTimerDelay(unsigned long long int delay);
unsigned long long int MSTimerDelta(unsigned long long int timer);
void MSTimer_SetTime(unsigned long long int time);
void MSTimer_GetSystemTimeStr(char* txt);
void MSTimer_Increment_milliseconds();


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// RTC

unsigned long long int RTC_GetTime();
void RTC_SetTime(unsigned long long int milisecs);
void RTC_GetSystemTimeStr(char* txt);
void RTC_SetAlarm(unsigned int timeOffset);


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// USRT functions

void GS_HAL_send(uint8_t* StrPtr, uint32_t Size);
unsigned int GS_HAL_recv(uint8_t* recvbyte, uint32_t Size, uint32_t block);
void GS_HAL_ClearBuff();


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// SPI functions

unsigned int SPI_Write(char* sendbyte, char size);
unsigned int SPI_Read( char* recvbyte, char size);
void SPI_Init();
void SPI_SetWrCompletedFlag();
void SPI_CS_Activate();
void SPI_CS_Deactivate();


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// GPIO

#define GPIO_GetButtonState()       EInt1_GetVal()
#define GPIO_LedToggle()       		Bits1_NegBit(0)
#define GPIO_LedOff()          		Bits1_ClrBit(0)
#define GPIO_LedOn()           		Bits1_SetBit(0)
#define GPIO_ClrRstWifi()			Bit2_ClrVal()
#define GPIO_SetRstWifi()			Bit2_SetVal()
#define GPIO_SetRstInputWifi()		Bit2_SetDir(false)
#define GPIO_SetRstOutputWifi()		Bit2_SetDir(true)
#define GPIO_GetRstValueWifi()		Bit2_GetVal()
#define GPIO_ClrRstNordic()			Bit1_ClrVal()
#define GPIO_SetRstNordic()			Bit1_SetVal()
#define GPIO_SetRstInputNordic()	Bit1_SetDir(false)
#define GPIO_SetRstOutputNordic()	Bit1_SetDir(true)
#define GPIO_SpiClrCS()    			Bits2_ClrBit(0)
#define GPIO_SpiSetCS()   			Bits2_SetBit(0)


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// MCU

#define CPU_System_Reset()          Cpu_SystemReset()


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// Flash image addresses

#define FLASH_CONFIG_IMAGE_ADDR 			0x00010000
#define FLASH_CERTIFICATE_IMAGE_ADDRESS  	0x00011000


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// ADC Battery Voltage Sense Channel

#define VOLTAGE_REFERENCE_BANDGAP 			1200
#define ADC_VOLTAGE_SENSE_CHANNEL    		16


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// Timer2 Interrupt period

#define TIMER2_INT_PERIOD  					100 		// defined by processor expert

#endif // HARDWARE_MODULES_H_
