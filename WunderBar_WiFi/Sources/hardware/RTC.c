#include <stdio.h>
#include "Hw_modules.h"
#include <RTC_PDD.h>


/**
 *  @brief  Set RTC time in milliseconds
 *
 *  Set RTC time in ms. Load time in RTC registers and Enable counter.
 *
 *  @param  64 bit value of current time in ms.
 *
 *  @return void
 */
void RTC_SetTime(unsigned long long int milisecs){
	unsigned int Seconds, Prescaler;

	Seconds = milisecs / 1000;
	Prescaler = (( milisecs % 1000 ) * 32768 + 999) / 1000;   	// round up

	RTC_PDD_EnableCounter(RTC_BASE_PTR, PDD_DISABLE);         	/* Disable counter */
	RTC_PDD_WriteTimePrescalerReg(RTC_BASE_PTR, Prescaler); 	/* Set prescaler */
	RTC_PDD_WriteTimeSecondsReg(RTC_BASE_PTR, Seconds); 		/* Set seconds counter */
	RTC_PDD_EnableCounter(RTC_BASE_PTR, PDD_ENABLE); 			/* Enable counter */
}

/**
 *  @brief  Get time from RTC module in milliseconds
 *
 *  Read RTC registers and return rtc time in milliseconds.
 *
 *  @return 64 bit value of current time in ms.
 */
unsigned long long int RTC_GetTime(){
	unsigned int Seconds, Prescaler;

	Seconds = RTC_PDD_ReadTimeSecondsReg(RTC_BASE_PTR); 		/* current seconds */

	Prescaler = RTC_PDD_ReadTimePrescalerReg(RTC_BASE_PTR);		/* current prescaler for milliseconds */

	return ( ((unsigned long long int) Seconds * 1000) + (Prescaler * 1000) / 32768 );
}

/**
 *  @brief  Set alarm at desired offset in seconds
 *
 *  Enable Alarm in RTC module and set it at desired offset in seconds.
 *
 *  @param  Offset in seconds
 *
 *  @return void
 */
void RTC_SetAlarm(unsigned int timeOffset){
	RTC_PDD_WriteTimeAlarmReg(RTC_BASE_PTR, (unsigned int) (RTC_PDD_ReadTimeSecondsReg(RTC_BASE_PTR) + timeOffset));
}

/*
 *  @brief  Returns string of current RTC time in ms
 *
 *  @param  Pointer to text string where current time will be returned
 *
 *  @return void
 */
void RTC_GetSystemTimeStr(char* txt){
	sprintf(txt, "%ld", RTC_GetTime());
}
