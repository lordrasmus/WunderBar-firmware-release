
/** @file   htu21d.h
 *  @brief  This file contains declarations of functions for working with HTU21D sensor 
 *          and corresponding macros.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */
 
#ifndef HTU21D
#define HTU21D

#include "i2c.h"
#include "wunderbar_common.h"

/**@brief Address of I2C slave. */
#define HTU21D_I2C_ADDR            0x40

/**@brief HTU21D Commands. */
#define HTU21D_TRIGGER_TEMP_HOLD   0xE3 
#define HTU21D_TRIGGER_HUMD_HOLD   0xE5
#define HTU21D_TRIGGER_TEMP_NOHOLD 0xF3
#define HTU21D_TRIGGER_HUMD_NOHOLD 0xF5
#define HTU21D_WRITE_USER_REG      0xE6
#define HTU21D_READ_USER_REG       0xE7
#define HTU21D_SOFT_RESET          0xFE

#define HTU21D_END_OF_BATTERY_SHIFT 6
#define HTU21D_ENABLE_HEATER_SHIFT  2
#define HTU21D_DISABLE_OTP_RELOAD   1
#define HTU21D_RESERVED_MASK        0x31

#define HTU21D_STARTUP_DELAY 15000
#define HTU21D_TEMP_MAX_DELAY 50000

typedef struct 
{
  TWI_STRUCT * i2c;
  uint8_t      addr;
  int16_t      user_register;
} 
htu21d_struct_t;

/** @brief  This function is called to initialize i2c interface to HTU21D sensor.
 *
 *  @param  htu21d  Pointer to an record that relates to the HTU21D sensor.
 *  @param  i2c     Pointer to an record that relates to the Two-Wire module.
 *  @param  addr    Slave address.
 *  @param  scl     I2C interface SCL pin.
 *  @param  sda     I2C interface SDA pin.
 *  @param  freq    Bus frequency.
 *
 *  @return  false in case that error is occurred, otherwise true.
 */
bool htu21d_init(htu21d_struct_t * htu21d, TWI_STRUCT * i2c, uint8_t addr, uint8_t scl, uint8_t sda, TWI_FREQUENCY freq);

/** @brief  This function configures measurement resolution.
 *
 *  @param  htu21d  Pointer to an record that relates to the HTU21D sensor.
 *  @param  config  Pointer to an sensor config record.
 *
 *  @return  false in case that error is occurred, otherwise true.
 */
bool htu21d_set_user_register(htu21d_struct_t * htu21d, sensor_htu_config_t * config);

/** @brief  This function retrieves temperature and humidity data.
 *
 *  @param  htu21d      Pointer to an record that relates to the HTU21D sensor.
 *  @param  temperature Provided storage used to hold the temperature measured data. 
 *  @param  humidity Provided storage used to hold the humidity measured data. 
 *
 *  @return false in case that error is occurred, otherwise true.
 */
void htu21d_get_data(htu21d_struct_t * htu21d, float * temperature, float * humidity);

#endif /* HTU21D */
