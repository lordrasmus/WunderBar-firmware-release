
/** @file   main.c
 *  @brief  This file contains the source code for firmware of MICROPHONE sensor of Wunderbar Board.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */
 
/* -- Includes -- */

#include <stdint.h>
#include <string.h>
#include "nrf_delay.h"
#include "ble_driver.h"
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

const uint8_t  long_service_uuid[]                        = {0x78,0x6d,0x2e,0x96,0x6f,0xd1,0x42,0x2f,0x8e,0x87,0x15,0x93,0xff,0xff,0x5a,0x09}; /**< 16-octet (128-bit) UUID. */


const uint16_t short_service_relayr_uuid                  = SHORT_SERVICE_RELAYR_UUID;                       /**< 16-bit UUID value for relayr service. */
const uint16_t short_service_relayr_open_comm_uuid        = SHORT_SERVICE_RELAYR_OPEN_COMM_UUID;             /**< 16-bit UUID value for relayr service which not require encryption. */
const uint16_t short_service_config_uuid                  = SHORT_SERVICE_CONFIG_UUID;                       /**< 16-bit UUID value for onboarding service. */

const uint16_t characteristic_sensorID_uuid               = CHARACTERISTIC_SENSOR_ID_UUID;                   /**< 16-bit UUID value for sensor ID char. */
const uint16_t characteristic_sensorBeaconFrequency_uuid  = CHARACTERISTIC_SENSOR_BEACON_FREQUENCY_UUID;     /**< 16-bit UUID value for beacon frequency char. */
const uint16_t characteristic_sensorFrequency_uuid        = CHARACTERISTIC_SENSOR_FREQUENCY_UUID;            /**< 16-bit UUID value for frequency char. */
const uint16_t characteristic_sensorLedState_uuid         = CHARACTERISTIC_SENSOR_LED_STATE_UUID;            /**< 16-bit UUID value for led state char. */
const uint16_t characteristic_sensorThreshold_uuid        = CHARACTERISTIC_SENSOR_THRESHOLD_UUID;            /**< 16-bit UUID value for threshold char. */
const uint16_t characteristic_sensorData_r_uuid           = CHARACTERISTIC_SENSOR_DATA_R_UUID;               /**< 16-bit UUID value for data up char. */
const uint16_t characteristic_sensorPasskey_uuid          = CHARACTERISTIC_SENSOR_PASSKEY_UUID;              /**< 16-bit UUID value for sensor passkey char. */
const uint16_t characteristic_sensorMitmReqFlag_uuid      = CHARACTERISTIC_SENSOR_MITM_REQ_FLAG_UUID;        /**< 16-bit UUID value for working encryption mode char. */ 

/**@brief  Records related to characteristics. */
ble_characteristic_info_t characteristic_sensorID_info;
ble_characteristic_info_t characteristic_sensorBeaconFrequency_info;
ble_characteristic_info_t characteristic_sensorFrequency_info;
ble_characteristic_info_t characteristic_sensorLedState_info;
ble_characteristic_info_t characteristic_sensorThreshold_info;
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
    NULL                               /**< Device name. */
};
ble_service_info_t service_info;

#define OPAMP_SHDW_PIN        12       
#define CONVERTER_ENABLE_PIN  11
#define SWITCH_ON_PIN         13
#define MIC_TIMER_SENSE_PIN   7

#define ENABLING_AN_TIMEOUT_MS 150

sensor_microphone_t sensor_microphone;
uint16_t            short_service_uuid;

/**@brief  Default values for corresponding characteristics. These values are used if corresponding block of persistent storage is empty. */
const uint8_t                DEFAULT_DEVICE_NAME[BLE_DEVNAME_MAX_LEN+1] = DEVICE_NAME_MIC; 
const sensorID_t             DEFAULT_SENSOR_ID = {0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
                                                  0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44};
const beaconFrequency_t      DEFAULT_SENSOR_BEACON_FREQUENCY            = ADV_INTERVAL_MS;
const frequency_t            DEFAULT_SENSOR_FREQUENCY                   = 1000;
const led_state_t            DEFAULT_SENSOR_LED_STATE                   = false;
const sensor_microphone_threshold_t DEFAULT_THRESHOLD                   = {0, 0x8000, 0x7FFF};
const passkey_t              DEFAULT_SENSOR_PASSKEY                     = "000000";
const security_level_t       DEFAULT_MITM_REQ_FLAG                      = true;


