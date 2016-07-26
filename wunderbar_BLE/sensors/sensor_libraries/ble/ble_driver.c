
/** @file  ble_driver.c
 *  @brief Bluetooth Low Energy driver.
 *
 *  This contains the definitions for the BLE
 *  driver and corresponding macros, constants,
 *  and global variables.
 *
 *  @author MikroElektronika
 *  @bug No known bugs.
 */

/* -- Includes -- */

#include "nrf_sdm.h"
#include "nrf_soc.h"
#include "nrf_svc.h"
#include "nrf_assert.h"
#include "ble.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "ble_hci.h"
#include "ble_bas.h"
#include "ble_bas.h"
#include "ble_flash.h"
#include "nordic_common.h"
#include "app_timer.h"
#include "app_gpiote.h"
#include "softdevice_handler.h"
#include "pstorage.h"
#include "ble_bondmngr.h"
#include "ble_dis.h"
#include "ble_gap.h"
#include "ble_driver.h"
#include "gpio.h"
#include "gpiote.h" 
#include "wunderbar_common.h"
#include "led_control.h"
#include "onboard.h"


static void my_app_tick_handler(void* context);
void pstorage_sys_event_handler (uint32_t p_evt);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Global variables and constants.
 */

const ble_server_definition_t* server_definition = NULL;

static uint32_t                last_error = 0;

static bool                    bas_enabled = false;                     
static ble_bas_t               battery_service;                // Structure used to identify the battery service
static ble_gap_sec_params_t    sec_params;

static ble_uuid_t              adv_uuids[MAX_ADV_UUIDS];
static uint8_t                 adv_uuid_count = 0;

static uint16_t                conn_handle = BLE_CONN_HANDLE_INVALID;

static app_timer_id_t          battery_timer_id;                         /**< Battery measurement timer. */

static ble_input_callback_t      my_input_callback;

static app_timer_id_t          tick_timer_id;
static ble_app_tick_callback_t app_tick_callback;

static bool                    clear_bondmngr_flag = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(DEBUG_NRF)
void assert_nrf_callback(uint16_t line_num, const uint8_t *file_name)
{
  (void) file_name; /* Unused parameter */
  (void) line_num;  /* Unused parameter */

  NVIC_SystemReset();
}
#endif /* DEBUG_NRF */

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Custom app stuff.
 */
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief Set state of LED_PIN.
 *
 *  @param on New LED state.
 *
 *  @return Void.
 */

