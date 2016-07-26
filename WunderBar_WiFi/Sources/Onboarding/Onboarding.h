/** @file   Onboarding.h
 *  @brief  File contains functions used for onboarding process.
 *  		Initiate onboarding and handles state machine during onboarding
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#ifndef ONBOARDING_H_
#define ONBOARDING_H_

#include <stdint.h>


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

typedef enum {
	ONBOARDING_OFF,
	ONBOARDING_START,
	ONBOARDING_SERVER_UP,
	ONBOARDING_WAIT,
	ONBOARDING_AP_RECV,
	ONBOARDING_BLE_RECV,
	ONBOARDING_SUCCESS,
	ONBOARDING_FAILED
} Onboarding_Process_State_t;

typedef struct {
	char ssid[22];
	char ip[16];
	char subnetMask[16];
	char port[5];
} AP_Parameters_t;


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// wunderbar json strings for parse

#define CFG_PASSKEY 				"passkey"
#define CFG_HTU 					"htu"
#define CFG_GYRO 					"gyro"
#define CFG_LIGHT					"light"
#define CFG_MICROPHONE				"microphone"
#define CFG_BRIDGE					"bridge"
#define CFG_IR						"ir"

#define CFG_WUNDERBAR				"wunderbar"
#define CFG_WIFI_SSID				"wifi_ssid"
#define CFG_WIFI_PASS				"wifi_pass"
#define CFG_WUNDERBARID				"master_id"
#define CFG_WUNDERBARPASS			"wunderbar_security"

#define CFG_CLOUD					"cloud"
#define CFG_CLOUD_URL               "url"


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// response for wifi client

#define ONBOARDING_WIFI_RESPONSE "{\"result\":%d}"


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// time constants for onboarding process

#define ONBOARDING_BUTTON_TIMEOUT   2000
#define ONBOARDING_LED_BLINK         500
#define ONBOARDING_SUCCESS_TIMEOUT  5000


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// mask constants

#define CFG_PASS_MASK 				0x0000003F
#define CFG_PASS_HTU_MASK   		0x00000001
#define CFG_PASS_GYRO_MASK   		0x00000002
#define CFG_PASS_LIGHT_MASK   		0x00000004
#define CFG_PASS_MICROPHONE_MASK   	0x00000008
#define CFG_PASS_BRIDGE_MASK   		0x00000010
#define CFG_PASS_IR_MASK   			0x00000020

#define CFG_WUNDERBAR_MASK			0x07030000
#define CFG_WIFI_SSID_MASK          0x00010000
#define CFG_WIFI_PASS_MASK          0x00020000
#define CFG_WUNDERBAR_ID_MASK       0x01000000
#define CFG_WUNDERBAR_PASS_MASK     0x02000000

#define CFG_CLOUD_URL_MASK 			0x04000000

#define CFG_PASS_FAILED_MASK        0x10000000
#define CFG_FLWR_FAILED_MASK        0x20000000
#define CFG_BADJSON_FAILED_MASK     0x40000000
#define CFG_FAILED_MASK  		    0x70000000


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// error responses for WiFi

#define WIFI_RESP_ERR_OK					200				// success
#define WIFI_RESP_ERR_NO_CFG				404				// no config written (possible non recognized json tokens) - this is not considered as error
#define WIFI_RESP_ERR_INVALID_JSON			405				// invalid json format message
#define WIFI_RESP_ERR_PASSKEY_WRITE_FAIL 	406				// failed to write passkey config to ble master
#define WIFI_RESP_ERR_CFG_SAVE_FAIL 		407				// error saving new configuration


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// public functions

/**
 *  @brief  Onboarding state machine.
 *
 *  Should be called frequently when onboarding process is activated.
 *  Starts Limited Access Point and tcp server at predefined port
 *  and waits for new configuration from WiFi client or from ble master module
 *
 *  When new configuration is received, saves it into flash.
 *
 *  @return void
 */
void Onbrd_State_Machine();

/**
 *  @brief  WiFi client closed connection
 *
 *  Called when WiFi client is disconnected from tcp server.
 *  Will be catch only if client sent data.
 *
 *  @return void
 */
void Onbrd_ClientDisconnected();

/**
 *  @brief  On board button press event
 *
 *  Should be called when user (onboarding) button is pressed.
 *  Set pressed state and starts measuring time that the button is pressed.
 *
 *  @return void
 */
void Onbrd_ButtonPressEvent();

/**
 *  @brief  Timer poll to detect 2 second hold and timeouts
 *
 *  Should be called from timer interrupt (TI2)
 *  Polls user button and detects short and long press of user button.
 *
 *  @return void
 */
void Onbrd_Poll();

/*
 *  @brief  Received config from ble master module.
 *
 *  Called when new configuration is received from ble master.
 *  After this we should process new data.
 *
 *  @return void
 */
void Onbrd_MasterBleReceived();

/**
 *  @brief  Received config from wifi client.
 *
 *  Called when new configuration is received from WiFi client.
 *  After this we should process new data.
 *
 *  @return void
 */
void Onbrd_WifiReceived();

/**
 *  @brief  Initiate onboarding process.
 *
 *  Signal that Onboarding process should be initiated.
 *  Used to send board into onboarding mode on start, if the board is blank (never configured)
 *
 *  @return void
 */
void Onbrd_GoToStart();

/**
 *  @brief  Set onboarding start flag.
 *
 *  Signal that onboarding process has started. Can be set from WiFi AP mode of ble mode.
 *  Used to detect that onboarding has been started and eventually error condition
 *
 *  @return void
 */
void Onbrd_SetStartProcessFlag();

/**
 *  @brief  Update current time
 *
 *  Updates current time of the process. Used to detect timeout.
 *
 *  @return void
 */
void Onbrd_UpdateCurrentProcessTime();

/**
 *  @brief  Process Onboarding configs from WiFi client
 *
 *  Process data received from WiFi client during onboarding process.
 *  Received passkeys will be sent to mastel ble device over SPI
 *  and wunderbar and cloud configs will be saved in flash and used later as valid parameters.
 *
 *  @param  pointer to received text message from WiFi client
 *
 *  @return error code for parsed message
 */
unsigned int Onbrd_Process_msg(char* msg);

/**
 *  @brief  Processes Onboarding configs from master ble module
 *
 *  Process data received from master ble module during Onboarding process.
 *  Parses data received and stores it into wunderbar configs global variable.
 *
 *  @param  index which determine config parameter
 *  @param  pointer to text with config
 *
 *  @return void
 */
void Onbrd_IncomingCfg(char index, char* cfg);


#endif // ONBOARDING_H_