typedef enum
{
    MIC_TIMER_STATE_WAIT        = 0,
    MIC_TIMER_STATE_ENABLING_AN = 1,
}
mic_timer_state_t;
mic_timer_state_t mic_timer_state = MIC_TIMER_STATE_WAIT;

// ------------------
// --- Misc utils ---
// ------------------

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
/** @brief  This function enables analog circuit.
 *
 *  @return Void.
 */

void enable_analog_circuit(void)
{
    gpio_write(SWITCH_ON_PIN, true);
    gpio_write(OPAMP_SHDW_PIN, true);
    gpio_write(CONVERTER_ENABLE_PIN, false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function disables analog circuit.
 *
 *  @return Void.
 */

void disable_analog_circuit(void)
{
    gpio_write(CONVERTER_ENABLE_PIN, true);
    gpio_write(OPAMP_SHDW_PIN, false);
    gpio_write(SWITCH_ON_PIN, false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function reads microphone level.
 *
 *  @return microphone rms level.
 */

uint16_t get_mic_level()
{
    uint16_t adc_result;
  
    while(NRF_ADC->BUSY == 1);
    NRF_ADC->EVENTS_END  = 0;
    NRF_ADC->INTENCLR    = ADC_INTENCLR_END_Enabled;
    NRF_ADC->CONFIG      = (ADC_CONFIG_RES_10bit                            << ADC_CONFIG_RES_Pos)     |
                           (ADC_CONFIG_INPSEL_AnalogInputNoPrescaling       << ADC_CONFIG_INPSEL_Pos)  |
                           (ADC_CONFIG_REFSEL_VBG                           << ADC_CONFIG_REFSEL_Pos)  |
                           (ADC_CONFIG_PSEL_AnalogInput7                    << ADC_CONFIG_PSEL_Pos)    |
                           (ADC_CONFIG_EXTREFSEL_None                       << ADC_CONFIG_EXTREFSEL_Pos);
    NRF_ADC->ENABLE      = ADC_ENABLE_ENABLE_Enabled;
    NRF_ADC->TASKS_START = 1;
    
    while(NRF_ADC->EVENTS_END == 0);
    
    NRF_ADC->EVENTS_END = 0;
    adc_result          = NRF_ADC->RESULT;
    
    NRF_ADC->TASKS_STOP = 1;
    
    disable_analog_circuit();
    
    return adc_result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for initializing some globals.
 *
 * @retval  Void.
 */

bool comfort_init() 
{ 
    sensor_microphone.led_state = DEFAULT_SENSOR_LED_STATE;
    memcpy((uint8_t*)&server_def.name, (uint8_t*)DEFAULT_DEVICE_NAME, BLE_DEVNAME_MAX_LEN);
    server_def.passkey =  sensor_microphone.passkey;
    sensor_microphone.data.mic_level = get_mic_level();
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
    if(!init_global((uint8_t*)&sensor_microphone.sensorID, (uint8_t*)&DEFAULT_SENSOR_ID, sizeof(sensor_microphone.sensorID))) 
    {
        return false;
    }
    // Read SensorBeaconFrequency from p_storage.
    if(!init_global((uint8_t*)&sensor_microphone.beaconFrequency, (uint8_t*)&DEFAULT_SENSOR_BEACON_FREQUENCY, sizeof(sensor_microphone.beaconFrequency))) 
    {
        return false;
    }
    // Read SensorFrequency from p_storage.
    if(!init_global((uint8_t*)&sensor_microphone.frequency, (uint8_t*)&DEFAULT_SENSOR_FREQUENCY, sizeof(sensor_microphone.frequency))) 
    {
        return false;
    }
    // Read Threshold from p_storage.
    if(!init_global((uint8_t*)&sensor_microphone.threshold, (uint8_t*)&DEFAULT_THRESHOLD, sizeof(sensor_microphone.threshold))) 
    {
        return false;
    }
    // Read Passkey from p_storage.
    if(!init_global((uint8_t*)sensor_microphone.passkey, (uint8_t*)DEFAULT_SENSOR_PASSKEY, sizeof(sensor_microphone.passkey))) 
    {
        return false;
    }
    // Read mitm_req_flag from p_storage.
    if(!init_global((uint8_t*)&sensor_microphone.mitm_req_flag, (uint8_t*)&DEFAULT_MITM_REQ_FLAG, sizeof(sensor_microphone.mitm_req_flag))) 
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
    uint16_t new_sample;
   
    // Reads microphone level.
    new_sample = get_mic_level();
    
	  // Compare obtained values with threshold.
    if(check_threshold_int(&sensor_microphone.threshold.mic_level, sensor_microphone.data.mic_level, new_sample))
    {
        sensor_microphone.data.mic_level = new_sample;
			
			  // Update characteristic.
        ble_update_characteristic_value(&characteristic_sensorData_r_info,  (uint8_t *)(&sensor_microphone.data),  sizeof(sensor_microphone.data));
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
    
		// Interval defined by sensor_microphone.frequency exceeded?
    if(mic_timer_state == MIC_TIMER_STATE_WAIT)
    {
			  // Enable analog circuit.
        enable_analog_circuit();
			  
			  // Set timer to wait to stabilize analog circuit.
        ble_set_app_tick(app_tick_handler, ENABLING_AN_TIMEOUT_MS, NULL);
        
			  // Go to corresponding state.
        mic_timer_state = MIC_TIMER_STATE_ENABLING_AN;
    }
		// Interval for stabilizing analog circuit exceeded?
    else if(mic_timer_state == MIC_TIMER_STATE_ENABLING_AN)
    {
			  // Reads new microhone level.
        get_sensor_data();
        ble_set_app_tick(app_tick_handler, sensor_microphone.frequency, NULL);
        
			  // Go to default state.
        mic_timer_state = MIC_TIMER_STATE_WAIT;
    }
    
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
        mic_timer_state = MIC_TIMER_STATE_WAIT;
        led_control_update_char(true, LED_TIMEOUT_CONNECTION_MS);			
			  
			  // Start timer for polling sensor data.
        if (!ble_set_app_tick(app_tick_handler, sensor_microphone.frequency, NULL)) 
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
        ble_start_advertising(sensor_microphone.beaconFrequency);
        
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
    ble_start_advertising(sensor_microphone.beaconFrequency);
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
    if ((char_info == &characteristic_sensorID_info) && (offset == 0) && (len == sizeof(sensor_microphone.sensorID))) 
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_microphone.sensorID, data, len); 
        pstorage_driver_request_store((uint8_t*)(&sensor_microphone.sensorID));
    }
    
		// Write to beacon frequency characteristic value?
    if ((char_info == &characteristic_sensorBeaconFrequency_info) && (offset == 0) && (len == sizeof(sensor_microphone.beaconFrequency))) 
    {
        beaconFrequency_t tmp_beaconFrequency;
        memcpy((uint8_t*)&tmp_beaconFrequency, data, len);    
        
			  // Check if new value is valid.
        if((tmp_beaconFrequency >= 20) && (tmp_beaconFrequency <= 10240))
        {
            sensor_microphone.beaconFrequency = tmp_beaconFrequency;
            pstorage_driver_request_store((uint8_t*)(&sensor_microphone.beaconFrequency));
        }
    }
    
		// Write to frequency characteristic value?
    if ((char_info == &characteristic_sensorFrequency_info) && (offset == 0) && (len == sizeof(sensor_microphone.frequency))) 
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_microphone.frequency, data, len);  
        pstorage_driver_request_store((uint8_t*)(&sensor_microphone.frequency));
    }
    
		// Write to led state characteristic value?
    if ((char_info == &characteristic_sensorLedState_info) && (offset == 0) && (len == sizeof(sensor_microphone.led_state))) 
    {
			  // Copy new data to local buffer, and update state of LED diode.
        memcpy((uint8_t*)&sensor_microphone.led_state, data, len);  
			  led_control_update_char(sensor_microphone.led_state, LED_TIMEOUT_CHAR_MS);

    }
    
		// Write to threshold characteristic value?
    if ((char_info == &characteristic_sensorThreshold_info) && (offset == 0) && (len == sizeof(sensor_microphone.threshold))) 
    {
			  // Copy new data to local buffer, and update state of LED diode.
        memcpy((uint8_t*)&sensor_microphone.threshold, data, len);  
        pstorage_driver_request_store((uint8_t*)(&sensor_microphone.threshold));
    } 
    
		// Write to sensor passkey characteristic value?
    if ((char_info == &characteristic_sensorPasskey_info) && (offset == 0) && (len == 6)) 
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_microphone.passkey, data, len);  
        pstorage_driver_request_store((uint8_t*)(&sensor_microphone.passkey));
			
			  // Set flag to clear bond manager.
        ble_clear_bondmngr_request();
    }
        
		// Write to working encryption mode characteristic value?
    if ((char_info == &characteristic_sensorMitmReqFlag_info) && (offset == 0) && (len == sizeof(sensor_microphone.mitm_req_flag)))
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        sensor_microphone.mitm_req_flag = (data[0] == 1);
        pstorage_driver_request_store((uint8_t*)(&sensor_microphone.mitm_req_flag));
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
  
	  // Config pins for controling switchs of analog circuit modules.
    gpio_set_pin_digital_output(SWITCH_ON_PIN, PIN_DRIVE_S0S1);
    gpio_set_pin_digital_output(CONVERTER_ENABLE_PIN, PIN_DRIVE_S0S1);
    gpio_set_pin_digital_output(OPAMP_SHDW_PIN, PIN_DRIVE_S0S1);
  
    disable_analog_circuit();                                                                      // Disable analog circuit.
  
	  // Initialize some global variables.  
    if (!comfort_init()) 
    {
        blink(101); 
    }
    
		// Initialize BLE server modules.
    if (!ble_init_server(&server_def, pstorage_driver_init, &sensor_microphone.mitm_req_flag)) 
    { 
        blink(101); 
    }
    
		// Initialize services and characteristics for non-onboarding mode.
    if(onboard_get_mode() == ONBOARD_MODE_IDLE)
    {
			  // Check whether there is a need for MITM encryption.
        uint16_t read_enc_flag  = (sensor_microphone.mitm_req_flag ? BLE_CHARACTERISTIC_READ_ENC_REQUIRE  : BLE_CHARACTERISTIC_READ_ENC_REQUIRE_NO_MITM);
        uint16_t write_enc_flag = (sensor_microphone.mitm_req_flag ? BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE : BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE_NO_MITM);
        
        short_service_uuid = sensor_microphone.mitm_req_flag ? short_service_relayr_uuid : short_service_relayr_open_comm_uuid; 
        
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
                                    (const uint8_t*)sensor_microphone.sensorID,
                                    sizeof(sensorID_t),
                                    &characteristic_sensorID_info)) 
        {
            blink(104);
        }
      
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorBeaconFrequency_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorBeaconFrequency", 
                                    (const uint8_t*)&sensor_microphone.beaconFrequency,
                                    sizeof(sensor_microphone.beaconFrequency),
                                    &characteristic_sensorBeaconFrequency_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorFrequency_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorFrequency", 
                                    (const uint8_t*)&sensor_microphone.frequency,
                                    sizeof(sensor_microphone.frequency),
                                    &characteristic_sensorFrequency_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorLedState_uuid,
                                    BLE_CHARACTERISTIC_CAN_WRITE | write_enc_flag,
                                    (const uint8_t*)"SensorLedState", 
                                    (const uint8_t*)&sensor_microphone.led_state,
                                    sizeof(sensor_microphone.led_state),
                                    &characteristic_sensorLedState_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorThreshold_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorThreshold", 
                                    (const uint8_t*)&sensor_microphone.threshold,
                                    sizeof(sensor_microphone.threshold),
                                    &characteristic_sensorThreshold_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorData_r_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_NOTIFY | BLE_CHARACTERISTIC_CAN_INDICATE | read_enc_flag,
                                    (const uint8_t*)"SensorData", 
                                    (const uint8_t*)&sensor_microphone.data,
                                    sizeof(sensor_microphone.data),
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
                                    (const uint8_t*)sensor_microphone.sensorID,
                                    sizeof(sensorID_t),
                                    &characteristic_sensorID_info)) 
        {
            blink(104);
        }  
      
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorPasskey_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE,
                                    (const uint8_t*)"SensorPasskey", 
                                    (const uint8_t*)sensor_microphone.passkey,
                                    6,
                                    &characteristic_sensorPasskey_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorMitmReqFlag_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE,
                                    (const uint8_t*)"SensorMitmRequireFlag", 
                                    (const uint8_t*)&sensor_microphone.mitm_req_flag,
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
    if (!ble_start_advertising(sensor_microphone.beaconFrequency)) 
    {
        blink(106);
    }
    
    ble_run();

    blink(107);
} 
