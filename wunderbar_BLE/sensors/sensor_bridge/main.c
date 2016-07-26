
/** @file   main.c
 *  @brief  This file contains the source code for firmware of BRIDGE sensor of Wunderbar Board.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */
 
/* -- Includes -- */

#include <stdint.h>
#include <string.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "ble.h"
#include "ble_driver.h"
#include "led_control.h"
#include "onboard.h"
#include "pstorage_driver.h"
#include "crc16.h"
#include "app_uart.h"

#define UART_RX_PIN  15    /**< UART RX pin number. */
#define UART_TX_PIN  16    /**< UART TX pin number. */
#define HWFC         false /**< UART hardware flow control. */

#define RELAY_PIN    16		 /**< P0.16 (pin 4 of grove connector). */

#define NUMBER_OF_RESEND 3 /**< Max number of resend tries. */

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

const uint8_t  long_service_uuid[]          = {0x78,0x6d,0x2e,0x96,0x6f,0xd1,0x42,0x2f,0x8e,0x87,0x15,0x93,0xff,0xff,0x5a,0x09}; /**< 16-octet (128-bit) UUID. */

const uint16_t short_service_relayr_uuid                  = SHORT_SERVICE_RELAYR_UUID;                       /**< 16-bit UUID value for relayr service. */
const uint16_t short_service_relayr_open_comm_uuid        = SHORT_SERVICE_RELAYR_OPEN_COMM_UUID;             /**< 16-bit UUID value for relayr service which not require encryption. */
const uint16_t short_service_config_uuid                  = SHORT_SERVICE_CONFIG_UUID;                       /**< 16-bit UUID value for onboarding service. */

const uint16_t characteristic_sensorID_uuid               = CHARACTERISTIC_SENSOR_ID_UUID;                   /**< 16-bit UUID value for sensor ID char. */
const uint16_t characteristic_sensorBeaconFrequency_uuid  = CHARACTERISTIC_SENSOR_BEACON_FREQUENCY_UUID;     /**< 16-bit UUID value for beacon frequency char. */
const uint16_t characteristic_sensorLedState_uuid         = CHARACTERISTIC_SENSOR_LED_STATE_UUID;            /**< 16-bit UUID value for led state char. */
const uint16_t characteristic_sensorConfig_uuid           = CHARACTERISTIC_SENSOR_CONFIG_UUID;               /**< 16-bit UUID value for config char. */
const uint16_t characteristic_sensorData_up_uuid          = CHARACTERISTIC_SENSOR_DATA_R_UUID;               /**< 16-bit UUID value for data up char. */
const uint16_t characteristic_sensorData_down_uuid        = CHARACTERISTIC_SENSOR_DATA_W_UUID;               /**< 16-bit UUID value for data down char. */
const uint16_t characteristic_sensorPasskey_uuid          = CHARACTERISTIC_SENSOR_PASSKEY_UUID;              /**< 16-bit UUID value for sensor passkey char. */
const uint16_t characteristic_sensorMitmReqFlag_uuid      = CHARACTERISTIC_SENSOR_MITM_REQ_FLAG_UUID;        /**< 16-bit UUID value for working encryption mode char. */ 

/**@brief  Records related to characteristics. */
ble_characteristic_info_t characteristic_sensorID_info;
ble_characteristic_info_t characteristic_sensorBeaconFrequency_info;
ble_characteristic_info_t characteristic_sensorLedState_info;
ble_characteristic_info_t characteristic_sensorConfig_info;
ble_characteristic_info_t characteristic_sensorData_up_info;
ble_characteristic_info_t characteristic_sensorData_down_info;
ble_characteristic_info_t characteristic_sensorPasskey_info;
ble_characteristic_info_t characteristic_sensorMitmReqFlag_info;

ble_server_definition_t server_def = 
{
    my_connection_callback,            /**< Connection_callback. */
    my_disconnection_callback,         /**< Disconnection_callback. */
    my_advertising_timeout_callback,   /**< Advertising_timeout_callback. */
    my_raw_write_callback,             /**< Write_raw_callback. */
    my_characteristic_write_callback,  /**< Write_characteristic_callback. */
    my_main_thread_callback,           /**< Main thread callback. */
    NULL,                              /**< Subscription_callback. */
    NULL,                              /**< Passkey. */
    NULL                               /**< Device name. */
};
ble_service_info_t service_info;

