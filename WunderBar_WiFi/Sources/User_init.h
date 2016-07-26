/** @file   User_init.h
 *  @brief  File contains function for user modules initialization
 *          and stack initialization
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <Common_Defaults.h>


#define RESET_MODULE_DELAY     			200        // delay while rst is in low state during reset
#define SLEEP_COUNTDOWN_MS   			500

// public functions

/**
*  @brief  Returns ExtIntEn flag
*
*  @return 1  - If ext int is ready to be used
*/
char Check_ExtIntEn();

/**
*  @brief  Checks of the wifi module is ready
*
*  On power up GS module will drive RST pin high to indicate ready state.
*
*  @return Sstate of the rst pin
*/
int  Chec_Wifi_Rst_Stable();

/**
*  @brief  Global User init
*
*  Reset all connected modules and call initialize routines
*
*  @return void
*/
void Global_Peripheral_Init();

/**
*  @brief  Reset Gain Span module by driving ExtReset pin low
*
*  @return void
*/
void Reset_Wifi();

/**
*  @brief  Reset Nordic chip by driving NRset pin low
*
*  @return void
*/
void Reset_Nordic();

/**
*  @brief  Check if the master module ID is empty
*
*  Check if the master module exists in flash
*  After onboarding process it will be loaded and used later on.
*
*  @param  Pointer to wcfg struct (Loaded from flash)
*
*  @return 1  - if master module id exists, 0  - missing
*/
char Check_MainBoard_ID_Exists(wcfg_t* wcfg);

/**
*  Count down sleep counter
*
*  If mqtt is running and no actions are performed countdown sleep counter.
*  Should be called from timer interrupt (TI2)
*
*  @return void
*/
void Sleep_DecrementCountDown();

/**
*  @brief  Check if sleep conditions are met.
*
*  IF sleep condition is met (no actions for last SLEEP_COUNTDOWN_MS milliseconds)
*  initiate sleep mode on kinetis.
*
*  @return void
*/
void Sleep_CheckConditions();

/**
*  @brief  Resotre sleep countdown counter
*
*  Should be called when an action is performed to avoid going into sleep mode.
*
*  @return void
*/
void Sleep_Restore_Countdown();
