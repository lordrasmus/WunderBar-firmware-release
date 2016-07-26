/** @file  ble_driver.h
 *  @brief Bluetooth Low Energy driver.
 *
 *  This contains the declaration for the BLE
 *  driver and eventually any macros, constants,
 *  or global variables you will need.
 *
 *  @author MikroElektronika
 *  @bug No known bugs.
 */

#ifndef _BLE_H_
#define _BLE_H_

#include "types.h"
#include "gpio.h"
#include "ble_gap.h"
#include "ble_gatts.h"
#include "pstorage_driver.h"
#include "wunderbar_common.h"
#include "nrf_gpio.h"

#define BLE_MAX_SERVICES_PER_PROFILE 5
#define BLE_MAX_CHARACTERISTICS_PER_SERVICE 5

#define BLE_DEVNAME_MAX_LEN           14

#define MANUFACTURER_NAME             "Relayr"                                    /**< Manufacturer. Will be passed to Device Information Service. */
#define HARDWARE_REVISION             "1.0.2"                                     /**< Hardware Revision String. */
#define FIRMWARE_REVISION             "1.0.0"                                     /**< Firmware Revision String. */

#define APP_MTU_SIZE                    23

#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(app_timer_event_t)                  /**< Maximum size of scheduler events. Note that scheduler BLE stack events do not contain any data, as the events are being pulled from the stack in the event handler. */
#define SCHED_QUEUE_SIZE                20                                          /**< Maximum number of events in the scheduler queue. */

#define APP_ADV_TIMEOUT_IN_SECONDS      180                                         /**< The advertising timeout (in units of seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_TIMEOUT               30                                          /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  1                                           /**< Man In The Middle protection required. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_KEYBOARD_ONLY               /**< Keyboard only capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#define FLASH_PAGE_SYS_ATTR              (BLE_FLASH_PAGE_END - 3)                   /**< Flash page used for bond manager system attribute information. */
#define FLASH_PAGE_BOND                  (BLE_FLASH_PAGE_END - 1)                   /**< Flash page used for bond manager bonding information. */

#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS            5                                           /**< Maximum number of simultaneously created timers. 1 for battery measurement, 1 for LED, 1 for connection parameters module */
#define APP_TIMER_OP_QUEUE_SIZE         6                                           /**< Size of timer operation queues. */

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS   1200                                        /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION    3                                           /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
        ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / 1023) * ADC_PRE_SCALING_COMPENSATION) /** <Convert the result of ADC conversion in millivolts. */

#define BUTTON_PIN                      22
#define LED_PIN                         29

#define USE_SCHEDULER false
#define USE_BONDMGR true

#define MAX_ADV_UUIDS 10


typedef enum 
{
  BLE_CHARACTERISTIC_BROADCAST                  =    1,
  BLE_CHARACTERISTIC_CAN_READ                   =    2,
  BLE_CHARACTERISTIC_CAN_WRITE_WO_RESPONSE      =    4,
  BLE_CHARACTERISTIC_CAN_WRITE                  =    8,
  BLE_CHARACTERISTIC_CAN_NOTIFY                 =   16,
  BLE_CHARACTERISTIC_CAN_INDICATE               =   32,
  BLE_CHARACTERISTIC_CAN_AUTH_SIGNED_WRITE      =   64,
  BLE_CHARACTERISTIC_CAN_RELIABLE_WRITE         =  128,
  BLE_CHARACTERISTIC_CAN_WRITE_AUX              =  256,
  BLE_CHARACTERISTIC_READ_ENC_REQUIRE           =  512,
  BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE          = 1024,
  BLE_CHARACTERISTIC_READ_ENC_REQUIRE_NO_MITM   = 2048,
  BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE_NO_MITM  = 4096
} 
ble_characteristic_flags_t;
  

typedef enum 
{
  BLE_CHARACTERISTIC_IS_NOTIFYING  = 1,
  BLE_CHARACTERISTIC_IS_INDICATING = 2
} 
ble_characteristic_state_t;


