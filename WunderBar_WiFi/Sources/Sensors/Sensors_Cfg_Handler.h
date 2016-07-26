/** @file   Sensors_Cfg_Handler.h
 *  @brief  File contains functions for handling config messages between kinetis and
 *  		master ble during onboarding process. Handles messages in both directions.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <stdbool.h>
#include "Common_Defaults.h"


#define CFG_PASSKEY_WRITE_TIMEOUT    30000

// public functions

/**
 *  @brief  Send config parameters to master ble
 *
 *  Passkeys that are received from wifi should be sent to master ble module.
 *
 *  @param  ble passkeys struct
 *
 *  @return True if all successful, false if at least one failed
 */
bool Sensors_Cfg_Upload(ble_pass_t* ble_pass);

/**
 *  @brief  Process incoming config message from master ble
 *
 *  Configs are received by SPI in SPI frame format.
 *  Should be processed and stored wunderbar configuration global variable.
 *
 *  @param  SPI frame
 *
 *  @return void
 */
void Sensors_Cfg_ProcessBleMsg(spi_frame_t* spi_msg);
