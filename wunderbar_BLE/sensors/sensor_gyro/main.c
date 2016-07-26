
/** @file   main.c
 *  @brief  This file contains the source code for firmware of ACC/GYRO sensor of Wunderbar Board.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */
 
/* -- Includes -- */

#include <stdint.h>
#include <string.h>
#include "nrf_delay.h"
#include "ble_driver.h"
#include "mpu6500.h"
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


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief Global constants and variables. */

const uint8_t  long_service_uuid[]          = {0x78,0x6d,0x2e,0x96,0x6f,0xd1,0x42,0x2f,0x8e,0x87,0x15,0x93,0xff,0xff,0x5a,0x09}; /**< 16-octet (128-bit) UUID. */

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
    my_connection_callback,            /**< Connection_callback. */
    my_disconnection_callback,         /**< Disconnection_callback. */
    my_advertising_timeout_callback,   /**< Advertising_timeout_callback. */
    my_raw_write_callback,             /**< Write_raw_callback. */
    my_characteristic_write_callback,  /**< Write_characteristic_callback. */
    NULL,                              /**< Main thread callback. */
    NULL,                              /**< Subscription_callback. */
    NULL,                              /**< Passkey. */
    NULL                               /**< Device name . */
  
};
ble_service_info_t service_info;

sensor_gyro_t sensor_gyro;
uint16_t      short_service_uuid;

/**@brief  Default values for corresponding characteristics. These values are used if corresponding block of persistent storage is empty. */
const uint8_t                  DEFAULT_DEVICE_NAME[BLE_DEVNAME_MAX_LEN+1] = DEVICE_NAME_GYRO;
const sensorID_t               DEFAULT_SENSOR_ID = {0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
                                                    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};
const beaconFrequency_t        DEFAULT_SENSOR_BEACON_FREQUENCY            = ADV_INTERVAL_MS;
const frequency_t              DEFAULT_SENSOR_FREQUENCY                   = 1000;
const led_state_t              DEFAULT_SENSOR_LED_STATE                   = false;
const sensor_gyro_threshold_t  DEFAULT_SENSOR_THRESHOLD                   = {{0, -200000, 200000}, {0, -1600, 1600}};
const sensor_gyro_config_t     DEFAULT_SENSOR_CONFIG                      = {GYRO_FULL_SCALE_250DPS, ACC_FULL_SCALE_2G};
const passkey_t                DEFAULT_SENSOR_PASSKEY                     = "000000";
const security_level_t         DEFAULT_MITM_REQ_FLAG                      = true;

static mpup6500_struct_t mpu;
static uint8_t whoami;

static threshold_float_t acc_threshold;
static threshold_float_t gyro_threshold;
static coord_float_t     acc_coord_current;
static coord_float_t     gyro_coord_current;

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
/** @brief  This function converts sensor threshold format into float format, which used for comparing with float gyro/accel coordinates.
 *
 *  @param  threshold       Pointer to threshold record (related to corresponding sensor characteristic).
 *  @param  gyro_threshold  Pointer to gyro threshold in float format.
 *  @param  acc_threshold   Pointer to accel threshold in float format.
 *
 *  @return Void.
 */

