
/** @file   led_control.h
 *  @brief  Contains declarations of functions for controling behaviour of sensor LED diode and corresponding macros.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */
 
/* -- Includes -- */

#ifndef _LED_H_
#define _LED_H_

#include "types.h"

/**< Turn of diode timeout in ms. */ 
#define  LED_TIMEOUT_CHAR_MS        15000                            /**< When LED is turned on through characteristic update. */ 
#define  LED_TIMEOUT_CONNECTION_MS  1000                             /**< When LED is turned on during connection. */ 

/** @brief  Function creates timer for controling LED diode.
 *
 *  @return false in case error occurred, otherwise true.
 */
bool led_control_init(void);

/** @brief  This function update on/off state of LED diode base of coressponding characteristic.
 *
 *  @param  value       New state of diode.
 *  @param  timeout_ms  This param is valid in the case that the first param is TRUE. In that case it is "turn off timeout interval" in ms.
 *
 *  @return true if operation is successful, otherwise false.
 */
bool led_control_update_char(bool value, uint32_t timeout_ms);

/** @brief  This function set config state, and start corresponding timer.
 *
 *  @return true if operation is successful, otherwise false.
 */
bool led_control_start_config(void);

#endif /* _LED_H_ */
