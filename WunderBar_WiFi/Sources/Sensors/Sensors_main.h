/** @file   Sensors_main.h
 *  @brief  File contains functions for processing sensor connection and
 *  		messages from and to sensors. Handles messages and prepares message format
 *  		for further sending.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include "wunderbar_common.h"

#define SENSOR_DUMMY_BYTE 0xDD

// handling function pointer for incoming messages from MQTT
typedef int (*Sensors_DataHandlerMQTT)(spi_frame_t* SPI_msg, char* msg);
// handling function pointer for incoming messages from BT
typedef void (*Sensors_DataHandlerBT) (spi_frame_t* SPI_msg, char* buf);

//////////////////////////////////////////////////////////////////////////////////


// mqtt topics for incoming messages (to extract correct characteristic)
#define SENS_DOWN_CHAR_FREQUENCY   		"/config/frequency"
#define SENS_DOWN_CHAR_BEACONFREQ  		"/config/beaconfreq"
//#define SENS_DOWN_CHAR_BEACONINT  	"/config/beaconinterrupt"
#define SENS_DOWN_CHAR_SENSCFG      	"/config/sensorcfg"
#define SENS_DOWN_CHAR_THRESHOLD    	"/config/threshold"
#define SENS_DOWN_MANUFACTURER_NAME 	"/cmd/ping/manufacturername"
#define SENS_DOWN_HARDWARE_REV      	"/cmd/ping/hardwarerev"
#define SENS_DOWN_FIRMWARE_REV      	"/cmd/ping/firmwarerev"
#define SENS_DOWN_LED_STATE         	"/cmd/led"
#define SENS_DOWN_DATA		          	"/cmd"


//////////////////////////////////////////////////////////////////////////////////


// mqtt topics for outgoing messages (to create correct subtopic)
#define SENS_UP_CHAR_FREQUENCY      	"/config/frequency"
#define SENS_UP_CHAR_BEACONFREQ			"/config/beaconfreq"
#define SENS_UP_CHAR_BATTERY_LEVEL 		"/data/power"
//#define SENS_UP_CHAR_BEACONINT		"/config/beaconinterrupt"
#define SENS_UP_CHAR_SENSCFG			"/config/sensorcfg"
#define SENS_UP_CHAR_THRESHOLD			"/config/threshold"

#define SENS_UP_HARDWAREREV				"/data/hardwarerev"
#define SENS_UP_FIRMWAREREV				"/data/firmwarerev"
#define SENS_UP_MANUFACTURER_NAME		"/data/manufacturername"
#define SENS_UP_LED_STATE				"/cmd/led"
#define SENS_UP_DATA					"/data"
#define SENS_UP_STATUS					"/data/status"


//////////////////////////////////////////////////////////////////////////////////

// mqtt subscription list

#define MQTT_SENS_SUBTOPICS_CONFIG   	"/config/+"
#define MQTT_SENS_SUBTOPICS_CMD_DATA 	"/cmd/"
#define MQTT_SENS_SUBTOPICS_CMD_LED  	"/cmd/led/"
#define MQTT_SENS_SUBTOPICS_CMD_PING 	"/cmd/ping/+"


//////////////////////////////////////////////////////////////////////////////////

// response messages

#define Template_error_response "{\"result\":%s}"

#define SENS_RESPONSE_ERROR_OK            	"200"
#define SENS_RESPONSE_ERROR_NOT_FOUND     	"404"
#define SENS_RESPONSE_ERROR_TIMEOUT       	"408"
#define SENS_RESPONSE_ERROR_UNAUTHORIZED  	"401"


//////////////////////////////////////////////////////////////////////////////////

/**
*  @brief  Init Sensors stack
*
*  Delete connected sensors list
*  Set call back function for received mqtt messages
*
*  @return void
*/
void Sensors_Init();

/*
*  @brief  Processes incomming message from master ble module (SPI)
*
*  Should be called when new message is received over SPI.
*  Processes type of the message and act accordingly.
*
*  @param  SPI message frame
*
*  @return void
*/
void Sensors_Process_Data(spi_frame_t* SPI_msg);

/**
 *  @brief  Send cfg command to ble master
 *
 *  This function will send master ble module command to enter config mode.
 *  Should be sent when board is entering onboarding mode.
 *
 *  @return void
 */
void Sensor_Cfg_Start();

/**
 *  @brief  Send run command to ble master
 *
 *  This function will send master ble module command to enter run mode.
 *  Should be sent when board is connected to cloud.
 *
 *  @return void
 */
void Sensor_Cfg_Run();

/*
*  @brief  Process response message timeout
*
*  Processes timeout for response message.
*  When new request comes from cloud, response should be generated.
*  In case of timeout, last SPI message should be discarded and timeout
*  response will be sent up on cloud
*
*  @return void
*/
void Sensors_ProcessTimeout();


//////////////////////////////////////////////////////////////////////////////////

/**
*  @brief  Send message to BT via SPI
*
*  Function receives SPI frame, which should be sent to master ble module.
*  First, message size will be calculated and entire message will be sent to ble module.
*
*  @param  SPI frame
*
*  @return True if desired number of bytes were sent.
*/
bool Sensors_SPI_SendMsg(spi_frame_t* SPI_msg);

/**
*  @brief  Read data from BT via SPI
*
*  Master ble module sill trigger ext interrupt when need something t send.
*  This function should be called to collect this data. Function will read
*  header bytes first and calculate how much bytes are left and read them too.
*  After entire message is read, it will form SPI frame and send it for further processing.
*
*  @return void
*/
void Sensors_SPI_ReadMsg();
