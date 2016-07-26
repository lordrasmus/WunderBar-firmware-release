
/** @file   htu21d.c
 *  @brief  This driver contains definitions of functions for working with HTU21D sensor 
 *          and corresponding macros, constants,and global variables.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */
 
/* -- Includes -- */

#include "htu21d.h"
#include "gpio.h"
#include "gpiote.h"
#include "nrf_delay.h"

#define Lo(param) ((uint8_t *)&param)[0]                   /**< Macro returns low byte of param. */
#define Hi(param) ((uint8_t *)&param)[1]                   /**< Macro returns high byte of param. */
  
static uint16_t temp_meas_time, humidity_meas_time;        /**< Temperature and Humidity measurement time. */

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

bool htu21d_init(htu21d_struct_t * htu21d, TWI_STRUCT * i2c, uint8_t addr, uint8_t scl, uint8_t sda, TWI_FREQUENCY freq)
{
  
		gpio_set_pin_digital_output(scl, PIN_DRIVE_S0S1);
		gpio_write(scl, true);
		nrf_delay_us(HTU21D_STARTUP_DELAY);
		gpio_write(scl, false);

		uint8_t reset_cmd = HTU21D_SOFT_RESET;
    
	  // Init I2C module with corresponding pins and speed.
		if (!i2c_init(i2c,scl, sda, freq)) 
		{
				return false;
		}
		
		// Reset device.
		if (i2c_write(i2c, addr, 1, &reset_cmd, true) < 0) 
		{
				return false;
		}

		htu21d->i2c = i2c;
		htu21d->addr = addr;
		htu21d->user_register = -1;

		i2c_disable(htu21d->i2c);

		return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function configures measurement resolution.
 *
 *  @param  htu21d  Pointer to an record that relates to the HTU21D sensor.
 *  @param  config  Pointer to an sensor config record.
 *
 *  @return  false in case that error is occurred, otherwise true.
 */

bool htu21d_set_user_register(htu21d_struct_t * htu21d, sensor_htu_config_t * config) 
{
  
  int ret_i2c;
  uint8_t  user_reg[2];
  
  user_reg[0] = HTU21D_WRITE_USER_REG;
  
	// Enable I2C.
  i2c_enable(htu21d->i2c);
  
	// Set byte to be written to user register.
	// Set corresponding measurement time.
  switch(*config)
  {
			// RH:   12bit
			// TEMP: 14bit
      case HTU21D_RH_12_TEMP14:
      {
          user_reg[1] = 0x2;
          temp_meas_time = 50000;
          humidity_meas_time = 16000;
          break;
      }
      
			// RH:   8bit
			// TEMP: 12bit
      case HTU21D_RH_8_TEMP12:
      {
          user_reg[1] = 0x3;
          temp_meas_time = 13000;
          humidity_meas_time = 3000;
          break;
      }
      
			// RH:   10bit
			// TEMP: 13bit
      case HTU21D_RH_10_TEMP13:
      {
          user_reg[1] = 0x82;
          temp_meas_time = 25000;
          humidity_meas_time = 5000;
          break;
      }
      
			// RH:   11bit
			// TEMP: 11bit
      case HTU21D_RH_11_TEMP11:
      {
          user_reg[1] = 0x83;
          temp_meas_time = 7000;
          humidity_meas_time = 8000;
          break;
      }
  }
  

  ret_i2c = i2c_write(htu21d->i2c, htu21d->addr, 2, user_reg, true);
  
  i2c_disable(htu21d->i2c);
  
  if (ret_i2c > 0)
  {
      htu21d->user_register = user_reg[1];
  }
	
  return ret_i2c > 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function retrieves temperature measured data.
 *
 *  @param  htu21d  Pointer to an record that relates to the HTU21D sensor.
 *  @param  data    Provided storage used to hold the measured data. 
 *
 *  @return  false in case that error is occurred, otherwise true.
 */

static bool htu21d_get_temp(htu21d_struct_t * htu21d, float * data)
{
    int32_t temp;
    uint8_t temp_cmd = HTU21D_TRIGGER_TEMP_NOHOLD;
  
	  // Request to reading temperature.
    int i = i2c_write(htu21d->i2c, htu21d->addr, 1, &temp_cmd, true);
  
    if (1 != i ) 
    {
        return false;
    }
    
		// Wait corresponding time.
    nrf_delay_us(temp_meas_time);

		// Read temperature data. 
    if (3 != i2c_read(htu21d->i2c, htu21d->addr, 3, (uint8_t *)&temp) ) 
		{
				return false;
    }
    
		// Convert to final value.
    temp = (Hi(temp) & 0xFC) | ((uint16_t)Lo(temp) << 8);
    *data = ((float)temp * 175.72) / 0x10000 - 46.85;
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function retrieves humidity measured data.
 *
 *  @param  htu21d  Pointer to an record that relates to the HTU21D sensor.
 *  @param  data    Provided storage used to hold the measured data. 
 *
 *  @return  false in case that error is occurred, otherwise true.
 */

static bool htu21d_get_humidity(htu21d_struct_t * htu21d, float * data)
{
    int32_t humidity;
    uint8_t humd_cmd = HTU21D_TRIGGER_HUMD_NOHOLD;
   
	  // Request to reading humidity.
    int i = i2c_write(htu21d->i2c, htu21d->addr, 1, &humd_cmd, true);
      
    if (1 != i ) 
    {
        return false;
    }
    
		// Wait corresponding time.
    nrf_delay_us(humidity_meas_time);
    
		// Read humidity data. 
    if (3 != i2c_read(htu21d->i2c, htu21d->addr, 3, (uint8_t *)&humidity) ) 
    {
        return false;
    }
    
		// Convert to final value.
		
    humidity = (Hi(humidity) & 0xFC) | ((uint16_t)Lo(humidity) << 8);
    
    *data = ((float)humidity * 125) / 0x10000 - 6;
    if(*data > 100)
    {
        *data = 100;
    }
    else if(*data < 0)
    {
        *data = 0;
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function retrieves temperature and humidity data.
 *
 *  @param  htu21d      Pointer to an record that relates to the HTU21D sensor.
 *  @param  temperature Provided storage used to hold the temperature measured data. 
 *  @param  humidity Provided storage used to hold the humidity measured data. 
 *
 *  @return false in case that error is occurred, otherwise true.
 */

void htu21d_get_data(htu21d_struct_t * htu21d, float * temperature, float * humidity)
{
    i2c_enable(htu21d->i2c);
    while(htu21d_get_temp(htu21d, temperature) == false);
    while(htu21d_get_humidity(htu21d, humidity) == false);
    i2c_disable(htu21d->i2c);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