typedef enum 
{
  BLE_NOTIFICATION_START = 1,
  BLE_INDICATION_START   = 2,
  BLE_NOTIFICATION_END   = 3,
  BLE_INDICATION_END     = 4
} 
ble_subscription_change_t;


typedef enum 
{
  BLE_ERR_OK = 0,
  BLE_ERR_ADV_UUIDS_FULL    = 10000,  //no more slots for advertising available
  BLE_ERR_INVALID_PARAMETER = 10001   //invalid parameter passed in
} 
ble_error_t;


/** everything we need to know about a service during runtime about a service */
typedef struct 
{
  uint16_t short_uuid;
  uint16_t uuid_type;
  uint16_t service_handle;
} 
__attribute__((packed)) ble_service_info_t;


/** everything we need to know about a service during runtime about a charcteristic */
typedef struct 
{
  uint16_t value_handle;
  uint16_t user_desc_handle;
  uint16_t cccd_handle;
  uint16_t sccd_handle;
  uint16_t flags;
  uint16_t state;
} 
__attribute__((packed)) ble_characteristic_info_t;


typedef void (*ble_connection_callback_t)();  
typedef void (*ble_disconnection_callback_t)();
typedef void (*ble_advertising_timeout_callback_t)();
typedef void (*ble_raw_write_callback_t)(ble_gatts_evt_write_t * evt_write);
typedef void (*ble_characteristic_write_callback_t)(ble_characteristic_info_t* char_info, uint16_t offset, uint16_t len, uint8_t* data);
typedef void (*ble_subscription_callback_t)(ble_characteristic_info_t* char_info, ble_subscription_change_t change);
typedef void (*ble_app_tick_callback_t)(void* context);
typedef void (*ble_main_thread_callback_t)(void);


/** BLE server definition - a profile with flags, callbacks etc. */
typedef struct 
{
  ble_connection_callback_t           connection_callback;
  ble_disconnection_callback_t        disconnection_callback;
  ble_advertising_timeout_callback_t  advertising_timeout_callback;
  ble_raw_write_callback_t            write_raw_callback;
  ble_characteristic_write_callback_t write_characteristic_callback;
  ble_main_thread_callback_t          main_thread_callback;
  ble_subscription_callback_t         subscription_callback;
  uint8_t *                           passkey;
  uint8_t                             name[BLE_DEVNAME_MAX_LEN];
}
ble_server_definition_t;

void ble_start_led_timeout(void);
  
float f_abs(float fl);

bool check_threshold_fl(threshold_float_t * threshold, float old_value, float new_value);
bool check_threshold_int(threshold_int16_t * threshold, int16_t old_value, int16_t new_value);

void ble_enable_input_interrupt(void);
void ble_disable_input_interrupt(void);

/**@brief   Function for handling a Connection Parameters error.
 *
 * @param   nrf_error   Error code containing information about what went wrong.
 *
 * @return  Void.
 */
bool ble_disconnect(void);
 
/**@brief Start updating value of battery level.
 *
 * @return Void.
 */
void ble_battery_start(void);

/**@brief   Function to be executed when the battery timer expires.
 *
 * @details This function checks if device is connected, and if so starts updating of battery level.
 *
 * @param   context General purpose pointer.
 *
 * @return  Void.
 */
bool ble_is_device_connected(void);

/**@brief   Function for initializing the Advertising functionality.
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_init_advertising(void);

/**@brief   Function sets flag which used during disconnection process to clear bonded central database from flash.
 *
 * @return  Void.
 */
void ble_clear_bondmngr_request(void);