void convert_threshold_to_float(sensor_gyro_threshold_t * threshold, threshold_float_t * gyro_threshold, threshold_float_t * acc_threshold)
{ 
    acc_threshold->high  = ((float)threshold->acc.high)  / 100;
    acc_threshold->low   = ((float)threshold->acc.low)   / 100;
    acc_threshold->sbl   = ((float)threshold->acc.sbl)   / 100;
  
    gyro_threshold->high = ((float)threshold->gyro.high) / 100;
    gyro_threshold->low  = ((float)threshold->gyro.low)  / 100;
    gyro_threshold->sbl  = ((float)threshold->gyro.sbl)  / 100;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function converts float coordinate of gyro and accel, into format which will be used on BLE interface.
 *          
 *  @param  gyro_coord_fl  Pointer to gyro coordinate record in float format.
 *  @param  acc_coord_fl   Pointer to accel coordinate record in float format.
 *  @param  data           Pointer to buffer containing converted data (related to corresponding sensor characteristic).
 *
 *  @return Void.
 */

void convert_float_to_data(coord_float_t * gyro_coord_fl, coord_float_t * acc_coord_fl, sensor_gyro_data_t * data)
{
    data->acc.x = (int32_t)(acc_coord_fl->x * 100);
    data->acc.y = (int32_t)(acc_coord_fl->y * 100);
    data->acc.z = (int32_t)(acc_coord_fl->z * 100);
  
    data->gyro.x = (int32_t)(gyro_coord_fl->x * 100);
    data->gyro.y = (int32_t)(gyro_coord_fl->y * 100);
    data->gyro.z = (int32_t)(gyro_coord_fl->z * 100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function reads sensor measurements.
 *          
 *  @param  gyro_coord_fl  Pointer to gyro coordinate record in float format.
 *  @param  acc_coord_fl   Pointer to accel coordinate record in float format.
 *
 *  @return false in case that error is occurred, otherwise true.
 */

bool read_gyro_acc(coord_float_t * gyro_coord_fl, coord_float_t * acc_coord_fl) 
{   
    bool status = true;
	
	  // Enable I2C interface.
    i2c_enable(mpu.i2c);  
    
	  // Wakeup sensor.
    if(mpu6500_wakeup(&mpu))
    {
        nrf_delay_us(MPU6500_WAKEUP_TIME);
        
			  // Read measurements.
        while(mpu6500_get_gyro(&mpu, gyro_coord_fl) == false);
        while(mpu6500_get_acc(&mpu, acc_coord_fl) == false);
        
        // Go to sleep.			
        if(!mpu6500_sleep(&mpu))
        {
            status = false;
        } 
    } 
    else
    {
        status = false;
    }
    
		// Disable I2C interface.
    i2c_disable(mpu.i2c); 
    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for initializing some globals.
 *
 * @retval  Void.
 */

void globals_init(void) 
{ 
    sensor_gyro.led_state = DEFAULT_SENSOR_LED_STATE;
    memcpy((uint8_t*)&server_def.name, (uint8_t*)DEFAULT_DEVICE_NAME, BLE_DEVNAME_MAX_LEN);
    server_def.passkey = sensor_gyro.passkey;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief    This funciton do initialization of I2C interface to sensor, and default configuration of sensor.
 *
 *  @return  false in case that error is occurred, otherwise true.
 */

bool gyro_init(void) 
{
    bool status = true;
    
	  // Initialize sensor.
    if (!mpu6500_init(&mpu, TWI1_HW)) 
    {
        status = false;
    }
		// Read Who-Am-I register.
    else if (!mpu6500_who_am_i(&mpu, &whoami))
    {
        status = false;
    }
		// Do defalt config.
    else if(!mpu6500_config(&mpu, &sensor_gyro.config))
    {
        status = false;
    }
		// Read sensor measurement.
    else if(!read_gyro_acc(&gyro_coord_current, &acc_coord_current))
    {
        status = false;
    }
		// Converts to format related to corresponding sensor characteristic (sensor_gyro.data) and write to it.
    else
    {
        convert_float_to_data(&gyro_coord_current, &acc_coord_current, &sensor_gyro.data);
    }
    
		// Disable I2C.
    i2c_disable(mpu.i2c);  
    
    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief       Inits global with value stored in persistent_storage, or with default_value (if corresponding block in persistent_storage is empty).
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
    if(!init_global((uint8_t*)&sensor_gyro.sensorID, (uint8_t*)&DEFAULT_SENSOR_ID, sizeof(sensor_gyro.sensorID))) 
    {
        return false;
    }
    // Read SensorBeaconFrequency from p_storage.
    if(!init_global((uint8_t*)&sensor_gyro.beaconFrequency, (uint8_t*)&DEFAULT_SENSOR_BEACON_FREQUENCY, sizeof(sensor_gyro.beaconFrequency))) 
    {
        return false;
    }
    // Read SensorFrequency from p_storage.
    if(!init_global((uint8_t*)&sensor_gyro.frequency, (uint8_t*)&DEFAULT_SENSOR_FREQUENCY, sizeof(sensor_gyro.frequency))) 
    {
        return false;
    }
    // Read Threshold from p_storage.
    if(!init_global((uint8_t*)&sensor_gyro.threshold, (uint8_t*)&DEFAULT_SENSOR_THRESHOLD, sizeof(sensor_gyro.threshold))) 
    {
        return false;
    }
    else
    {
        convert_threshold_to_float(&sensor_gyro.threshold, &gyro_threshold, &acc_threshold);
    }
    // Read Config from p_storage.
    if(!init_global((uint8_t*)&sensor_gyro.config, (uint8_t*)&DEFAULT_SENSOR_CONFIG, sizeof(sensor_gyro.config))) 
    {
        return false;
    }
    // Read Passkey from p_storage.
    if(!init_global((uint8_t*)sensor_gyro.passkey, (uint8_t*)DEFAULT_SENSOR_PASSKEY, sizeof(sensor_gyro.passkey))) 
    {
        return false;
    }
        // Read mitm_req_flag from p_storage.
    if(!init_global((uint8_t*)&sensor_gyro.mitm_req_flag, (uint8_t*)&DEFAULT_MITM_REQ_FLAG, sizeof(sensor_gyro.mitm_req_flag))) 
    {
        return false;
    }
    
    return true;
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
    coord_float_t acc_coord_new;
    coord_float_t gyro_coord_new;
    
    // Reads sensor measurements.  
    read_gyro_acc(&gyro_coord_new, &acc_coord_new);
    
	  // Compare obtained values with threshold.
    if(
        (check_threshold_fl(&acc_threshold, acc_coord_current.x, acc_coord_new.x))    ||
        (check_threshold_fl(&acc_threshold, acc_coord_current.y, acc_coord_new.y))    ||
        (check_threshold_fl(&acc_threshold, acc_coord_current.z, acc_coord_new.z))    ||
    
        (check_threshold_fl(&gyro_threshold, gyro_coord_current.x, gyro_coord_new.x)) ||
        (check_threshold_fl(&gyro_threshold, gyro_coord_current.y, gyro_coord_new.y)) ||
        (check_threshold_fl(&gyro_threshold, gyro_coord_current.z, gyro_coord_new.z)) 
      )
   {
        // Update "current" values for coordinates.
        memcpy((uint8_t*)&acc_coord_current, (uint8_t*)&acc_coord_new, sizeof(acc_coord_current));
        memcpy((uint8_t*)&gyro_coord_current, (uint8_t*)&gyro_coord_new, sizeof(gyro_coord_current));
		 
		    // Converts to format related to corresponding sensor characteristic (sensor_gyro.data) and write to it.
        convert_float_to_data(&gyro_coord_current, &acc_coord_current, &sensor_gyro.data);
		    // Update characteristic.
        ble_update_characteristic_value(&characteristic_sensorData_r_info, (uint8_t *)(&sensor_gyro.data), sizeof(sensor_gyro.data));
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
        if (!ble_set_app_tick(app_tick_handler, sensor_gyro.frequency, NULL)) 
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
        ble_start_advertising(sensor_gyro.beaconFrequency);
        
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
    ble_start_advertising(sensor_gyro.beaconFrequency);
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
    if ((char_info == &characteristic_sensorID_info) && (offset == 0) && (len == sizeof(sensor_gyro.sensorID))) 
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_gyro.sensorID, data, len); 
        pstorage_driver_request_store((uint8_t*)(&sensor_gyro.sensorID));
    } 
    
		// Write to beacon frequency characteristic value?
    if ((char_info == &characteristic_sensorBeaconFrequency_info) && (offset == 0) && (len == sizeof(sensor_gyro.beaconFrequency))) 
    {
        beaconFrequency_t tmp_beaconFrequency;
        memcpy((uint8_t*)&tmp_beaconFrequency, data, len);    
      
			  // Check if new value is valid.
        if((tmp_beaconFrequency >= 20) && (tmp_beaconFrequency <= 10240))
        {
            sensor_gyro.beaconFrequency = tmp_beaconFrequency;
            pstorage_driver_request_store((uint8_t*)(&sensor_gyro.beaconFrequency));
        }
    }
    
		// Write to frequency characteristic value?
    if ((char_info == &characteristic_sensorFrequency_info) && (offset == 0) && (len == sizeof(sensor_gyro.frequency))) 
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_gyro.frequency, data, len);  
        pstorage_driver_request_store((uint8_t*)(&sensor_gyro.frequency));
			  // Set new interval for app timer.
        ble_set_app_tick(app_tick_handler, sensor_gyro.frequency, NULL);
    }
    
		// Write to led state characteristic value?
    if ((char_info == &characteristic_sensorLedState_info) && (offset == 0) && (len == sizeof(sensor_gyro.led_state))) 
    {
			  // Copy new data to local buffer, and update state of LED diode.
        memcpy((uint8_t*)&sensor_gyro.led_state, data, len);  
        led_control_update_char(sensor_gyro.led_state, LED_TIMEOUT_CHAR_MS);
    }
    
		// Write to threshold characteristic value?
    if ((char_info == &characteristic_sensorThreshold_info) && (offset == 0) && (len == sizeof(sensor_gyro.threshold))) 
    {
			  // Copy new data to local buffer, and update state of LED diode.
        memcpy((uint8_t*)&sensor_gyro.threshold, data, len);  
        pstorage_driver_request_store((uint8_t*)(&sensor_gyro.threshold));
			  // Converts sensor threshold format into float format.
        convert_threshold_to_float(&sensor_gyro.threshold, &gyro_threshold, &acc_threshold);
    }
    
		// Write to config characteristic value?
    if ((char_info == &characteristic_sensorConfig_info) && (offset == 0) && (len == sizeof(sensor_gyro.config))) 
    {
			  // Check if new value is valid.
        if( (data[0] <= GYRO_FULL_SCALE_2000DPS) && (data[1] <= ACC_FULL_SCALE_16G) )
        {
					  // Copy new data to local buffer, and update state of LED diode.
            memcpy((uint8_t*)&sensor_gyro.config, data, len); 
            pstorage_driver_request_store((uint8_t*)(&sensor_gyro.config));
					  // Write new configuration into sensor.
            i2c_enable(mpu.i2c);  
            mpu6500_config(&mpu, &sensor_gyro.config);
            i2c_disable(mpu.i2c);  
        }
    }
    
		// Write to sensor passkey characteristic value?
    if ((char_info == &characteristic_sensorPasskey_info) && (offset == 0) && (len == 6)) 
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_gyro.passkey, data, len);  
        pstorage_driver_request_store((uint8_t*)(&sensor_gyro.passkey));
			
			  // Set flag to clear bond manager.
        ble_clear_bondmngr_request();
    }
    
    // Write to working encryption mode characteristic value?
    if ((char_info == &characteristic_sensorMitmReqFlag_info) && (offset == 0) && (len == sizeof(sensor_gyro.mitm_req_flag)))
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        sensor_gyro.mitm_req_flag = (data[0] == 1);
        pstorage_driver_request_store((uint8_t*)(&sensor_gyro.mitm_req_flag));
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
  
	  // Initialize some global variables.
    globals_init(); 
    
	  // Initialize BLE server modules.
    if (!ble_init_server(&server_def, pstorage_driver_init, &sensor_gyro.mitm_req_flag)) 
    { 
        blink(101); 
    }
    
		// Initialize services and characteristics for non-onboarding mode.
    if(onboard_get_mode() == ONBOARD_MODE_IDLE)
    {
			  // Check whether there is a need for MITM encryption.	
        uint16_t read_enc_flag  = (sensor_gyro.mitm_req_flag ? BLE_CHARACTERISTIC_READ_ENC_REQUIRE  : BLE_CHARACTERISTIC_READ_ENC_REQUIRE_NO_MITM);
        uint16_t write_enc_flag = (sensor_gyro.mitm_req_flag ? BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE : BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE_NO_MITM);
      
			  // Initialize sensor.
        if (!gyro_init()) 
        {
            blink(102);
        }
    
        short_service_uuid = sensor_gyro.mitm_req_flag ? short_service_relayr_uuid : short_service_relayr_open_comm_uuid; 
        
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
                                    (const uint8_t*)sensor_gyro.sensorID,
                                    sizeof(sensorID_t),
                                    &characteristic_sensorID_info)) 
        {
            blink(104);
        }
      
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorBeaconFrequency_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorBeaconFrequency", 
                                    (const uint8_t*)&sensor_gyro.beaconFrequency,
                                    sizeof(sensor_gyro.beaconFrequency),
                                    &characteristic_sensorBeaconFrequency_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorFrequency_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorFrequency", 
                                    (const uint8_t*)&sensor_gyro.frequency,
                                    sizeof(sensor_gyro.frequency),
                                    &characteristic_sensorFrequency_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorLedState_uuid,
                                    BLE_CHARACTERISTIC_CAN_WRITE | write_enc_flag,
                                    (const uint8_t*)"SensorLedState", 
                                    (const uint8_t*)&sensor_gyro.led_state,
                                    sizeof(sensor_gyro.led_state),
                                    &characteristic_sensorLedState_info)) 
        {
            blink(104);
        };
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorThreshold_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorThreshold", 
                                    (const uint8_t*)&sensor_gyro.threshold,
                                    sizeof(sensor_gyro.threshold),
                                    &characteristic_sensorThreshold_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorConfig_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorConfig",
                                    (const uint8_t*)&sensor_gyro.config,
                                    sizeof(sensor_gyro.config),
                                    &characteristic_sensorConfig_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorData_r_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_NOTIFY | BLE_CHARACTERISTIC_CAN_INDICATE | read_enc_flag,
                                    (const uint8_t*)"SensorData", 
                                    (const uint8_t*)&sensor_gyro.data,
                                    sizeof(sensor_gyro.data),
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
                                (const uint8_t*)sensor_gyro.sensorID,
                                sizeof(sensorID_t),
                                &characteristic_sensorID_info)) 
        {
            blink(104);
        }
      
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorPasskey_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE,
                                    (const uint8_t*)"SensorPasskey", 
                                    (const uint8_t*)sensor_gyro.passkey,
                                    6,
                                    &characteristic_sensorPasskey_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorMitmReqFlag_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE,
                                    (const uint8_t*)"SensorMitmRequireFlag", 
                                    (const uint8_t*)&sensor_gyro.mitm_req_flag,
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
    if (!ble_start_advertising(sensor_gyro.beaconFrequency)) 
    {
        blink(106);
    }
    
    ble_run();

    blink(107);
} 