void led(bool on) 
{
    gpio_write(LED_PIN, on);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief Error handler for bond manager.
 *
 *  @param nrf_error Error code.
 *
 *  @return Void.
 */

void ble_error(uint32_t nrf_error) 
{
    ;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief Error handler function.
 *
 *  @param error_code  Error code supplied to the handler.
 *  @param line_num    Line number where the handler is called.
 *  @param p_file_name Pointer to the file name. 
 *
 *  @return Void.
 */

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name) 
{
    NVIC_SystemReset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief Check state of sensor connection.
 *
 * @retval true if sensor connected.
 */

bool ble_is_device_connected()
{
    return (conn_handle != BLE_CONN_HANDLE_INVALID);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief Function for handling the ADC interrupt.
 *
 * @details  This function will fetch the conversion result from the ADC, convert the value into
 *           percentage and send it to peer.
 *
 *  @return Void.
 */

void ADC_IRQHandler(void)
{
    uint16_t    batt_lvl_in_milli_volts;
    uint8_t     percentage_batt_lvl;  
    uint16_t    adc_result;

    if (NRF_ADC->EVENTS_END != 0)
    {
        uint32_t    err_code;

        NRF_ADC->EVENTS_END     = 0;
        adc_result              = NRF_ADC->RESULT;
        NRF_ADC->TASKS_STOP     = 1;

        batt_lvl_in_milli_volts = ADC_RESULT_IN_MILLI_VOLTS(adc_result);                           // Get battery level in millivolts.
        percentage_batt_lvl = battery_level_in_percent(batt_lvl_in_milli_volts);                   // Convert to percentage.
        err_code = ble_bas_battery_level_update(&battery_service, percentage_batt_lvl);            // Update value of battery characteristic.
        if (                                                                                       // Check for error.  
            (err_code != NRF_SUCCESS)
            &&
            (err_code != NRF_ERROR_INVALID_STATE)
            &&
            (err_code != BLE_ERROR_NO_TX_BUFFERS)
            &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
        )
        {
            APP_ERROR_HANDLER(err_code);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief Start updating value of battery level.
 *
 * @return Void.
 */

void ble_battery_start(void)
{
    uint32_t err_code;
    
    while(NRF_ADC->BUSY == 1);
  
    // Configure ADC
    NRF_ADC->INTENSET   = ADC_INTENSET_END_Msk;
    NRF_ADC->CONFIG     = (ADC_CONFIG_RES_10bit                       << ADC_CONFIG_RES_Pos)     |
                          (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos)  |
                          (ADC_CONFIG_REFSEL_VBG                      << ADC_CONFIG_REFSEL_Pos)  |
                          (ADC_CONFIG_PSEL_Disabled                   << ADC_CONFIG_PSEL_Pos)    |
                          (ADC_CONFIG_EXTREFSEL_None                  << ADC_CONFIG_EXTREFSEL_Pos);
    NRF_ADC->EVENTS_END = 0;
    NRF_ADC->ENABLE     = ADC_ENABLE_ENABLE_Enabled;

    // Enable ADC interrupt
    err_code = sd_nvic_ClearPendingIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_LOW);
    APP_ERROR_CHECK(err_code);

    err_code = sd_nvic_EnableIRQ(ADC_IRQn);
    APP_ERROR_CHECK(err_code);

    // Stop any running conversions and start task.
    NRF_ADC->EVENTS_END  = 0;    
    NRF_ADC->TASKS_START = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function to be executed when the battery timer expires.
 *
 * @details This function checks if device is connected, and if so starts updating of battery level.
 *
 * @param   context General purpose pointer.
 *
 * @return  Void.
 */

static void battery_level_meas_timeout_handler(void* context) 
{
    if(!ble_is_device_connected())
    {
        return;
    }
    ble_battery_start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for handling the battery service events.
 *
 * @param   bas Contains various status information for the service. 
 * @param   evt Battery Service event.
 *
 * @return  Void.
 */

static void on_battery_service_evt(ble_bas_t * bas, ble_bas_evt_t *evt) 
{
  uint32_t err_code;
  
  switch (evt->evt_type)
  {
      case BLE_BAS_EVT_NOTIFICATION_ENABLED:
          // Start battery timer
          err_code = app_timer_start(battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
          APP_ERROR_CHECK(err_code);
          break;

      case BLE_BAS_EVT_NOTIFICATION_DISABLED:
				  // Stop battery timer
          err_code = app_timer_stop(battery_timer_id);
          APP_ERROR_CHECK(err_code);
          break;

      default:
          break;
  }
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for events in the Connection Parameters Module which
 *          are passed to the application.
 *
 * @param   p_evt   Event received from the Connection Parameters Module.
 *
 * @return  Void.
 */

static void on_conn_params_evt(ble_conn_params_evt_t * p_evt) 
{ 
  uint32_t err_code;

  if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
  {
      err_code = sd_ble_gap_disconnect(conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
      APP_ERROR_CHECK(err_code);
  }
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for handling a Connection Parameters error.
 *
 * @param   nrf_error   Error code containing information about what went wrong.
 *
 * @return  Void.
 */

static void conn_params_error_handler(uint32_t nrf_error) 
{
    APP_ERROR_HANDLER(nrf_error); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for handling a Connection Parameters error.
 *
 * @param   nrf_error   Error code containing information about what went wrong.
 *
 * @return  Void.
 */

bool ble_disconnect(void)
{
    if(conn_handle != BLE_CONN_HANDLE_INVALID)  
    {
        return (sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION) == NRF_SUCCESS) ? true : false;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for handling the BLE level events.
 *
 * @param   p_ble_evt Common BLE Event type, wrapping the module specific event reports.
 *
 * @return  Void.
 */

static void on_ble_evt(ble_evt_t* p_ble_evt) 
{
    uint32_t err_code = NRF_SUCCESS;
    
    switch (p_ble_evt->header.evt_id) 
    {
			  // Request to provide security parameters.
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST: 
            err_code = sd_ble_gap_sec_params_reply(conn_handle, BLE_GAP_SEC_STATUS_SUCCESS, &sec_params); 
            break;
        
				// A persistent system attribute access is pending, awaiting a sd_ble_gatts_sys_attr_set().
        case BLE_GATTS_EVT_SYS_ATTR_MISSING: 
            err_code = sd_ble_gatts_sys_attr_set(conn_handle, NULL, 0);
            break;

				// GAP BLE Event base. Connection established.
        case BLE_GAP_EVT_CONNECTED: 
            conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            if (server_definition && server_definition->connection_callback) 
            {
                server_definition->connection_callback();
            }
            break;

				// Disconnected from peer.
        case BLE_GAP_EVT_DISCONNECTED: 
            if (USE_BONDMGR) 
            {
							  // Check whether it is necessary to delete the bonded central database from flash.
                if(clear_bondmngr_flag == true) 
                {
                    clear_bondmngr_flag = false;
                    err_code = ble_bondmngr_bonded_centrals_delete();
                }
								// Store the bonded centrals data into flash memory.
                else 
                {
                    err_code = ble_bondmngr_bonded_centrals_store();
                }
            }     
        
					// Set conn_handle mark to invalid.	
          conn_handle = BLE_CONN_HANDLE_INVALID;
						
          if (server_definition && server_definition->disconnection_callback) 
          {
              server_definition->disconnection_callback();
          }
          break;
        
        // Request to provide an authentication key.
        case BLE_GAP_EVT_AUTH_KEY_REQUEST: 
            sd_ble_gap_auth_key_reply(p_ble_evt->evt.gap_evt.conn_handle,BLE_GAP_AUTH_KEY_TYPE_PASSKEY, server_definition->passkey);
            break;

				// Timeout expired. 
        case BLE_GAP_EVT_TIMEOUT: 
            if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISEMENT) 
            {           
                if (server_definition && server_definition->advertising_timeout_callback) 
                {
                    server_definition->advertising_timeout_callback();
                }
            }
            break;

				// Write operation performed.
        case BLE_GATTS_EVT_WRITE: 
            if (server_definition && server_definition->write_raw_callback)
            {
                server_definition->write_raw_callback(&p_ble_evt->evt.gatts_evt.params.write);
            }
          break;
            
        default:
            break;
    }

    APP_ERROR_CHECK(err_code);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function to be called for each received BLE event.
 *
 * @param   p_ble_evt Common BLE Event type.
 *
 * @return  Void.
 */

static void ble_evt_dispatch(ble_evt_t* p_ble_evt)
{
    if (USE_BONDMGR) 
    {
        ble_bondmngr_on_ble_evt(p_ble_evt);
    }
    ble_conn_params_on_ble_evt(p_ble_evt);
    if (bas_enabled) 
    {
        ble_bas_on_ble_evt(&battery_service, p_ble_evt);
    }
    on_ble_evt(p_ble_evt);
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Config LED_PIN to be output, and set to off state.
 *
 * @return  true.
 */

static bool leds_init(void) 
{ 
    gpio_set_pin_digital_output(LED_PIN, PIN_DRIVE_S0S1);
    gpio_write(LED_PIN, false); 
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Create app timer.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

static bool timers_init(void) 
{
    uint32_t err_code;

    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, USE_SCHEDULER);

    // Create app tick timer.
    err_code = app_timer_create(&tick_timer_id, APP_TIMER_MODE_REPEATED, my_app_tick_handler);
	  
	  // Check error code.
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for initializing the Connection Parameters module.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

static bool conn_params_init(void) 
{ 
    ble_conn_params_init_t cp_init = 
    {
        NULL,                           //p_conn_params
        FIRST_CONN_PARAMS_UPDATE_DELAY, //first_conn_params_update_delay
        NEXT_CONN_PARAMS_UPDATE_DELAY,  //next_conn_params_update_delay
        MAX_CONN_PARAMS_UPDATE_COUNT,   //max_conn_params_update_count
        BLE_GATT_HANDLE_INVALID,        //start_on_notify_cccd_handle
        false,                          //disconnect_on_fail
        on_conn_params_evt,             //evt_handler
        conn_params_error_handler       //error_handler
    };

    uint32_t err_code = ble_conn_params_init(&cp_init);
		
		// Check error code.
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for initializing the Bond Manager.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

static bool bond_manager_init(void) 
{
    uint32_t err_code;
    
    if (USE_BONDMGR) 
    {
        ble_bondmngr_init_t bond_init_data = 
        {
          FLASH_PAGE_BOND,            //flash_page_num_bond
          FLASH_PAGE_SYS_ATTR,        //flash_page_sys_attr
          false,                      //binds_delete
          NULL,                       //evt_handler
          ble_error                   //error_handler
        };
        
        err_code = ble_bondmngr_init(&bond_init_data);
				
				// Check error code.
        if (err_code != NRF_SUCCESS) 
        {
            last_error = err_code;
            return false;
        }
    }
    return true;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function sets flag which used during disconnection process to clear bonded central database from flash.
 *
 * @return  Void.
 */

void ble_clear_bondmngr_request(void) 
{
    clear_bondmngr_flag = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param   sys_evt   System stack event.
 */

static void sys_evt_dispatch(uint32_t sys_evt) 
{ 
    pstorage_sys_event_handler(sys_evt);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for dispatching a system event to interested modules.
 *
 * @return   true
 */

bool ble_stack_init(void) 
{ 
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, false);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
  
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for initializing the event scheduler.
 *
 * @return  true
 */

static bool scheduler_init(void) 
{ 
    if (USE_SCHEDULER) 
    {
        APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for initializing GAP security parameters.
 *
 * @param   mitm_flag  Man In The Middle protection required.
 * @param   io_caps    IO capabilities.
 *
 * @return  void
 */

static void sec_params_init(uint8_t mitm_flag, uint8_t io_caps)
{
    sec_params.timeout = SEC_PARAM_TIMEOUT;
    sec_params.bond = SEC_PARAM_BOND;
    sec_params.mitm = mitm_flag;
    sec_params.io_caps = io_caps;
    sec_params.oob = SEC_PARAM_OOB;
    sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function configures GAP parameters.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

static bool gap_params_init(void) 
{
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&sec_mode);

	  // Set GAP device name.
    uint32_t err_code = sd_ble_gap_device_name_set(&sec_mode,server_definition->name,strlen((const char *)server_definition->name));
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }

		// Set GAP Appearance.
    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_TAG);
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }

    ble_gap_conn_params_t gap_conn_params = 
    {
        MIN_CONNECTION_INTERVAL, //min_conn_interval
        MAX_CONNECTION_INTERVAL, //max_conn_interval
        SLAVE_LATENCY,           //slave_latency
        SUPERVISION_TIMEOUT,     //conn_sup_timeout
    };
    
		// Set GAP Peripheral Preferred Connection Parameters.
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for initializing the advertising functionality.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_init_advertising(void) 
{  
    static uint8_t flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    // Build and set advertising data
    static ble_advdata_t advdata;
    advdata.name_type = BLE_ADVDATA_FULL_NAME;
    advdata.short_name_len = 0;
    advdata.include_appearance = true;
    advdata.flags.size = 1;
    advdata.flags.p_data = &flags;
    advdata.p_tx_power_level = 0;
    advdata.uuids_more_available.uuid_cnt = 0;
    advdata.uuids_more_available.p_uuids = NULL;
    advdata.uuids_complete.uuid_cnt = adv_uuid_count;
    advdata.uuids_complete.p_uuids = adv_uuids;
    advdata.uuids_solicited.uuid_cnt = 0;
    advdata.uuids_solicited.p_uuids =  NULL;
    advdata.p_slave_conn_int = NULL;
    advdata.p_manuf_specific_data = NULL;
    advdata.p_service_data_array = NULL;
    advdata.service_data_count = 0;

    uint32_t err_code = ble_advdata_set(&advdata, NULL);
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for initializing BLE server modules.
 *
 * @param   definition            Callbacks and properties data related to BLE server.
 * @param   pstorage_driver_init  Pointer to function which initialize and configure pstorage, and registers characteristic values to corresponding blocks in persistent memory.
 * @param   mitm_req_flag         Flag which indicates is there Man In The Middle protection required.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_init_server(const ble_server_definition_t* definition, pstorage_driver_init_t pstorage_driver_init, bool * mitm_req_flag) 
{  
    if (!definition)
		{ 
		    return false;
		}
		
    if (!mitm_req_flag)
		{
		    return false;
		}
    
    server_definition = definition;
    
    //Init modules
    if (!leds_init())             return false;
    if (!timers_init())           return false;
    if (!ble_stack_init())        return false;
    if (!pstorage_driver_init())  return false;
    if (!bond_manager_init())     return false;
    if (!scheduler_init())        return false;
    if (!gap_params_init())       return false;
    if (!onboard_init())          return false;
    if (!led_control_init())      return false;
  
		// If sensor is in onboard mode starts blinking led diode.
    if(onboard_get_mode() == ONBOARD_MODE_ACTIVE)
    {
        if (!led_control_start_config()) return false; 
    }
    
		// If sensor is in onboard mode or mitm_req_flag is false set Just-Works associate mode.
    if(
        (onboard_get_mode() == ONBOARD_MODE_ACTIVE) ||
        (*mitm_req_flag == false) 
      )
    {   
        sec_params_init(0, BLE_GAP_IO_CAPS_NONE);
    }
		// Set Man In The Middle protection, with Keyboard-Only capabilities.
    else
    {
        sec_params_init(1, BLE_GAP_IO_CAPS_KEYBOARD_ONLY);
    }
    
    return true;  
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for starting BLE server.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_start_server(void) 
{ 
    if (!conn_params_init()) 
    {
		    return false;
	  }
    return true;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Main thread loop.
 *
 * @return  Void.
 */

void ble_run(void) 
{     
    while (1) 
    {
        if (USE_SCHEDULER) 
        {
            app_sched_execute();
        }
        
        pstorage_driver_run();
        
        if(server_definition->main_thread_callback != NULL)
        {
            server_definition->main_thread_callback();
        }
        
        uint32_t err_code = sd_app_evt_wait();
        if (err_code != NRF_SUCCESS) 
        {
            last_error = err_code;
            return;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Adding Device Information Service
 *
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_add_device_information_service(void) 
{
    uint32_t       err_code;
    ble_dis_init_t dis_init;
  
    memset(&dis_init, 0, sizeof(dis_init));

	  // Set manufacturer name, and hardware/firmware revision.
    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)MANUFACTURER_NAME);
    ble_srv_ascii_to_utf8(&dis_init.hw_rev_str,        (char *)HARDWARE_REVISION);
    ble_srv_ascii_to_utf8(&dis_init.fw_rev_str,        (char *)FIRMWARE_REVISION);

	  // Set attribute security requirement.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

	  // Initialize Device Information Service.
    err_code = ble_dis_init(&dis_init);
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }
    
    adv_uuids[adv_uuid_count].type = BLE_UUID_TYPE_BLE;
    adv_uuids[adv_uuid_count].uuid = BLE_UUID_DEVICE_INFORMATION_SERVICE;
    adv_uuid_count++;
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Adding Battery Service
 *
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_add_bat_service(void) 
{
  
    if (bas_enabled) 
    {
        return true;
    }
		
    if (adv_uuid_count >= MAX_ADV_UUIDS) 
    {
        last_error = BLE_ERR_ADV_UUIDS_FULL;
        return false;
    }
    
		// Set Battery Service init structure.
    ble_bas_init_t bas_init_obj = 
    {
        on_battery_service_evt,     //evt_handler
        true,                       //support_notification
        NULL,                       //p_report_ref
        255,                        //initial batt level = invalid
        {0,0,0},                    //battery_level_char_attr_md
        {0,0}                       //battery_level_report_read_perm
    };

		// Set attribute security requirement.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init_obj.battery_level_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init_obj.battery_level_report_read_perm);

		// Initialize Battery Service.
    uint32_t err_code = ble_bas_init(&battery_service, &bas_init_obj);
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }

    // Create battery timer.
    err_code = app_timer_create(&battery_timer_id, APP_TIMER_MODE_REPEATED, battery_level_meas_timeout_handler);
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }

		// Start battery timer.
    err_code = app_timer_start(battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    adv_uuids[adv_uuid_count].type = BLE_UUID_TYPE_BLE;
    adv_uuids[adv_uuid_count].uuid = BLE_UUID_BATTERY_SERVICE;
    adv_uuid_count++;

    bas_enabled = true;
		
    // Start updating value of battery level.
		ble_battery_start();
		
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Adding user service.
 *
 * @param   short_uuid 16-bit UUID value
 * @param   long_uuid  16-octet (128-bit) little endian Vendor Specific UUID
 * @param   flags      Not-used for now 
 * @param   info       Service properties.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_add_service(const uint16_t short_uuid, const uint8_t* long_uuid, const uint16_t flags, ble_service_info_t* info) 
{
    uint32_t err_code;
    
    if (!info) 
    {
        last_error = BLE_ERR_INVALID_PARAMETER;
        return false;
    }
    
    if (adv_uuid_count >= MAX_ADV_UUIDS) 
    {
        last_error = BLE_ERR_ADV_UUIDS_FULL;
        return false;
    }

    ble_uuid_t* ble_uuid = &(adv_uuids[adv_uuid_count]);
    adv_uuid_count++;
    
		// Check if long_uuid is valid.
    if (long_uuid != NULL) 
    {
			  // Add a Vendor Specific UUID.
        err_code = sd_ble_uuid_vs_add((ble_uuid128_t const *)(long_uuid), &(ble_uuid->type));
        if (err_code != NRF_SUCCESS) 
        {
            last_error = err_code;
            return false;
        }
    } 
    else 
    {
        ble_uuid->type = BLE_UUID_TYPE_BLE;
    }
    ble_uuid->uuid = short_uuid;

		// Add a service declaration to the local server ATT table.
    uint16_t service_handle;
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, ble_uuid, &service_handle);
    if (err_code) 
    {
        last_error = err_code;
        return false;
    }

		// Set service properties. 
    info->short_uuid = ble_uuid->uuid;
    info->uuid_type = ble_uuid->type;
    info->service_handle = service_handle;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_add_characteristic(const ble_service_info_t* service,
                            const uint16_t char_uuid,
                            const uint16_t flags,
                            const uint8_t* user_desc,
                            const uint8_t* init_value,
                            const uint16_t data_len,
                            ble_characteristic_info_t* info) 
{
                              
  
    if ((!service) || (!info)) 
    {
        last_error = BLE_ERR_INVALID_PARAMETER;
        return false;
    }

    ble_gatts_attr_md_t cccd_md = 
    {
        {1,1},                      //read_perm (security mode, level), BT Core Spec, Vol. 3, Part C 10.2 
        {1,1},                      //write_perm (security mode, level)
        false,                      //vlen (variable len). false: fixed length
        BLE_GATTS_VLOC_STACK,       //vloc (value location) Stack: Value is stored in BT stack
        false,                      //rd_auth (read authorization will be requested by app each read command)
        false                       //wr_auth (write authorization will be requested by app each write request)
    };

    //Characteristic metadata 
    ble_gatts_char_md_t char_md = 
    {
        {0,0,0,0,0,0,0},    //char_props (broadcast,read,write_wo_resp,write,notify,indicate,auth_signed_wr)
        {0,0},              //char_ext_props (reliable_wr,wr_aux)
        NULL,               //p_char_user_desc
        0,                  //char_user_desc_max_size
        0,                  //char_user_desc_size
        NULL,               //p_char_pf
        NULL,               //p_user_desc_md
        NULL,               //p_cccd_md
        NULL                //p_sccd_md
    };
    
    //Attribute metadata 
    ble_gatts_attr_md_t attr_md = 
    {
        {0,0},                      //read_perm (security mode, level), BT Core Spec, Vol. 3, Part C 10.2 
        {0,0},                      //write_perm (security mode, level)
        false,                      //vlen (variable len). false: fixed length
        BLE_GATTS_VLOC_STACK,       //vloc (value location) Stack: Value is stored in BT stack
        false,                      //rd_auth (read authorization will be requested by app each read command)
        false                       //wr_auth (write authorization will be requested by app each write request)
    };
    
    
    // Add User Description Declaration
    if(user_desc != NULL) 
    {
        char_md.p_char_user_desc = (uint8_t*)user_desc;
        char_md.char_user_desc_max_size = strlen((const char*)user_desc);
        char_md.char_user_desc_size = char_md.char_user_desc_max_size;
    }
    
    if (flags & BLE_CHARACTERISTIC_BROADCAST) 
    {
        char_md.char_props.broadcast = 1;
    }
    
		// Check if read permission should be enabled.
    if (flags & BLE_CHARACTERISTIC_CAN_READ) 
    {
        char_md.char_props.read = 1;
			  // Check if there is encrypted link required, MITM protection necessary.
        if (flags & BLE_CHARACTERISTIC_READ_ENC_REQUIRE) 
        {
            BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(&attr_md.read_perm);
        }
				// Check if there is encrypted link required, MITM protection not necessary.
        else if (flags & BLE_CHARACTERISTIC_READ_ENC_REQUIRE_NO_MITM)
        {
            BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
        }
        else 
        {
            BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
        }
    }
    
    if (flags & BLE_CHARACTERISTIC_CAN_WRITE_WO_RESPONSE) 
    {
        char_md.char_props.write_wo_resp = 1;
        if (flags & BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE) 
        {
            BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(&attr_md.write_perm);
        }
        else if (flags & BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE_NO_MITM)
        {
            BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);
        }
        else 
        {
            BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
        }
    }
    
    if (flags & BLE_CHARACTERISTIC_CAN_WRITE) 
    {
        char_md.char_props.write = 1;
        if (flags & BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE) 
        {
            BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(&attr_md.write_perm);
        }
        else if (flags & BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE_NO_MITM)
        {
            BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);
        }
        else 
        {
            BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
        }
    }
    
    if (flags & BLE_CHARACTERISTIC_CAN_AUTH_SIGNED_WRITE) 
    {
        char_md.char_props.auth_signed_wr = 1;
    }
    
    if (flags & BLE_CHARACTERISTIC_CAN_RELIABLE_WRITE) 
    {
        char_md.char_ext_props.reliable_wr = 1;
    }
    
    if (flags & BLE_CHARACTERISTIC_CAN_WRITE_AUX) 
    {
        char_md.char_ext_props.wr_aux = 1;
    }
    
    if (flags & BLE_CHARACTERISTIC_CAN_NOTIFY) 
    {
        char_md.char_props.notify = 1;
    }
    
    if (flags & BLE_CHARACTERISTIC_CAN_INDICATE) 
    {
        char_md.char_props.indicate = 1;
    }
    
    // Do we need cccd ?
    if((flags & BLE_CHARACTERISTIC_CAN_NOTIFY) || (flags & BLE_CHARACTERISTIC_CAN_INDICATE))
    {
        char_md.p_cccd_md = &cccd_md;
      
        if((flags & BLE_CHARACTERISTIC_READ_ENC_REQUIRE) || (flags & BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE))
        {
            BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(&cccd_md.write_perm);
        }
        else if((flags & BLE_CHARACTERISTIC_READ_ENC_REQUIRE_NO_MITM) || (flags & BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE_NO_MITM))
        {
            BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&cccd_md.write_perm);
        }
    }

    //Attribute UUID        
    ble_uuid_t ble_uuid;
    ble_uuid.type = service->uuid_type;
    ble_uuid.uuid = char_uuid;

    //Characteristic value attribute
    ble_gatts_attr_t    attr_char_value;

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = data_len;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = data_len;
    attr_char_value.p_value      = (uint8_t*)init_value;    

    ble_gatts_char_handles_t handles;
		
    //Add a characteristic declaration, a characteristic value declaration and characteristic descriptor declarations to the local server ATT table.
    uint32_t err_code = sd_ble_gatts_characteristic_add(service->service_handle, &char_md, &attr_char_value, &handles);
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }
		
		// Set characteristic properties. 
    info->value_handle     = handles.value_handle;
    info->user_desc_handle = handles.user_desc_handle;
    info->cccd_handle      = handles.cccd_handle;
    info->sccd_handle      = handles.sccd_handle;
    info->flags            = flags;
    info->state            = 0;
    return true;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Set the value of the attribute, and notify or indicate an attribute value (if notification/inidication enabled).
 *
 * @param   char_info  Characteristic properties.
 * @param   data       Buffer containing the desired attribute value
 * @param   len        Length in bytes to be written.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_update_characteristic_value(ble_characteristic_info_t* char_info, uint8_t* data, uint16_t len) 
{
    uint32_t err_code = sd_ble_gatts_value_set(char_info->value_handle,0, &len,data);     // Set the value of the attribute
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }
    
    // Check if sensor connected
    if (conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        bool notifying  = (char_info->state & BLE_CHARACTERISTIC_IS_NOTIFYING) != 0;     
        bool indicating = (char_info->state & BLE_CHARACTERISTIC_IS_INDICATING) != 0;    
      
			  // Check if notification or indication enabled.
        if (notifying || indicating) 
        {
            ble_gatts_hvx_params_t hvx_params = {
                char_info->value_handle,    //handle
                notifying ? BLE_GATT_HVX_NOTIFICATION : BLE_GATT_HVX_INDICATION,    //type BLE_GATT_HVX_TYPES
                0,                          //offset
                &len,                       //p_len
                data                        //p_data
            };
						// Send notification/indication.
            err_code = sd_ble_gatts_hvx(conn_handle, &hvx_params);
            if (err_code != NRF_SUCCESS) 
            {
                last_error = err_code;
                return false;
            }
        }
    }
    return true;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function to be called for starting advertising.
 *
 * @param   beacon_frequency  Adverdising interval in ms.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_start_advertising(uint32_t beacon_frequency) 
{
    static ble_gap_adv_params_t adv_params;

    adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;    // Connectable undirected.
    adv_params.p_peer_addr = NULL;
    adv_params.fp          = BLE_GAP_ADV_FP_ANY;          // Allow scan requests and connect requests from any device.
    adv_params.p_whitelist = NULL;
    adv_params.interval    = MSEC_TO_UNITS(beacon_frequency, UNIT_0_625_MS); // Advertising interval.
    adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;  // Advertising timeout.

    uint32_t err_code = sd_ble_gap_adv_start(&adv_params);
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }
    return true;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function to be called for stoping advertising.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_stop_advertising(void) 
{
    uint32_t err_code = sd_ble_gap_adv_stop();
    if (err_code != NRF_SUCCESS) 
    {
        last_error = err_code;
        return false;
    }
    return true;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Dispatches a raw write operation to a characteristic. Should be called within a raw write callback with all respective characteristics.
 *
 * @param   handle     Characteristic handle.
 * @param   offset     Offset for the write operation.
 * @param   len        Length of the incoming data.
 * @param   data       Incoming data.
 * @param   char_info  Characteristic properties. 
 *
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_dispatch_write_characteristic(const uint16_t handle, uint16_t offset, uint16_t len, uint8_t* data, ble_characteristic_info_t* char_info) 
{
    if (!char_info) 
    { 
      return false; 
    }
    
		// Write to characteristic value.
    if (handle == char_info->value_handle) 
    {
        if (server_definition->write_characteristic_callback) 
        {
            server_definition->write_characteristic_callback(char_info, offset, len, data);
        }
        return true;
    } 
		// Write to characteristic cccd.
    else if (handle == char_info->cccd_handle) 
    {
        if (len == 2) 
        {
            bool not_was_on = char_info->state & BLE_CHARACTERISTIC_IS_NOTIFYING;
            bool ind_was_on = char_info->state & BLE_CHARACTERISTIC_IS_INDICATING;
            bool not_is_on  = ble_srv_is_notification_enabled(data);
            bool ind_is_on  = ble_srv_is_indication_enabled(data);
            
					  // Check whether it is necessary to change state of characteristic.
					
            if (not_is_on && !not_was_on) 
            {
                char_info->state |= BLE_CHARACTERISTIC_IS_NOTIFYING;
                if (server_definition->subscription_callback) 
                {
                    server_definition->subscription_callback(char_info, BLE_NOTIFICATION_START);
                }
            }
            
            if (!not_is_on && not_was_on) 
            {
                char_info->state &= ~BLE_CHARACTERISTIC_IS_NOTIFYING;
                if (server_definition->subscription_callback) 
                {
                    server_definition->subscription_callback(char_info, BLE_NOTIFICATION_END);
                }
            }
            if (ind_is_on && !ind_was_on) 
            {
                char_info->state |= BLE_CHARACTERISTIC_IS_INDICATING;
                if (server_definition->subscription_callback) 
                {
                    server_definition->subscription_callback(char_info, BLE_INDICATION_START);
                }
            }
            if (!ind_is_on && ind_was_on) 
            {
                char_info->state &= ~BLE_CHARACTERISTIC_IS_INDICATING;
                if (server_definition->subscription_callback) 
                {
                    server_definition->subscription_callback(char_info, BLE_INDICATION_END);
                }
            }
        }
        return true;
    }
    return false; 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Clear characteristic state properties.
 *
 * @param   char_info  Pointer to characteristic properties.
 *
 * @return  Void.
 */

void ble_reset_characteristic(ble_characteristic_info_t* char_info) 
{ 
    if (char_info) 
    {     
        char_info->state = 0; 
    } 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Puts the chip in System OFF mode.
 *
 * @return  Void.
 */

void ble_shutdown(void) 
{ 
    sd_power_system_off(); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function return value of last_error variable.
 *
 * @return  Value of last_error variable.
 */

uint32_t ble_get_error(void) 
{
    return last_error;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Enable external interrupt.
 *
 * @return  Void.
 */

void ble_enable_input_interrupt(void)
{
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    NVIC_SetPriority(GPIOTE_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(GPIOTE_IRQn);   
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Disable external interrupt.
 *
 * @return  Void.
 */

void ble_disable_input_interrupt(void)
{
    NVIC_DisableIRQ(GPIOTE_IRQn);    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Set callback for external interrupt.
 *
 * @param   pin_no     Pin number
 * @param   sense      Selecting the pin to sense high or low level on the pin input.
 * @param   pull_mode  Selecting the pin to be pulled down or up.
 * @param   callback   Callback to be called when external interrupt occurred.
 *
 * @return  Void.
 */

void ble_set_input_callback(uint8_t pin_no, nrf_gpio_pin_sense_t sense, nrf_gpio_pin_pull_t pull_mode, ble_input_callback_t callback) 
{
    nrf_gpio_cfg_sense_input(pin_no, pull_mode, sense);
    
    my_input_callback = callback;
  
    // Set the GPIOTE PORT event as interrupt source.
    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for handling the GPIOTE interrupt.
 *
 * @return  Void.
 */

void GPIOTE_IRQHandler_ble(void) 
{
    my_input_callback();
    GPIOTE_HW->EVENTS_PORT = 0;
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function to be called to set application timer interval and callback
 *
 * @param   callback   Callback to be called when timer timeout occurred.
 * @param   interval   Number of ms to timeout event.
 * @param   context    General purpose pointer.
 *
 * @return  false in case that error is occurred, otherwise true.
 */

bool ble_set_app_tick(ble_app_tick_callback_t callback, uint32_t interval, void* context) 
{ 
    bool start = (callback && interval);
    uint32_t err_code = NRF_SUCCESS;
  
	  // Do we need to start timer?
    if (start) 
    {
        app_tick_callback = callback;
        app_timer_stop(tick_timer_id);
        uint32_t err_code = app_timer_start(tick_timer_id, APP_TIMER_TICKS(interval, APP_TIMER_PRESCALER), context);
        if (err_code != NRF_SUCCESS) 
        {
            last_error = err_code;
            return false;
        }
    } 
		// Stop timer.
    else 
    {
      app_tick_callback = NULL;
      app_timer_stop(tick_timer_id);
      if (err_code != NRF_SUCCESS) 
      {
          last_error = err_code;
          return false;
      }
    }
    return true;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function call application tick callback.
 *
 * @param   context    General purpose pointer.
 *
 * @return  Void.
 */

static void my_app_tick_handler(void* context) 
{  
    if (app_tick_callback) 
    {
        app_tick_callback(context);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function returns absolute value of float variable.
 *
 * @param   fl Float parameter.
 *
 * @return  Absolute value of fl param.
 */

float f_abs(float fl)
{
    if(fl < 0)
    {
        return -fl;
    }
    
    return fl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**@brief    This function compares float type new_value and old_value variables by used threshold params.
 *
 * @param    threshold  Record which contains hi, low and sbl value of threshold.
 * @param    old_value  Old value for comparing.
 * @param    new_value  New value for comparing.
 *
 * @retval   true if threshold exceeded, false otherwise.
 */

bool check_threshold_fl(threshold_float_t * threshold, float old_value, float new_value)
{ 
    if( 
        ( f_abs(old_value - new_value) >= threshold->sbl) ||
        (new_value < threshold->low) ||
        (new_value > threshold->high)
      )
  {
      return true;
  }
  
  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief    This function compares int_16 type new_value and old_value variables by used threshold params.
 *
 * @param    threshold  Record which contains hi, low and sbl value of threshold.
 * @param    old_value  Old value for comparing.
 * @param    new_value  New value for comparing.
 *
 * @retval   true if threshold exceeded, false otherwise.
 */

bool check_threshold_int(threshold_int16_t * threshold, int16_t old_value, int16_t new_value)
{ 
    if( 
        ( abs(old_value - new_value) >= threshold->sbl) ||
        (new_value < threshold->low) ||
        (new_value > threshold->high)
      )
  {
      return true;
  }
  
  return false;
}