/**@brief   Function for initializing BLE server modules.
 *
 * @param   definition            Callbacks and properties data related to BLE server.
 * @param   pstorage_driver_init  Pointer to function which initialize and configure pstorage, and registers characteristic values to corresponding blocks in persistent memory..
 * @param   mitm_req_flag         Flag which indicates is there Man In The Middle protection required.
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_init_server(const ble_server_definition_t* definition, pstorage_driver_init_t pstorage_driver_init, bool * mitm_req_flag);

/**@brief   Adding Battery Service
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_add_bat_service(void);

/**@brief   Adding Device Information Service
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_add_device_information_service(void);

/**@brief   Adding Custom service.
 *
 * @param   short_uuid 16-bit UUID value
 * @param   long_uuid  16-octet (128-bit) little endian Vendor Specific UUID
 * @param   flags      Not-used for now 
 * @param   info       Service properties.
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_add_service(const uint16_t short_uuid, const uint8_t* long_uuid, const uint16_t flags, ble_service_info_t* info);

/**@brief   Adding Characteristic to the Custom service.
 *
 * @param   service       Service properties.
 * @param   char_uuid     Characteristic 16-bit UUID value.
 * @param   flags         Set permissions and authorization properties of attribute metadata.
 * @param   user_desc     User descriptor.
 * @param   init_value    Used for initialization of characteristic value.
 * @param   data_len      Maximum attribute value length in bytes.
 * @param   info          Characteristic properties.
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_add_characteristic(const ble_service_info_t* service, const uint16_t char_uuid, const uint16_t flags, const uint8_t* user_desc, const uint8_t* init_value, const uint16_t data_len, ble_characteristic_info_t* info);

/**@brief   Set the value of the attribute, and notify or indicate an attribute value (if notification/inidication enabled).
 *
 * @param   char_info  Characteristic properties.
 * @param   data       Buffer containing the desired attribute value
 * @param   len        Length in bytes to be written.
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_update_characteristic_value(ble_characteristic_info_t* char_info, uint8_t* data, uint16_t len);

/**@brief   Function for starting BLE server.
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_start_server(void);

/**@brief   Main thread loop.
 *
 * @return  Void.
 */
void ble_run(void);

/**@brief   Function return value of last_error variable.
 *
 * @return  Value of last_error variable.
 */
uint32_t ble_get_error(void);

/**@brief   Function to be called for starting advertising.
 *
 * @param   beacon_frequency  Adverdising interval in ms.
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_start_advertising(uint32_t beacon_frequency);

/**@brief   Function to be called for stoping advertising.
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_stop_advertising(void);

/**@brief   Dispatches a raw write operation to a characteristic. Should be called within a raw write callback with all respective characteristics.
 *
 * @param   handle     Characteristic handle.
 * @param   offset     Offset for the write operation.
 * @param   len        Length of the incoming data.
 * @param   data       Incoming data.
 * @param   char_info  Characteristic properties. 
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_dispatch_write_characteristic(const uint16_t handle, uint16_t offset, uint16_t len, uint8_t* data, ble_characteristic_info_t* char_info);

/**@brief   Clear characteristic state properties.
 *
 * @param   char_info  Pointer to characteristic properties.
 *
 * @return  Void.
 */
void ble_reset_characteristic(ble_characteristic_info_t* char_info);

/**@brief   Puts the chip in System OFF mode.
 *
 * @return  Void.
 */
void ble_shutdown(void);

/** an input callback */
typedef void (*ble_input_callback_t)(void);

/**@brief   Set callback for external interrupt.
 *
 * @param   pin_no     Pin number
 * @param   sense      Selecting the pin to sense high or low level on the pin input.
 * @param   pull_mode  Selecting the pin to be pulled down or up.
 * @param   callback   Callback to be called when external interrupt occurred.
 *
 * @return  Void.
 */
void ble_set_input_callback(uint8_t pin_no, nrf_gpio_pin_sense_t sense, nrf_gpio_pin_pull_t pull_mode, ble_input_callback_t callback);

/**@brief   Function to be called to set application timer interval and callback
 *
 * @param   callback   Callback to be called when timer timeout occurred.
 * @param   interval   Number of ms to timeout event.
 * @param   context    General purpose pointer.
 *
 * @return  false in case error occurred, otherwise true.
 */
bool ble_set_app_tick(ble_app_tick_callback_t callback, uint32_t interval, void* context);

#endif  /* _BLE_H_ */
