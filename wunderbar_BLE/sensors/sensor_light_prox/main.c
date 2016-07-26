
/** @file   main.c
 *  @brief  This file contains the source code for firmware of LIGHT/PROX sensor of Wunderbar Board.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */
 
/* -- Includes -- */

#include <stdint.h>
#include <string.h>
#include "nrf_delay.h"
#include "ble_driver.h"
#include "tcs3771.h"
#include "tcs3771_api.h"
#include "led_control.h"
#include "onboard.h"
#include "pstorage_driver.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief Callbacks from BLE lib. */

void my_connection_callback(void);
void my_disconnection_callback(void);
void my_advertising_timeout_callback(void);
void my_raw_write_callback(ble_gatts_evt_write_t * evt_write);
void my_characteristic_write_callback(ble_characteristic_info_t* char_info, uint16_t offset, uint16_t len, uint8_t* data);
void my_main_thread_callback(void);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief Global constants and variables. */

const uint8_t  long_service_uuid[]                        = {0x78,0x6d,0x2e,0x96,0x6f,0xd1,0x42,0x2f,0x8e,0x87,0x15,0x93,0xff,0xff,0x5a,0x09}; /**< 16-octet (128-bit) UUID. */

const uint16_t short_service_relayr_uuid                  = SHORT_SERVICE_RELAYR_UUID;                       /**< 16-bit UUID value for relayr service. */
const uint16_t short_service_relayr_open_comm_uuid        = SHORT_SERVICE_RELAYR_OPEN_COMM_UUID;             /**< 16-bit UUID value for relayr service which not require encryption. */
const uint16_t short_service_config_uuid                  = SHORT_SERVICE_CONFIG_UUID;                       /**< 16-bit UUID value for onboarding service. */

const uint16_t characteristic_sensorID_uuid               = CHARACTERISTIC_SENSOR_ID_UUID;                   /**< 16-bit UUID value for sensor ID char. */
const uint16_t characteristic_sensorBeaconFrequency_uuid  = CHARACTERISTIC_SENSOR_BEACON_FREQUENCY_UUID;     /**< 16-bit UUID value for beacon frequency char. */
const uint16_t characteristic_sensorFrequency_uuid        = CHARACTERISTIC_SENSOR_FREQUENCY_UUID;            /**< 16-bit UUID value for frequency char. */
const uint16_t characteristic_sensorLedState_uuid         = CHARACTERISTIC_SENSOR_LED_STATE_UUID;            /**< 16-bit UUID value for led state char. */
const uint16_t characteristic_sensorThreshold_uuid        = CHARACTERISTIC_SENSOR_THRESHOLD_UUID;            /**< 16-bit UUID value for threshold char. */
const uint16_t characteristic_sensorConfig_uuid           = CHARACTERISTIC_SENSOR_CONFIG_UUID;               /**< 16-bit UUID value for config char. */
const uint16_t characteristic_sensorData_r_uuid           = CHARACTERISTIC_SENSOR_DATA_R_UUID;               /**< 16-bit UUID value for data up char. */
const uint16_t characteristic_sensorPasskey_uuid          = CHARACTERISTIC_SENSOR_PASSKEY_UUID;              /**< 16-bit UUID value for sensor passkey char. */
const uint16_t characteristic_sensorMitmReqFlag_uuid      = CHARACTERISTIC_SENSOR_MITM_REQ_FLAG_UUID;        /**< 16-bit UUID value for working encryption mode char. */ 

/**@brief  Records related to characteristics. */
ble_characteristic_info_t characteristic_sensorID_info;
ble_characteristic_info_t characteristic_sensorBeaconFrequency_info;
ble_characteristic_info_t characteristic_sensorFrequency_info;
ble_characteristic_info_t characteristic_sensorLedState_info;
ble_characteristic_info_t characteristic_sensorThreshold_info;
ble_characteristic_info_t characteristic_sensorConfig_info;
ble_characteristic_info_t characteristic_sensorData_r_info;
ble_characteristic_info_t characteristic_sensorPasskey_info;
ble_characteristic_info_t characteristic_sensorMitmReqFlag_info;

