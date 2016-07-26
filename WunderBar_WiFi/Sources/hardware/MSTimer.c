#include <stdio.h>
#include "Hw_modules.h"



// should be the same as period of used ISR
#define MSTIMER_MILISECOND_COUNTER_INC    100

// milliseconds counter
static unsigned long long int milliseconds_counter;

/**
 *  @brief  Increment total milliseconds
 *
 *  Increment number of miliseconds by defined constant.
 *  Should be called from timer ISR on defined period (MSTIMER_MILISECOND_COUNTER_INC)
 *
 *  @return void
 */
void MSTimer_Increment_milliseconds(){
	 milliseconds_counter += MSTIMER_MILISECOND_COUNTER_INC;
}

/**
 *  @brief  Returns current time in milliseconds
 *
 *  Returns current millisecond counter.
 *
 *  @return 64 bit value of millisecond counter
 */
unsigned long long int MSTimerGet (){
	return milliseconds_counter;
}

/**
 *  @brief  Returns delta time from input time in ms
 *
 *  Calculates Passed time in milliseconds from given time in milliseconds.
 *
 *  @param  64 bit value of start time in ms.
 *
 *  @return 64 bit value of passed time in ms
 */
unsigned long long int MSTimerDelta(unsigned long long int timer){
	return ((unsigned int) (milliseconds_counter - timer));
}

/**
 *  @brief  Delays for desired number of milliseconds
 *
 *  Stays in while loop for desired amount of time in milliseconds.
 *
 *  @param  64 bit value of desired delay in ms
 *
 *  @return void
 */
void MSTimerDelay(unsigned long long int delay){
	unsigned long long int start_timer = MSTimerGet();
	while (MSTimerDelta(start_timer) < delay)
		;
}

/**
 *  @brief  Set current time in ms
 *
 *  Set milliseconds counter on desired value
 *
 *  @param  64 bit value of current time in ms.
 *
 *  @return void
 */
void MSTimer_SetTime(unsigned long long int time){
	milliseconds_counter = time;
}

/**
 *  Returns string of current time in ms
 *
 *  @param  Pointer to text string where current time will be returned
 *
 *  @return void
 */
void MSTimer_GetSystemTimeStr(char* txt){
	sprintf(txt, "%ld", milliseconds_counter);
}
