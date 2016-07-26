#ifndef _TCS3771_API_
#define _TCS3771_API_

#include "i2c.h"
#include "tcs3771.h"
#include "wunderbar_common.h"

// --------------------------
// --- TCS3771 public API ---
// --------------------------

/** turns the chip on (initially in sleep mode)
@param i2c the I2C interface
@param addr the target's I2C address
@return true on success, false on error */
bool TCS3771_PowerOn(TWI_STRUCT* i2c, uint8_t addr);

/** turns the chip off
@param i2c the I2C interface
@param addr the target's I2C address
@return true on success, false on error */
bool TCS3771_PowerOff(TWI_STRUCT* i2c, uint8_t addr);

/** returns the chip status
@param i2c the I2C interface
@param addr the target's I2C address
@return the status, -1 on error */
int TCS3771_GetStatus(TWI_STRUCT* i2c, uint8_t addr);

/** returns the chip ID
@param i2c the I2C interface
@param addr the target's I2C address
@return the chip ID, -1 on error */
int TCS3771_GetId(TWI_STRUCT* i2c, uint8_t addr);

/** Set color (RGBC) sensing parameters
@param i2c the I2C interface
@param addr the target's I2C address
@param time the integration time in 2.4ms units (+1)*/
bool TCS3771_SetColorSensParams(TWI_STRUCT* i2c, uint8_t addr, uint8_t integ_cycles);

/** Set delay time between samples
@param i2c the I2C interface
@param addr the target's I2C address
@param time the integration time in 2.4ms or 28.8ms units (+1) 
@param x12 if true, time units are 28.8ms, otherwise 2.4ms */
bool TCS3771_SetWaitTime(TWI_STRUCT* i2c, uint8_t addr, uint8_t time, bool x12);

/** Set proximity detection parameters
@param i2c the I2C interface
@param addr the target's I2C address
@param time proximity detection time in 2.4ms units (+1) - 0 recommended
@param pulsecount count of IR pulses per proximity detection (1-64)
@return true on success, false on error */
bool TCS3771_SetProximityParams(TWI_STRUCT* i2c, uint8_t addr, uint8_t integ_cycles, uint8_t pulsecount);

bool TCS3771_SetControlRegister(TWI_STRUCT* i2c, uint8_t addr, sensor_lightprox_prox_drive_t pulsestrength, sensor_lightprox_rgbc_gain_t gain);

/** Set all configuration parameters in one call
@param i2c the I2C interface
@param addr the target's I2C address
@param rgbc_time the integration time in 2.4ms units (+1) 
@param rgbc_gain RGBC gain factor
@param prox_time proximity detection time in 2.4ms units (+1) - 0 recommended
@param prox_pulses count of IR pulses per proximity detection (1-64)
@param prox_drive IR pulse current 
@param prox_channel which sensors to use for proximity detection 
@param wait_time the integration time in 2.4ms or 28.8ms units (+1) 
@param wait_long if true, wait_time units are 28.8ms, otherwise 2.4ms
@return true on success, false on error */
bool TCS3771_SetControlParams(TWI_STRUCT* i2c, uint8_t addr,
                uint8_t rgbc_time, tcs3771_ctl_again_t rgbc_gain,
                uint8_t prox_time, uint8_t prox_pulses, tcs3771_ctl_pdrive_t prox_drive, tcs3771_ctl_pdiode_t prox_channel,
                uint8_t wait_time, bool wait_long);

/** sets the operation mode
@param i2c the I2C interface
@param addr the target's I2C address
@param en_rgbc enables rgbc (color) sensing
@param en_prox enables proximity sensing
@param en_wait enables wait time between sensing passes
@param en_bright_int enables brightness window interrupts
@param en_prox_int enables proximity window interrupts
@return true on success, false on error */
bool TCS3771_SetMode(TWI_STRUCT* i2c, uint8_t addr, bool en_rgbc, bool en_prox, bool en_wait, bool en_bright_int, bool en_prox_int);

/** reads the sampled rgbc (c=clear) brightness and proximity values.
This call does NOT check whether the values are valid. Use GetStatus() for that.

@param i2c the I2C interface
@param addr the target's I2C address
@param c clear channel return value. Passing NULL will omit reading this channel.
@param r red channel return value. Passing NULL will omit reading this channel.
@param g green channel return value. Passing NULL will omit reading this channel.
@param b blue channel return value. Passing NULL will omit reading this channel.
@param p proximity channel return value. Passing NULL will omit reading this channel.
@return true on success, false on error */
bool TCS3771_GetValues(TWI_STRUCT* i2c, uint8_t addr, uint16_t* c, uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* p);

/** sets the brightness interrupt window. Values outside this range can trigger an interrupt, if enabled
@param i2c the I2C interface
@param addr the target's I2C address
@param lower lower threshold for the clear channel
@param upper upper threshold for the clear channel
@param persistence value filtering - number of continuous outliers that trigger an interrupt
@return true on success, false on error */
bool TCS3771_SetBrightnessWindow(TWI_STRUCT* i2c, uint8_t addr, uint16_t lower, uint16_t upper, tcs3771_pers_bright_t persistence);

/** sets the proximity interrupt window. Values outside this range can trigger an interrupt, if enabled
@param i2c the I2C interface
@param addr the target's I2C address
@param lower lower threshold for the proximity value
@param upper upper threshold for the proximity channel
@param persistence value filtering - number of continuous outliers that trigger an interrupt
@return true on success, false on error */
bool TCS3771_SetProximityWindow(TWI_STRUCT* i2c, uint8_t addr, uint16_t lower, uint16_t upper, tcs3771_pers_prox_t persistence);

/** clears interrupt flags
@param i2c the I2C interface
@param addr the target's I2C address
@param clear_rgbc if true, the rgbc interrupt is cleared (if set)
@param clear_prox if true, the proximity interrupt is cleared (if set)
@return true on success, false on error */
bool TCS3771_ClearInterrupts(TWI_STRUCT* i2c, uint8_t addr, bool clear_rgbc, bool clear_prox);

#endif