ble_server_definition_t server_def = 
{
    my_connection_callback,             /**< Connection_callback. */
    my_disconnection_callback,          /**< Disconnection_callback. */
    my_advertising_timeout_callback,    /**< Advertising_timeout_callback. */
    my_raw_write_callback,              /**< Write_raw_callback. */
    my_characteristic_write_callback,   /**< Write_characteristic_callback. */
    NULL,                               /**< Main thread callback. */  
    NULL,                               /**< Subscription_callback. */
    NULL,                               /**< Passkey. */
    NULL                                /**< Device name. */
};
ble_service_info_t service_info;

sensor_lightprox_t sensor_lightprox;
uint16_t           short_service_uuid;

/**@brief  Default values for corresponding characteristics. These values are used if corresponding block of persistent storage is empty. */
const uint8_t                      DEFAULT_DEVICE_NAME[BLE_DEVNAME_MAX_LEN+1] = DEVICE_NAME_LIGHT;
const sensorID_t                   DEFAULT_SENSOR_ID = {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
                                                        0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33};
const beaconFrequency_t            DEFAULT_SENSOR_BEACON_FREQUENCY            = ADV_INTERVAL_MS;
const frequency_t                  DEFAULT_SENSOR_FREQUENCY                   = 1000;
const led_state_t                  DEFAULT_SENSOR_LED_STATE                   = false;
const sensor_lightprox_threshold_t DEFAULT_THRESHOLD                          = {{0, 0x8000, 0x7FFF}, {0, 0x8000, 0x7FFF}};
const sensor_lightprox_config_t    DEFAULT_SENSOR_CONFIG                      = {RGBC_GAIN_4, PROX_DRIVE_12_5_MA};
const passkey_t                    DEFAULT_SENSOR_PASSKEY                     = "000000";
const security_level_t             DEFAULT_MITM_REQ_FLAG                      = true;

#define WLED_CTRL_PIN 21

#define TCS37717_SCL_PIN            23                                         /**< I2C interface SCL pin. */
#define TCS37717_SDA_PIN            24                                         /**< I2C interface SDA pin. */
#define TCS37717_INT_PIN            25                                         /**< TCS3771 interrupt pin. */
#define I2C                         TWI1_HW                                    /**< I2C module. */
#define ADDR                        TCS3771_7_I2C_ADDR                         /**< I2C	slave address. */
#define SENSOR_MAX_READ_RETRIES     3
#define SENSOR_MAX_RESET_RETRIES    10
#define PROXIMITY_COLOR_THRESHOLD   200


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  Naive busy delay.
 *
 *  @param  len   Number of passes of loop.
 *
 *  @return Void.
 */