sensor_bridge_t sensor_bridge;
uint16_t        short_service_uuid;

/**@brief  Default values for corresponding characteristics. These values are used if corresponding block of persistent storage is empty. */
const uint8_t                DEFAULT_DEVICE_NAME[BLE_DEVNAME_MAX_LEN+1] = DEVICE_NAME_BRIDGE;
const sensorID_t             DEFAULT_SENSOR_ID = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                                  0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
const beaconFrequency_t      DEFAULT_SENSOR_BEACON_FREQUENCY            = ADV_INTERVAL_MS;
const led_state_t            DEFAULT_SENSOR_LED_STATE                   = false;
const sensor_bridge_config_t DEFAULT_SENSOR_CONFIG                      = {115200};
const passkey_t              DEFAULT_SENSOR_PASSKEY                     = "000000";
const security_level_t       DEFAULT_MITM_REQ_FLAG                      = true;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**@brief  List of bridge commands. */
typedef enum
{
    BRIDGE_COMM_WRITE_UP_CHANNEL  = 0x01,                  /**< Write "Up-Channel" payload. */
    BRIDGE_COMM_READ_UP_CHANNEL   = 0x02,                  /**< Read "Up-Channel" payload. */
    BRIDGE_COMM_READ_DOWN_CHANNEL = 0x03,                  /**< Read "Down-Channel" payload. */
    BRIDGE_COMM_ACK               = 0x04,                  /**< ACK (Received OK). */
    BRIDGE_COMM_NACK              = 0x05,                  /**< NACK (Error). */
    BRIDGE_COMM_PING              = 0x06,                  /**< PING (Respond with an ACK). */
    BRIDGE_COMM_RCV_FROM_BLE      = 0x07,                  /**< Data received from Relayr cloud. */
    BRIDGE_COMM_NCONN             = 0x08,                  /**< Sensor is not connected. */
}
bridge_commands_t;

/**@brief  Bridge packet protocol record. */
typedef __packed struct
{
    uint8_t  command;                                      /**< Command form bridge_commands_t list. */
    uint8_t  payload_length;                               /**< Length of payload in bytes. */
    uint8_t  payload[BRIDGE_PAYLOAD_SIZE];                 /**< Payload data. */
    uint16_t crc16;                                        /**< Cyclic redundancy check. */
}
bridge_packet_t;

/**@brief  List of bridge states. */
typedef enum
{
    BRIDGE_STATE_COMMAND_WAIT,                             /**< Wait for next packet. */ 
    BRIDGE_STATE_LENGTH_WAIT,                              /**< Command received, wait for payload lenght. */
    BRIDGE_STATE_PAYLOAD_WAIT,                             /**< Length received, wait for all payload data. */
    BRIDGE_STATE_CRC16_LOW_WAIT,                           /**< Wait for low byte of CRC16. */
    BRIDGE_STATE_CRC16_HIGH_WAIT,                          /**< Wait for high byte of CRC16. */
    BRIDGE_STATE_ACK_WAIT,                                 /**< Wait for ACK. */
}
bridge_state_t;

/**@brief  Bridge Rx path related record. */
typedef __packed struct
{
    bridge_packet_t packet;                                /**< Bridge packet. */
    bridge_state_t  state;                                 /**< State of packet receiving. */
}
bridge_rx_t;

/**@brief  Bridge Tx path related record. */
typedef __packed struct
{
    bridge_packet_t packet;                                /**< Bridge packet. */
    uint8_t         resend_counter;                        /**< Current Number of resend tries. */
}
bridge_tx_t;

/**@brief  General Bridge record. */
typedef struct 
{
    bridge_rx_t  rx;                                       /**< Tx path field. */
    bridge_tx_t  tx;                                       /**< Rx path field. */
}
bridge_t;
bridge_t bridge;

bool     bridge_rcv_from_ble_mark = false;                 /**< Indicate received data through ble interface. */
uint16_t crc16_1;

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
/** @brief  This function converts desired baudrate to value which should writes in baudrate register.
 *
 *  @param  baud_rate      Desired baudrate.
 *  @param  baud_rate_reg  Converted value.
 *
 *  @return true if operation is successful, otherwise false.
 */

