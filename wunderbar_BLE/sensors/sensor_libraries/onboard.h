
/** @file   onboard.h
 *  @brief  Contains declarations of functions for onboarding mode of sensor and corresponding macros.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#ifndef _ONBOARD_H_
#define _ONBOARD_H_

#include "types.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**@brief This enum contains possible onboard modes. */
typedef enum
{
    ONBOARD_MODE_IDLE    = 0x64,                                               /**< Onboard process is not active. */
    ONBOARD_MODE_ACTIVE  = 0x65,                                               /**< Onboard process is active. */ 
}
onboard_mode_t;

/**@brief This enum contains possible states for active onboard mode. */
typedef enum
{
    ONBOARD_STATE_RUN                 = 0,                                     /**< Start onboarding. */ 
    ONBOARD_STATE_BUTTON_DOWN         = 1,                                     /**< Detected button press event. */ 
    ONBOARD_STATE_WAIT_TO_STORE_MODE  = 2,                                     /**< Wait to store new mode to persistent memory. */
    ONBOARD_STATE_STORE_MODE          = 3,                                     /**< Storing is in progress. */
    ONBOARD_STATE_COMPLETE            = 4                                      /**< Onboard is completed. */  
}
onboard_state_t;

/**@brief Used for configure persistent memory block for onboard_mode_ps_storage variable. */
extern bool init_global(uint8_t * global, uint8_t * default_value, uint16_t size);


/** @brief  This function reads onboard mode from pstorage and init timer and input interrupt callbacks
 *
 *  @return false in case error occurred, otherwise true.
 */
bool onboard_init(void);

/** @brief       This function return current onboard mode.
 *
 *  @retval      Current onboard mode.
 */
onboard_mode_t onboard_get_mode(void);

/** @brief       This function return current onboard state.
 *
 *  @retval      Current onboard state.
 */
onboard_state_t onboard_get_state(void);

/**@brief       This function is called when there is a successful entry to pstorage.
 *
 * @retval      Void.
 */
void onboard_on_store_complete(void);

/**@brief       This function handle disconnection event in onboarding active mode.
 *
 * @retval      Void.
 */
void onboard_on_disconnect(void);


#endif /* _ONBOARD_H_ */