void delay(uint32_t len) 
{
    volatile uint32_t i = 0;
    while (i < len) 
    { 
        i++; 
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  Never returns - blinks an 8 bit pattern.
 *
 *  @param  val Value to blink, MSB first.
 *
 *  @return Void.
 */

void blink(uint8_t val) 
{   
    while (1) 
    {
        int mask = 0x80;
        while (mask > 0) 
        {
            bool bit = ((val & mask) != 0);
            gpio_write(LED_PIN, true);
            delay(bit ? 500000 : 100000);
            gpio_write(LED_PIN, false);
            delay(bit ? 100000 : 500000);
            mask >>= 1;
        }
        delay(1000000);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief    This funciton sets sensor properties according to control struct.
 *
 *  @return  false in case that error is occurred, otherwise true.
 */

bool set_sensor_props(void) 
{
	  // Set the internal integration time of the RGBC clear and IR channel ADCs.
    if (!TCS3771_SetColorSensParams(I2C, ADDR, 4))
    { 
        return false; 
    }
    
		// Set the integration time of the proximity ADC.
    if (!TCS3771_SetProximityParams(I2C, ADDR, 2, 1))
    { 
        return false; 
    }
    
		// Set RGBC gain and proximiti drive parameters.
    if(!TCS3771_SetControlRegister(I2C, ADDR, sensor_lightprox.config.prox_drive, sensor_lightprox.config.rgbc_gain))
    { 
        return false; 
    }
    
		// Set wait time.
    if (!TCS3771_SetWaitTime(I2C, ADDR, 40, false))
    { 
        return false; 
    }

    return true;    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief   Function for initializing some globals.
 *
 *  @retval  Void.
 */

void set_globals(void)
{
    server_def.passkey = sensor_lightprox.passkey;    
    sensor_lightprox.led_state = DEFAULT_SENSOR_LED_STATE;
    memcpy((uint8_t*)&server_def.name, (uint8_t*)DEFAULT_DEVICE_NAME, BLE_DEVNAME_MAX_LEN);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief   This function called when the sensor is ready to deliver data.
 *
 *  @param   data  Pointer to the measured data of TCS3771.
 *
 *  @return  false in case that error is occurred, otherwise true.
 */

bool sensor_callback(sensor_lightprox_data_t * data) 
{
    bool status = true;
    
    i2c_enable(I2C);
  
    // Enable proximity, color, wait and color interrupt generation.
    if (!TCS3771_SetMode(I2C, ADDR, true, true, true, true, false)) 
    {
        status = false; 
    }
    
    // Wait for interrupt.
    while(gpio_read(TCS37717_INT_PIN) == 1);
    
    if(!TCS3771_GetValues(I2C, ADDR, &(data->white),NULL,NULL,NULL,&(data->proximity)))
    {
        status = false; 
    }
    
		// Clear interrupts.
    if(!TCS3771_ClearInterrupts(I2C, ADDR, true, false))
    {
        status = false; 
    }   
    
    // If "proximity - color" threshold exceeded starts measuring RGB.
    if(data->proximity > PROXIMITY_COLOR_THRESHOLD)
    {
			  // Turn on light for color measuring.
        gpio_write(WLED_CTRL_PIN, true);
      
        // Enable proximity, color, wait and color interrupt generation.
        if (!TCS3771_SetMode(I2C, ADDR, true, true, true, true, false)) 
        {
            status = false; 
        }
        
        // Wait for interrupt.
        while(gpio_read(TCS37717_INT_PIN) == 1);
        
				// Read RGB data.
        if(!TCS3771_GetValues(I2C, ADDR, NULL,&(data->r),&(data->g),&(data->b),NULL))
        {
            status = false; 
        }
        
				// Clear interrupt.
        if(!TCS3771_ClearInterrupts(I2C, ADDR, true, false))
        {
            status = false; 
        }
        
				// Turn off light for color measuring.
        gpio_write(WLED_CTRL_PIN, false);
    }
  
		// Turn off sensor.
    if (!TCS3771_PowerOff(I2C, ADDR)) 
    {
        status = false; 
    }
    
    i2c_disable(I2C);
    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief   Initial sensor startup.
 *
 *  @return  false in case that error is occurred, otherwise true.
 */

bool sensor_init() 
{ 
    gpio_set_pin_digital_input(TCS37717_INT_PIN, PIN_PULL_UP);
	
	  // Init I2C with corresponding params.
    i2c_init(I2C, TCS37717_SCL_PIN, TCS37717_SDA_PIN, K100);

    int id = TCS3771_GetId(I2C, ADDR);
    
	  // Check device ID.
    if (id != TCS3771_ID_3_7) 
    { 
        return false; 
    }
    
		// Set brightness window.
    if (!TCS3771_SetBrightnessWindow(I2C, ADDR, 0, 65535, TCS3771_PERS_BRIGHT_EVERY)) 
    { 
        return false; 
    }
     
		// Set proximity window.
    if (!TCS3771_SetProximityWindow(I2C, ADDR, 0, 65535, TCS3771_PERS_PROX_EVERY)) 
    { 
        return false; 
    }
    
		// Set sensor properties.
    if (!set_sensor_props())
    { 
        return false; 
    } 
    
		// Get sensor measurements.
    sensor_callback(&sensor_lightprox.data);
    
    i2c_disable(I2C);
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Inits global with value stored in persistent_storage, or with default_value (if corresponding block in persistent_storage is empty).
 *
 * @param   global          Global which will be initialized.
 * @param   default_value   If corresponding block in persistent_storage is empty, initialize with this param.
 * @param   default_value   Size of global param.
 *
 * @return  true if operation is successful, otherwise false.
 */

bool init_global(uint8_t * global, uint8_t * default_value, uint16_t size) 
{
    uint32_t load_status;
   
    // Set pstorage block id.  	
    if (!pstorage_driver_register_block(global, size)) 
    {
        return false;
    }
		
		// Load data from corresponding block.
    load_status = pstorage_driver_load(global);
    if((load_status == PS_LOAD_STATUS_FAIL)||(load_status == PS_LOAD_STATUS_NOT_FOUND)) 
    {
        return false;
    }
		// If there is no data use default value.
    else if(load_status == PS_LOAD_STATUS_EMPTY) 
    { 
        memcpy(global, default_value, size);
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief  This function initializes and configures pstorage. 
 *         Also, it registers characteristic values to corresponding blocks in persistent memory.
 *
 * @param  None.
 *
 * @return true if operation is successful, otherwise false.
 */

bool pstorage_driver_init() 
{ 
    if (pstorage_init() != NRF_SUCCESS) 
    {
        return false;
    }
   
    // Configure pstorage driver.
    if(!pstorage_driver_cfg(0x20)) 
    {
        return false;
    }
    
    // Read SensorID from p_storage.
    if(!init_global((uint8_t*)&sensor_lightprox.sensorID, (uint8_t*)&DEFAULT_SENSOR_ID, sizeof(sensor_lightprox.sensorID)))
    {
        return false;
    }
    // Read SensorBeaconFrequency from p_storage.
    if(!init_global((uint8_t*)&sensor_lightprox.beaconFrequency, (uint8_t*)&DEFAULT_SENSOR_BEACON_FREQUENCY, sizeof(sensor_lightprox.beaconFrequency))) 
    {
        return false;
    }
    // Read SensorFrequency from p_storage.
    if(!init_global((uint8_t*)&sensor_lightprox.frequency, (uint8_t*)&DEFAULT_SENSOR_FREQUENCY, sizeof(sensor_lightprox.frequency))) 
    {
        return false;
    }
    // Read Threshold from p_storage.
    if(!init_global((uint8_t*)&sensor_lightprox.threshold, (uint8_t*)&DEFAULT_THRESHOLD, sizeof(sensor_lightprox.threshold))) 
    {
        return false;
    }
    // Read Config from p_storage.
    if(!init_global((uint8_t*)&sensor_lightprox.config, (uint8_t*)&DEFAULT_SENSOR_CONFIG, sizeof(sensor_lightprox.config))) 
    {
        return false;
    }
    // Read Passkey from p_storage.
    if(!init_global((uint8_t*)sensor_lightprox.passkey, (uint8_t*)DEFAULT_SENSOR_PASSKEY, sizeof(sensor_lightprox.passkey))) 
    {
        return false;
    }
    // Read mitm_req_flag from p_storage.
    if(!init_global((uint8_t*)&sensor_lightprox.mitm_req_flag, (uint8_t*)&DEFAULT_MITM_REQ_FLAG, sizeof(sensor_lightprox.mitm_req_flag))) 
    {
        return false;
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief    This function compares new measurement data with old measurement data, using sensor threshold parameters.
 *
 * @param    threshold  Record which contains hi, low and sbl value of threshold.
 * @param    old_value  Old value for comparing.
 * @param    new_value  New value for comparing.
 *
 * @retval   true if threshold exceeded, false otherwise.
 */

bool compare_with_threshold(sensor_lightprox_threshold_t * sensor_threshold, sensor_lightprox_data_t * old_data, sensor_lightprox_data_t * new_lightprox_data) 
{ 
    if(
        check_threshold_int(&sensor_threshold->white, old_data->white, new_lightprox_data->white) ||
        check_threshold_int(&sensor_threshold->proximity, old_data->proximity, new_lightprox_data->proximity)
      )
    {
        return true;
    } 
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Reads new sensor data, compare it with server's sensor data usind threshold params. 
 *          If threshold exceeded, updates characteristic value. 
 *
 * @retval  Void.
 */

void get_sensor_data() 
{ 
    sensor_lightprox_data_t new_lightprox_data;
    
    // Clear new_lightprox_data.
    memset((uint8_t *)&new_lightprox_data, 0, sizeof(new_lightprox_data));
	
	  // Reads sensor measurements.
    if(!sensor_callback(&new_lightprox_data))
    {
        return;
    }
    
		// Compare obtained values with threshold.
    if(
        compare_with_threshold(&sensor_lightprox.threshold, &sensor_lightprox.data, &new_lightprox_data)
      )
    {
			  // Update values for data.
        memcpy((uint8_t*)&sensor_lightprox.data, (uint8_t*)&new_lightprox_data, sizeof(sensor_lightprox.data));
			
			  // Update characteristic.
        ble_update_characteristic_value(&characteristic_sensorData_r_info,  (uint8_t *)(&sensor_lightprox.data),  sizeof(sensor_lightprox.data));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Application tick handler.
 *
 * @param   ctx   Value that will be passed back to the callback  
 *
 * @retval  Void.
 */

void app_tick_handler(void * ctx) 
{
	  // Check if notification or indication is enabled.
    if(
        ((characteristic_sensorData_r_info.state & BLE_CHARACTERISTIC_IS_NOTIFYING)  == 0)&&
        ((characteristic_sensorData_r_info.state & BLE_CHARACTERISTIC_IS_INDICATING) == 0)
      )
    {
        return;
    }
    
		// Reads new sensor data.
    get_sensor_data();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief  Callback which is called on connection established GAP BLE event.
 *
 * @return Void.
 */

void my_connection_callback() 
{
    ble_stop_advertising();                                                    // Stop advertising.
  
	  // If sensor is not in onboard active mode turn on LED diode.
    if(onboard_get_mode() == ONBOARD_MODE_IDLE)
    {
			  led_control_update_char(true, LED_TIMEOUT_CONNECTION_MS);			

			  // Start timer for polling sensor data.
        if (!ble_set_app_tick(app_tick_handler, sensor_lightprox.frequency, NULL)) 
        {
            blink(106);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief  Callback which is called on disconnected from peer GAP BLE event.
 *
 * @return Void.
 */

void my_disconnection_callback() 
{ 
	  // If we not detected button down event jet.
    if(onboard_get_state() < ONBOARD_STATE_BUTTON_DOWN)
    {
      ble_start_advertising(sensor_lightprox.beaconFrequency);
      
			// If we in onboarding active mode?
      if(onboard_get_mode() == ONBOARD_MODE_ACTIVE)
      {
          onboard_on_disconnect();
      }
			// If we not in onboarding active mode?
      else if(onboard_get_mode() == ONBOARD_MODE_IDLE)
      {
          ble_set_app_tick(app_tick_handler, 0, NULL);
          characteristic_sensorData_r_info.state = 0;
          led_control_update_char(false, 0);
      }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief  Callback which is called on advertisement timeout GAP BLE event.
 *
 * @return Void.
 */

void my_advertising_timeout_callback() 
{
    ble_start_advertising(sensor_lightprox.beaconFrequency);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief  Callback which is called on write BLE GATTS event.
 *
 * @param  evt_write  Event structure for BLE_GATTS_EVT_WRITE event.
 *
 * @return Void.
 */

void my_raw_write_callback(ble_gatts_evt_write_t * evt_write) 
{ 
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorID_info);
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorBeaconFrequency_info);
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorFrequency_info);
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorLedState_info);
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorThreshold_info); 
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorConfig_info);
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorData_r_info);  
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorPasskey_info);
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorMitmReqFlag_info);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief  Callback which is called when write to characteristic value occurred.
 *
 * @param  char_info  Characteristic properties. 
 * @param  offset     Offset for the write operation.
 * @param  len        Length of the incoming data.
 * @param  data       Incoming data.
 *
 * @return Void.
 */

void my_characteristic_write_callback(ble_characteristic_info_t* char_info, uint16_t offset, uint16_t len, uint8_t* data) 
{ 
	  // Write to sensor ID characteristic value?
    if ((char_info == &characteristic_sensorID_info) && (offset == 0) && (len == sizeof(sensor_lightprox.sensorID))) 
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_lightprox.sensorID, data, len);  
        pstorage_driver_request_store((uint8_t*)(&sensor_lightprox.sensorID));
    } 
    
		// Write to beacon frequency characteristic value?
    if ((char_info == &characteristic_sensorBeaconFrequency_info) && (offset == 0) && (len == sizeof(sensor_lightprox.beaconFrequency))) 
    {
        beaconFrequency_t tmp_beaconFrequency;
        memcpy((uint8_t*)&tmp_beaconFrequency, data, len);    
      
			  // Check if new value is valid.
        if((tmp_beaconFrequency >= 20) && (tmp_beaconFrequency <= 10240))
        {
            sensor_lightprox.beaconFrequency = tmp_beaconFrequency;
            pstorage_driver_request_store((uint8_t*)(&sensor_lightprox.beaconFrequency));
        }
    }
    
		// Write to frequency characteristic value?
    if ((char_info == &characteristic_sensorFrequency_info) && (offset == 0) && (len == sizeof(sensor_lightprox.frequency))) 
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_lightprox.frequency, data, len); 
        pstorage_driver_request_store((uint8_t*)(&sensor_lightprox.frequency));
			  // Set new interval for app timer.
        ble_set_app_tick(app_tick_handler, sensor_lightprox.frequency, NULL);
    }
    
		// Write to led state characteristic value?
    if ((char_info == &characteristic_sensorLedState_info) && (offset == 0) && (len == sizeof(sensor_lightprox.led_state))) 
    {
			  // Copy new data to local buffer, and update state of LED diode.
        memcpy((uint8_t*)&sensor_lightprox.led_state, data, len); 
        led_control_update_char(sensor_lightprox.led_state, LED_TIMEOUT_CHAR_MS);			  
    }
    
		// Write to threshold characteristic value?
    if ((char_info == &characteristic_sensorThreshold_info) && (offset == 0) && (len == sizeof(sensor_lightprox.threshold))) 
    {
			  // Copy new data to local buffer, and update state of LED diode.
        memcpy((uint8_t*)&sensor_lightprox.threshold, data, len);
        pstorage_driver_request_store((uint8_t*)(&sensor_lightprox.threshold));
    } 
    
		// Write to config characteristic value?
    if ((char_info == &characteristic_sensorConfig_info) && (offset == 0) && (len == sizeof(sensor_lightprox.config))) 
    {
			  // Check if new value is valid.
        if(
            ((data[0] == RGBC_GAIN_1) || (data[0] == RGBC_GAIN_16) || (data[0] == RGBC_GAIN_16) || (data[0] == RGBC_GAIN_60)) &&
            ((data[1] == PROX_DRIVE_12_5_MA)  || (data[1] == PROX_DRIVE_25_MA) || (data[1] == PROX_DRIVE_50_MA) || (data[1] == PROX_DRIVE_100_MA))
          )
        {
					  // Copy new data to local buffer, and update state of LED diode.
            memcpy((uint8_t*)&sensor_lightprox.config, data, len);  
            pstorage_driver_request_store((uint8_t*)(&sensor_lightprox.config));
            
					  // Write new configuration into sensor.
            i2c_enable(I2C);
            if(!TCS3771_SetControlRegister(I2C, ADDR, sensor_lightprox.config.prox_drive, sensor_lightprox.config.rgbc_gain))
            {
                blink(106);
            }
            i2c_disable(I2C);
        }
    }
    
		// Write to sensor passkey characteristic value?
    if ((char_info == &characteristic_sensorPasskey_info) && (offset == 0) && (len == 6)) 
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_lightprox.passkey, data, len); 
        pstorage_driver_request_store((uint8_t*)(&sensor_lightprox.passkey));
			
			  // Set flag to clear bond manager.
        ble_clear_bondmngr_request();
    }
    
		// Write to working encryption mode characteristic value?
    if ((char_info == &characteristic_sensorMitmReqFlag_info) && (offset == 0) && (len == sizeof(sensor_lightprox.mitm_req_flag)))
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        sensor_lightprox.mitm_req_flag = (data[0] == 1);
        pstorage_driver_request_store((uint8_t*)(&sensor_lightprox.mitm_req_flag));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief Function for application main entry.
 */

int main(void) 
{
    gpio_set_pin_digital_input(BUTTON_PIN, PIN_PULL_UP);                                           // Set button pin to be input, with pull up.
    while(gpio_read(BUTTON_PIN) == 0);                                                             // If button pin is pressed, wait to be released.
    nrf_delay_us(50000);
  
    gpio_set_pin_digital_input(TCS37717_INT_PIN, PIN_PULL_NONE);                                   // Config interrupt pin to be input.
    gpio_set_pin_digital_output(WLED_CTRL_PIN, PIN_DRIVE_S0S1);                                    // Config "light diode" pin to be output.
    gpio_write(WLED_CTRL_PIN, false);
  
	  // Initialize some global variables.  
    set_globals();
    
	  // Initialize BLE server modules.
    if (!ble_init_server(&server_def, pstorage_driver_init, &sensor_lightprox.mitm_req_flag)) 
    { 
        blink(101); 
    }
    
		// Initialize services and characteristics for non-onboarding mode.
    if(onboard_get_mode() == ONBOARD_MODE_IDLE)
    {
			  // Check whether there is a need for MITM encryption.
        uint16_t read_enc_flag  = (sensor_lightprox.mitm_req_flag ? BLE_CHARACTERISTIC_READ_ENC_REQUIRE  : BLE_CHARACTERISTIC_READ_ENC_REQUIRE_NO_MITM);
        uint16_t write_enc_flag = (sensor_lightprox.mitm_req_flag ? BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE : BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE_NO_MITM);
        
			  // Initialize sensor.
        if (!sensor_init())
        {
            blink(102);
        }
        short_service_uuid = sensor_lightprox.mitm_req_flag ? short_service_relayr_uuid : short_service_relayr_open_comm_uuid;
        
				// Adding user service.
        if (!ble_add_service(short_service_uuid, 0, 0, &service_info)) 
        { 
            blink(103);
        }
        
				// Adding related characteristics.
				
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorID_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | read_enc_flag,
                                    (const uint8_t*)"SensorID", 
                                    (const uint8_t*)sensor_lightprox.sensorID,
                                    sizeof(sensorID_t),
                                    &characteristic_sensorID_info)) 
        {
            blink(104);
        }
      
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorBeaconFrequency_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorBeaconFrequency", 
                                    (const uint8_t*)&sensor_lightprox.beaconFrequency,
                                    sizeof(sensor_lightprox.beaconFrequency),
                                    &characteristic_sensorBeaconFrequency_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorFrequency_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorFrequency", 
                                    (const uint8_t*)&sensor_lightprox.frequency,
                                    sizeof(sensor_lightprox.frequency),
                                    &characteristic_sensorFrequency_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorLedState_uuid,
                                    BLE_CHARACTERISTIC_CAN_WRITE | write_enc_flag,
                                    (const uint8_t*)"SensorLedState", 
                                    (const uint8_t*)&sensor_lightprox.led_state,
                                    sizeof(sensor_lightprox.led_state),
                                    &characteristic_sensorLedState_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorThreshold_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorThreshold", 
                                    (const uint8_t*)&sensor_lightprox.threshold,
                                    sizeof(sensor_lightprox.threshold),
                                    &characteristic_sensorThreshold_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorConfig_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorConfig", 
                                    (const uint8_t*)&sensor_lightprox.config,
                                    sizeof(sensor_lightprox.config),
                                    &characteristic_sensorConfig_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorData_r_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_NOTIFY | BLE_CHARACTERISTIC_CAN_INDICATE | read_enc_flag,
                                    (const uint8_t*)"SensorData", 
                                    (const uint8_t*)&sensor_lightprox.data,
                                    sizeof(sensor_lightprox.data),
                                    &characteristic_sensorData_r_info)) 
        {
            blink(104);
        }
    }
		// Initialize services and characteristics for onboarding mode.
    else                            
    {
			  // Adding user service.
        if (!ble_add_service(short_service_config_uuid, 0, 0, &service_info)) 
        { 
            blink(103);
        }
    
				// Adding related characteristics.
					
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorID_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE,
                                    (const uint8_t*)"SensorID", 
                                    (const uint8_t*)sensor_lightprox.sensorID,
                                    sizeof(sensorID_t),
                                    &characteristic_sensorID_info)) 
        {
            blink(104);
        }
      
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorPasskey_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE,
                                    (const uint8_t*)"SensorPasskey", 
                                    (const uint8_t*)sensor_lightprox.passkey,
                                    6,
                                    &characteristic_sensorPasskey_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorMitmReqFlag_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE,
                                    (const uint8_t*)"SensorMitmRequireFlag", 
                                    (const uint8_t*)&sensor_lightprox.mitm_req_flag,
                                    sizeof(security_level_t),
                                    &characteristic_sensorMitmReqFlag_info)) 
        {
            blink(104);
        }
    }
    
		// Adding Device Informaiton Service.
    if (!ble_add_device_information_service())
    {
        blink(102);
    }
    
		// Adding Battery Service.
    if (!ble_add_bat_service()) 
    {
        blink(102);
    }
    
		// Starting server.
    if (!ble_start_server()) 
    { 
        blink(105);
    }
    
		// Initializing the advertising functionality.
    if (!ble_init_advertising()) 
    {
        blink(106);
    } 
    
		// Start advertising.
    if (!ble_start_advertising(sensor_lightprox.beaconFrequency)) 
    {
        blink(106);
    }
    
    ble_run();

    blink(107);
} 