bool bridge_set_baud_rate_register(uint32_t baud_rate, uint32_t * baud_rate_reg)
{
    bool status = true;
  
    switch(baud_rate)
    {
        case 1200:   *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud1200;   break;
        case 2400:   *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud2400;   break;
        case 4800:   *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud4800;   break;
        case 9600:   *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud9600;   break;
        case 14400:  *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud14400;  break;
        case 19200:  *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud19200;  break;
        case 28800:  *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud28800;  break;
        case 38400:  *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud38400;  break;
        case 57600:  *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud57600;  break;
        case 76800:  *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud76800;  break;
        case 115200: *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud115200; break;
        case 230400: *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud230400; break;
        case 250000: *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud250000; break;
        case 460800: *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud460800; break;
        case 921600: *baud_rate_reg = UART_BAUDRATE_BAUDRATE_Baud921600; break;
        default : status = false;
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function creates pakcet to be send over uart.
 *
 *  @param  command Command form bridge_commands_t list.
 *  @param  len     Length of payload in bytes. 
 *  @param  data    Payload buffer.
 *
 *  @return true.
 */

bool bridge_create_tx_packet(uint8_t command, uint8_t len, uint8_t * data)
{
    bridge.tx.packet.command = command;
    bridge.tx.packet.payload_length = len;
    memcpy((uint8_t *)bridge.tx.packet.payload, data, len);
    bridge.tx.packet.crc16 = crc16_compute((uint8_t *)&bridge.tx.packet, bridge.tx.packet.payload_length + BRIDGE_HEDER_SIZE, NULL);  
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function sends over uart.
 *
 *  @return true if operation is successful, otherwise false.
 */

bool bridge_send_packet(void)
{
    uint8_t cnt;
    uint8_t * ptr;
  
    app_uart_put(bridge.tx.packet.command);
    app_uart_put(bridge.tx.packet.payload_length);
    
    ptr = (uint8_t *)&bridge.tx.packet.payload;
    for(cnt = 0; cnt < bridge.tx.packet.payload_length; cnt++)  
    {
        if(app_uart_put(ptr[cnt]) != NRF_SUCCESS)
        {
            return false;
        }   
    }
    app_uart_put((uint8_t)bridge.tx.packet.crc16);
    app_uart_put((uint8_t)(bridge.tx.packet.crc16 >> 8));
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function serves the received command.
 *
 *  @param  command Received command code (form bridge_commands_t list).
 *
 *  @return Void.
 */

void bridge_check_command_rcv(bridge_commands_t command)
{
    switch(command)
    {
        // PING (Respond with an ACK).			  
        case BRIDGE_COMM_PING:
        {
            app_uart_put(BRIDGE_COMM_ACK);
            bridge.rx.state = BRIDGE_STATE_COMMAND_WAIT;
            break;
        }
				
        // Read "Down-Channel" payload.
        case BRIDGE_COMM_READ_DOWN_CHANNEL:
        {
            bridge_create_tx_packet(BRIDGE_COMM_READ_DOWN_CHANNEL, sensor_bridge.data_down.payload_length, sensor_bridge.data_down.payload);
            bridge_send_packet();
            bridge.rx.state = BRIDGE_STATE_ACK_WAIT;
            break;
        }
        
				// Read "Up-Channel" payload.
        case BRIDGE_COMM_READ_UP_CHANNEL:
        {
            bridge_create_tx_packet(BRIDGE_COMM_READ_UP_CHANNEL, sensor_bridge.data_up.payload_length, sensor_bridge.data_up.payload);
            bridge_send_packet();
            bridge.rx.state = BRIDGE_STATE_ACK_WAIT;
            break;
        }
        
				// Write "Up-Channel" payload.
        case BRIDGE_COMM_WRITE_UP_CHANNEL:
        {
            bridge.rx.packet.command = command;
            bridge.rx.state = BRIDGE_STATE_LENGTH_WAIT;
            break;
        }
        
        default:
        {
            bridge.rx.state = BRIDGE_STATE_COMMAND_WAIT;
        }
    }

} 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function serves the uart events.
 *
 *  @param  p_app_uart_event Events from the UART module.
 *
 *  @return Void.
 */

void bridge_uart_event_handler(app_uart_evt_t *p_app_uart_event)
{
    uint8_t uart_rx;
  
	  // Check if UART data has been received.
    if(p_app_uart_event->evt_type == APP_UART_DATA_READY)
    {
			  // Getting a byte from the UART.
        if(app_uart_get(&uart_rx) != NRF_SUCCESS)
        {
            return;
        }
        
        switch(bridge.rx.state) 
        {
            static uint8_t   payload_count;
            static uint16_t  crc16;
            
					  // Start of next packet detected.
            case BRIDGE_STATE_COMMAND_WAIT:
            {
                bridge_check_command_rcv((bridge_commands_t)uart_rx);
                break;
            }
            
						// Length byte received.
            case BRIDGE_STATE_LENGTH_WAIT:
            {
                bridge.rx.packet.payload_length = uart_rx;
                payload_count = 0;
                bridge.rx.state = BRIDGE_STATE_PAYLOAD_WAIT;
                break;
            }
            
						// Reading payload.
            case BRIDGE_STATE_PAYLOAD_WAIT:
            {
                
                bridge.rx.packet.payload[payload_count] = uart_rx;
                payload_count++;
                if(payload_count >= bridge.rx.packet.payload_length)
                {
                    bridge.rx.state = BRIDGE_STATE_CRC16_LOW_WAIT;
                }
                
                break;
            }
            
						// Get first crc16 byte.
            case BRIDGE_STATE_CRC16_LOW_WAIT:
            {
                crc16 = uart_rx;
                bridge.rx.state = BRIDGE_STATE_CRC16_HIGH_WAIT;
                break;
            }
            
						// Get second crc16 byte.
            case BRIDGE_STATE_CRC16_HIGH_WAIT:
            {  
                
							  // If device is not connected of notification/indication is not enabled send BRIDGE_COMM_NCONN byte.
                if( !ble_is_device_connected() ||
                    (
                        ((characteristic_sensorData_up_info.state & BLE_CHARACTERISTIC_IS_NOTIFYING)  == 0)&&
                        ((characteristic_sensorData_up_info.state & BLE_CHARACTERISTIC_IS_INDICATING) == 0)
                    ) 
                  )
                {
                    app_uart_put(BRIDGE_COMM_NCONN);
                }  
                else
                {
                    crc16 += ((uint16_t)uart_rx << 8);
                    crc16_1 = crc16_compute((uint8_t *)&bridge.rx.packet, bridge.rx.packet.payload_length + BRIDGE_HEDER_SIZE, NULL);
									  // Compare received crc16 with calculated value.
                    if(
                        (crc16 == crc16_compute((uint8_t *)&bridge.rx.packet, bridge.rx.packet.payload_length + BRIDGE_HEDER_SIZE, NULL)) &&
                        (bridge.rx.packet.payload_length <= BRIDGE_PAYLOAD_SIZE)
                      )
                    {
                        // Send data over ble and ACK over uart.
											  memset((uint8_t*)&sensor_bridge.data_up, 0, sizeof(sensor_bridge.data_up)); 
                        sensor_bridge.data_up.payload_length = bridge.rx.packet.payload_length;
                        memcpy((uint8_t*)&sensor_bridge.data_up.payload, (uint8_t*)&bridge.rx.packet.payload, bridge.rx.packet.payload_length); 
                        ble_update_characteristic_value(&characteristic_sensorData_up_info, (uint8_t *)(&sensor_bridge.data_up), sizeof(sensor_bridge.data_up));
                        app_uart_put(BRIDGE_COMM_ACK);
                    }
										// Send BRIDGE_COMM_NACK byte.
                    else
                    {
                        app_uart_put(BRIDGE_COMM_NACK);
                    }
                }
                
								// Go to BRIDGE_STATE_COMMAND_WAIT state (wait to next packet).
                bridge.rx.state = BRIDGE_STATE_COMMAND_WAIT;
                break;
            }
            
            // ACK wait state. 
            case BRIDGE_STATE_ACK_WAIT:
            {
							  // If ACK received reset resend counter and go to BRIDGE_STATE_COMMAND_WAIT state.
                if(uart_rx == BRIDGE_COMM_ACK)
                {
                    bridge.tx.resend_counter = 0;
                    bridge.rx.state = BRIDGE_STATE_COMMAND_WAIT;
                }
                
								// If NACK received?
                else if(uart_rx == BRIDGE_COMM_NACK)
                {
									  // If there is need to do more resends?
                    if(bridge.tx.resend_counter < NUMBER_OF_RESEND)
                    {
											  // Resend last packet, and increment resend_counter.
                        bridge_send_packet();
                        bridge.tx.resend_counter++;
                    }
                    else
                    {
											  // Reset resend counter and go to BRIDGE_STATE_COMMAND_WAIT state.
                        bridge.tx.resend_counter = 0;
                        bridge.rx.state = BRIDGE_STATE_COMMAND_WAIT;
                    }
                }
								
								// Can be considered to have received the beginning of a new package.
                else
                {
                    bridge_check_command_rcv((bridge_commands_t)uart_rx);
                }
                
                break;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @brief  This function initialize uart module.
 *
 *  @return true if operation is successful, otherwise false.
 */

bool bridge_uart_init()
{
    uint32_t baud_rate_reg;
  
    bridge_set_baud_rate_register(sensor_bridge.config.baud_rate, &baud_rate_reg);
      
    const app_uart_comm_params_t  p_comm_params = 
    {
        UART_RX_PIN,
        UART_TX_PIN,
        0,
        0,   
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        baud_rate_reg
    };
    uint32_t error;
    
    APP_UART_FIFO_INIT(&p_comm_params,
												32,
												32,
												bridge_uart_event_handler,
												APP_IRQ_PRIORITY_LOW,
												error);
    
    if(error != NRF_SUCCESS)
    {
        return false;
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief   Function for initializing some globals
 *
 * @retval  true.
 */

bool comfort_init() 
{   
    server_def.passkey =  sensor_bridge.passkey;
    memcpy((uint8_t*)&server_def.name, (uint8_t*)DEFAULT_DEVICE_NAME, BLE_DEVNAME_MAX_LEN);
    sensor_bridge.led_state = DEFAULT_SENSOR_LED_STATE;
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
 * @return true if operation is successful, otherwise false.
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
    
    // Read SensorID from p_storage
    if(!init_global((uint8_t*)&sensor_bridge.sensorID, (uint8_t*)&DEFAULT_SENSOR_ID, sizeof(sensor_bridge.sensorID))) 
    {
        return false;
    }
    // Read SensorBeaconFrequency from p_storage
    if(!init_global((uint8_t*)&sensor_bridge.beaconFrequency, (uint8_t*)&DEFAULT_SENSOR_BEACON_FREQUENCY, sizeof(sensor_bridge.beaconFrequency))) 
    {
        return false;
    }
    // Read Config from p_storage
    if(!init_global((uint8_t*)&sensor_bridge.config, (uint8_t*)&DEFAULT_SENSOR_CONFIG, sizeof(sensor_bridge.config))) 
    {
        return false;
    }
    // Read Passkey from p_storage
    if(!init_global((uint8_t*)sensor_bridge.passkey, (uint8_t*)DEFAULT_SENSOR_PASSKEY, sizeof(sensor_bridge.passkey))) 
    {
        return false;
    }
    // Read mitm_req_flag from p_storage
    if(!init_global((uint8_t*)&sensor_bridge.mitm_req_flag, (uint8_t*)&DEFAULT_MITM_REQ_FLAG, sizeof(sensor_bridge.mitm_req_flag))) 
    {
        return false;
    }
    
    return true;
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
      ble_start_advertising(sensor_bridge.beaconFrequency);
      
			// If we in onboarding active mode?
      if(onboard_get_mode() == ONBOARD_MODE_ACTIVE)
      {
          onboard_on_disconnect();
      }
			// If we not in onboarding active mode?
			else if(onboard_get_mode() == ONBOARD_MODE_IDLE)
			{
					characteristic_sensorData_up_info.state = 0;
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
    ble_start_advertising(sensor_bridge.beaconFrequency);
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
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorLedState_info); 
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorConfig_info);
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorData_up_info);
    ble_dispatch_write_characteristic(evt_write->handle, evt_write->offset, evt_write->len, evt_write->data, &characteristic_sensorData_down_info); 
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
	  if ((char_info == &characteristic_sensorID_info) && (offset == 0) && (len == sizeof(sensor_bridge.sensorID))) 
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_bridge.sensorID, data, len); 
        pstorage_driver_request_store((uint8_t*)(&sensor_bridge.sensorID));
    }
		
		// Write to beacon frequency characteristic value?
    if ((char_info == &characteristic_sensorBeaconFrequency_info) && (offset == 0) && (len == sizeof(sensor_bridge.beaconFrequency))) 
    {
        beaconFrequency_t tmp_beaconFrequency;
        memcpy((uint8_t*)&tmp_beaconFrequency, data, len);    
        
			  // Check if new value is valid.
        if((tmp_beaconFrequency >= 20) && (tmp_beaconFrequency <= 10240))
        {
					  // Copy new data to local buffer, and store it to persistent memory.
            sensor_bridge.beaconFrequency = tmp_beaconFrequency;
            pstorage_driver_request_store((uint8_t*)(&sensor_bridge.beaconFrequency));
        }
    }
    
		// Write to led state characteristic value?
    if ((char_info == &characteristic_sensorLedState_info) && (offset == 0) && (len == sizeof(sensor_bridge.led_state))) 
    {
			  // Copy new data to local buffer, and update state of LED diode.
        memcpy((uint8_t*)&sensor_bridge.led_state, data, len);  
			  led_control_update_char(sensor_bridge.led_state, LED_TIMEOUT_CHAR_MS);
    }
    
		// Write to config characteristic value?
    if ((char_info == &characteristic_sensorConfig_info) && (offset == 0) && (len == sizeof(sensor_bridge.config))) 
    {
        uint32_t baud_rate_reg;
			
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_bridge.config, data, len); 
        pstorage_driver_request_store((uint8_t*)(&sensor_bridge.config));
        if(bridge_set_baud_rate_register(sensor_bridge.config.baud_rate, &baud_rate_reg) == true)
        {
            NRF_UART0->BAUDRATE = (baud_rate_reg << UART_BAUDRATE_BAUDRATE_Pos);  
        }
    } 
    
		// Write to data down characteristic value?
    if ((char_info == &characteristic_sensorData_down_info) && (offset == 0) && (len <= sizeof(sensor_bridge.data_down))) 
    {
        memcpy((uint8_t*)&sensor_bridge.data_down, data, len);
        //bridge_rcv_from_ble_mark = true;
        
			  // Update relay state.
			  if (sensor_bridge.data_down.payload[0] == 0) 
				{
					  gpio_write(RELAY_PIN, false);
				}
				else if (sensor_bridge.data_down.payload[0] == 1) 
				{
						gpio_write(RELAY_PIN, true);
				}
    }
    
		// Write to sensor passkey characteristic value?
    if ((char_info == &characteristic_sensorPasskey_info) && (offset == 0) && (len == 6)) 
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        memcpy((uint8_t*)&sensor_bridge.passkey, data, len);  
        pstorage_driver_request_store((uint8_t*)(&sensor_bridge.passkey));
			
			  // Set flag to clear bond manager.
        ble_clear_bondmngr_request();
    }
    
		// Write to working encryption mode characteristic value?
    if ((char_info == &characteristic_sensorMitmReqFlag_info) && (offset == 0) && (len == sizeof(sensor_bridge.mitm_req_flag)))
    {
			  // Copy new data to local buffer, and store it to persistent memory.
        sensor_bridge.mitm_req_flag = (data[0] == 1);
        pstorage_driver_request_store((uint8_t*)(&sensor_bridge.mitm_req_flag));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief  Callback which is called from main thread.
 *
 * @return Void.
 */

void my_main_thread_callback(void)
{
	  // If data is received through BLE interface and there no reading packet in progress, send received data through uart.
    if((bridge_rcv_from_ble_mark == true) && (bridge.rx.state == BRIDGE_STATE_COMMAND_WAIT))
    {
        bridge_create_tx_packet(BRIDGE_COMM_RCV_FROM_BLE, sensor_bridge.data_down.payload_length, sensor_bridge.data_down.payload);
        if(bridge_send_packet() == true)
        {
            bridge_rcv_from_ble_mark = false;
        }
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
    if (!comfort_init()) 
    {
        blink(101); 
    }
    
		// Initialize BLE server modules.
    if (!ble_init_server(&server_def, pstorage_driver_init, &sensor_bridge.mitm_req_flag))
    { 
        blink(101); 
    }
    
		// Initialize uart module.
		if (!bridge_uart_init())
    { 
        blink(101); 
    }
    
		/* NOTE: First release: We are going to use a simple RELAY output in this module, the UART app will be upgraded over the air when 
											      the backend is ready to support this functionality.
		*/
		
		NRF_UART0->PSELTXD = 0xFFFFFFFF;
		gpio_write(RELAY_PIN, false);	
		gpio_set_pin_digital_output(RELAY_PIN, PIN_DRIVE_S0S1);
		
		// Initialize services and characteristics for non-onboarding mode.
    if(onboard_get_mode() == ONBOARD_MODE_IDLE)
    {
			  // Check whether there is a need for MITM encryption.
        uint16_t read_enc_flag  = (sensor_bridge.mitm_req_flag ? BLE_CHARACTERISTIC_READ_ENC_REQUIRE  : BLE_CHARACTERISTIC_READ_ENC_REQUIRE_NO_MITM);
        uint16_t write_enc_flag = (sensor_bridge.mitm_req_flag ? BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE : BLE_CHARACTERISTIC_WRITE_ENC_REQUIRE_NO_MITM);
        
        short_service_uuid = sensor_bridge.mitm_req_flag ? short_service_relayr_uuid : short_service_relayr_open_comm_uuid;         
      
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
																		(const uint8_t*)sensor_bridge.sensorID,
																		sizeof(sensorID_t),
																		&characteristic_sensorID_info))
        {
            blink(104);
        }
      
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorBeaconFrequency_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorBeaconFrequency", 
                                    (const uint8_t*)&sensor_bridge.beaconFrequency,
                                    sizeof(sensor_bridge.beaconFrequency),
                                    &characteristic_sensorBeaconFrequency_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorLedState_uuid,
                                    BLE_CHARACTERISTIC_CAN_WRITE | write_enc_flag,
                                    (const uint8_t*)"SensorLedState", 
                                    (const uint8_t*)&sensor_bridge.led_state,
                                    sizeof(sensor_bridge.led_state),
                                    &characteristic_sensorLedState_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorConfig_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE | read_enc_flag | write_enc_flag,
                                    (const uint8_t*)"SensorConfig", 
                                    (const uint8_t*)&sensor_bridge.config,
                                    sizeof(sensor_bridge.config),
                                    &characteristic_sensorConfig_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorData_up_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_NOTIFY | BLE_CHARACTERISTIC_CAN_INDICATE | read_enc_flag,
                                    (const uint8_t*)"SensorDataUp", 
                                    (const uint8_t*)&sensor_bridge.data_up,
                                    sizeof(sensor_bridge.data_up),
                                    &characteristic_sensorData_up_info)) 
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorData_down_uuid,
                                    BLE_CHARACTERISTIC_CAN_WRITE | write_enc_flag,
                                    (const uint8_t*)"SensorDataDown", 
                                    (const uint8_t*)&sensor_bridge.data_down,
                                    sizeof(sensor_bridge.data_down),
                                    &characteristic_sensorData_down_info)) 
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
																		(const uint8_t*)sensor_bridge.sensorID,
																		sizeof(sensorID_t),
																		&characteristic_sensorID_info)) 
        {
            blink(104);
        }
      
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorPasskey_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE,
                                    (const uint8_t*)"SensorPasskey", 
                                    (const uint8_t*)sensor_bridge.passkey,
                                    6,
                                    &characteristic_sensorPasskey_info))
        {
            blink(104);
        }
        
        if (!ble_add_characteristic(&service_info,
                                    characteristic_sensorMitmReqFlag_uuid,
                                    BLE_CHARACTERISTIC_CAN_READ | BLE_CHARACTERISTIC_CAN_WRITE,
                                    (const uint8_t*)"SensorMitmRequireFlag", 
                                    (const uint8_t*)&sensor_bridge.mitm_req_flag,
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
    if (!ble_start_advertising(sensor_bridge.beaconFrequency)) 
    {
        blink(106);
    }
    
    ble_run();

    blink(107);
} 
