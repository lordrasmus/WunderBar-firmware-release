/** @file   Sensors_main.c
 *  @brief  File contains functions for processing sensor connection and
 *  		messages from and to sensors. Handles messages and prepares message format
 *  		for further sending.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */


#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "Common_Defaults.h"
#include "Sensors_main.h"
#include "Sensors_Cfg_Handler.h"
#include "Sensors_SensID.h"
#include "My_Sensors/Sensors_common.h"
#include "../MQTT/MQTT_API_Client/MQTT_Api.h"


// static declarations


static spi_frame_t myLastSPIFrame;

// data handlers function pointer list for processing down data from cloud
static Sensors_DataHandlerMQTT Sensors_DataHandlersMQTT[8] =   {
													Sensors_htu_ProcessData,
													Sensors_gyro_ProcessData,
													Sensors_light_ProcessData,
													Sensors_sound_ProcessData,
													Sensors_bridge_ProcessData,
													Sensors_ir_ProcessData,
													0,
													MainBoard_ProcessData
												};

// data handlers function pointer list for preparing up data for cloud
static Sensors_DataHandlerBT Sensors_DataHandlersBT[8] =   {
													Sensors_htu_Update,
													Sensors_gyro_Update,
													Sensors_light_Update,
													Sensors_sound_Update,
													Sensors_bridge_Update,
													Sensors_ir_Update,
													0,
													MainBoard_Update
												};

static void Sensors_Update_Data(spi_frame_t* SPI_msg);
static void Sensors_Update_Response(spi_frame_t* SPI_msg);
static void Sensors_SetLastMsg(spi_frame_t* SPI_msg);
static void Sensors_DiscardLastSpiFrame();
static int  Sensors_AddMessageID(char** pptr);
static int  Sensors_Add_SensorId(char** pptr, data_id_t data_id);
static int  Sensors_AddSubtopic_SensChar(char** pptr, field_id_char_index_t field_id);
static void Sensors_Save_CentralFwRev(char* fwRev);
static field_id_char_index_t Sensors_ExtractSensChar(char* topic);
static bool Sensors_ResponseHandlerBT(char resp, char* buf);




///////////////////////////////////////
/*         public functions          */
///////////////////////////////////////


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
void Sensors_Process_Data(spi_frame_t* SPI_msg){
	// received regular message from master ble module (status, notification)
	if 		((SPI_msg->data_id >= DATA_ID_DEV_HTU) && (SPI_msg->data_id <= DATA_ID_DEV_IR))
	{
		Sensors_Update_Data(SPI_msg);
	}
	// received onboarding configuration parameters from master ble module
	else if (SPI_msg->data_id == DATA_ID_CONFIG)
	{
		Sensors_Cfg_ProcessBleMsg(SPI_msg);
	}
 	// received firmware revision from master ble module (sent on master ble power up)
	else if (SPI_msg->data_id == DATA_ID_DEV_CENTRAL)
	{
		Sensors_Save_CentralFwRev((char *) &SPI_msg->data[0]);
	}
 	// received response from master ble on previous command
	else if ((SPI_msg->data_id >= DATA_ID_RESPONSE_OK) && (SPI_msg->data_id <= DATA_ID_RESPONSE_TIMEOUT))
	{
		Sensors_Update_Response(SPI_msg);
	}
}

/**
*  @brief  Parses new mqtt message received from server
*
*  Should be called when new mqtt message is received.
*  Invokes appropriate function from data handler function pointer list,
*  depending on sensor id mentioned in topic.
*
*  It will be used as callback function in mqtt lib.
*
*  @param  Pointer to struct of mqtt message
*
*  @return void
*/
void Sensors_MsgParse(MQTT_User_Message_t* MyMessage){
	spi_frame_t SPI_msg;

	if (!MQTT_Get_RunnigStatus())
		return;

	// get sensor ID from sensor list
	if ((SPI_msg.data_id = Sensors_ID_FindSensorID(MyMessage->topicStr)) == 255)
		return;

	// get characteristic
	if ((SPI_msg.field_id = Sensors_ExtractSensChar(MyMessage->topicStr)) == 255)
		return;

	// add operation flag
	SPI_msg.operation = OPERATION_WRITE;

	// process message and data field
	if (Sensors_DataHandlersMQTT[SPI_msg.data_id](&SPI_msg, MyMessage->payloadStr) != 0)
		return;

	// send SPI message to master ble
	Sensors_SPI_SendMsg(&SPI_msg);

	// save last SPI message
	Sensors_SetLastMsg(&SPI_msg);
}

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
void Sensors_ProcessTimeout(){
	spi_frame_t dummy_SPI_frame;

	if (myLastSPIFrame.data_id == SENSOR_DUMMY_BYTE)
		return;

	dummy_SPI_frame.data_id = DATA_ID_RESPONSE_TIMEOUT;
	Sensors_Update_Response(&dummy_SPI_frame);
	// delete last stored spi frame
	Sensors_DiscardLastSpiFrame();
}

/**
*  @brief  Init Sensors stack
*
*  Delete connected sensors list
*  Set call back function for received mqtt messages
*
*  @return void
*/
void Sensors_Init(){
	Sensors_ID_ClearList();			// clear list of connected ble modules (on master ble reset)
	MQTT_Api_SetReceiveCallBack(Sensors_MsgParse); // set user call back for mqtt return messages
}

/**
 *  @brief  Send run command to ble master
 *
 *  This function will send master ble module command to enter run mode.
 *  Should be sent when board is connected to cloud.
 *
 *  @return void
 */
void Sensor_Cfg_Run(){
	spi_frame_t spi_msg;

	spi_msg.data_id   = DATA_ID_CONFIG;
	spi_msg.field_id  = FIELD_ID_RUN;
	spi_msg.operation = OPERATION_READ;

	Sensors_SPI_SendMsg(&spi_msg);
}

/**
 *  @brief  Send cfg command to ble master
 *
 *  This function will send master ble module command to enter config mode.
 *  Should be sent when board is entering onboarding mode.
 *
 *  @return void
 */
void Sensor_Cfg_Start(){
	spi_frame_t spi_msg;

	spi_msg.data_id   = DATA_ID_CONFIG;
	spi_msg.field_id  = FIELD_ID_CONFIG_START;
	spi_msg.operation = OPERATION_READ;

	Sensors_SPI_SendMsg(&spi_msg);
}



///////////////////////////////////////
/*         static functions          */
///////////////////////////////////////


/**
*  @brief  Updates data on mqtt server with new data from BT
*
*  Receives SPI frame from the master ble module, and prepares mqtt message for
*  publishing on cloud. Creates topic string and payload string according to data.
*
*  @param  SPI frame message from ble
*
*  @return void
*/
static void Sensors_Update_Data(spi_frame_t* SPI_msg){
	MQTT_User_Message_t MyMessage;
	char* ptr = MyMessage.topicStr;

	strcpy(ptr, MQTT_TOPIC_PREFIX);
	ptr += strlen(MyMessage.topicStr);

	if (Sensors_Add_SensorId(&ptr, SPI_msg->data_id) == 0)
		return;			// invalid sensor name index

	Sensors_DataHandlersBT[SPI_msg->data_id](SPI_msg, MyMessage.payloadStr);   		// handle data from sensor, and create payload string

	if (Sensors_AddSubtopic_SensChar(&ptr, SPI_msg->field_id) == 0)
		return;         // invalid sensor characteristic

	if (Sensors_ID_GetActiveStatus(SPI_msg->data_id) != 1)
		return;         // sensor not active


	// clear message in progress flag for hardware and firmware revision query
	if ((SPI_msg->field_id == FIELD_ID_CHAR_HARDWARE_REVISION) || (SPI_msg->field_id == FIELD_ID_CHAR_FIRMWARE_REVISION))
	{
		MQTT_Msg_ClearMsgInProgress();
		Sensors_DiscardLastSpiFrame();
	}

	MyMessage.payloadlen = strlen(MyMessage.payloadStr);							// get payload length

	if (MQTT_Get_RunnigStatus())
		MQTT_Api_Publish(&MyMessage);												// schedule for publishing
}

/**
*  @brief  Updates response on mqtt server
*
*  Receives SPI frame from the master ble module, and prepares mqtt message for
*  publishing on cloud. Generates response message on last down msg from cloud.
*
*  @param  SPI frame message from ble
*
*  @return void
*/
static void Sensors_Update_Response(spi_frame_t* SPI_msg){
	MQTT_User_Message_t MyMessage;
	char* ptr = MyMessage.topicStr;

	strcpy(ptr, MQTT_TOPIC_PREFIX);
	ptr += strlen(MyMessage.topicStr);

	if (Sensors_Add_SensorId(&ptr, myLastSPIFrame.data_id) == 0)
		return;			// invalid sensor name index

	if (Sensors_AddSubtopic_SensChar(&ptr, myLastSPIFrame.field_id) == 0)
		return;         // invalid sensor characteristic

	if (Sensors_AddMessageID(&ptr) <= 1)
		return;

	// add payload
	Sensors_ResponseHandlerBT(SPI_msg->data_id, MyMessage.payloadStr);

	// response received, delete saved message
	MQTT_Msg_ClearMsgInProgress();
	Sensors_DiscardLastSpiFrame();

	MyMessage.payloadlen = strlen(MyMessage.payloadStr);

	if (MQTT_Get_RunnigStatus())
		MQTT_Api_Publish(&MyMessage);
}

/**
*  @brief  Updates firmware revision on mqtt server
*
*  When main board firmware revision is requested, mqtt message will be generated
*  with firmware revision parameters and main board topic.
*  There is no communication with master ble module.
*
*  @param  SPI frame message
*
*  @return void
*/
void MainBoard_Update_FwRev(spi_frame_t* SPI_msg){
	MQTT_User_Message_t MyMessage;
	char* ptr = MyMessage.topicStr;

	strcpy(ptr, MQTT_TOPIC_PREFIX);
	ptr += strlen(MyMessage.topicStr);

	strcpy(ptr, "/");
	ptr++;

	strcpy(ptr, (const char *) wunderbar_configuration.wunderbar.id);
	ptr += strlen((const char *) wunderbar_configuration.wunderbar.id);

	Sensors_AddSubtopic_SensChar(&ptr, SPI_msg->field_id);

	MainBoard_Update(SPI_msg, MyMessage.payloadStr);

	MyMessage.payloadlen = strlen(MyMessage.payloadStr);

	if (MQTT_Get_RunnigStatus())
		MQTT_Api_Publish(&MyMessage);
}

/**
*  @brief  Stores last message from BT device
*
*  @param  Pointer to SPI frame struct of incoming message
*
*  @return void
*/
static void Sensors_SetLastMsg(spi_frame_t* SPI_msg){
	myLastSPIFrame = *SPI_msg;
}

/**
*  @brief  Discard last saved spi message
*
*  @return void
*/
static void Sensors_DiscardLastSpiFrame(){
	memset((void *) &myLastSPIFrame, SENSOR_DUMMY_BYTE, sizeof(spi_frame_t));
}

/**
 *  @brief  Append message id to a given string.
 *
 *  Appends message id and advances pointer to the end of new string.
 *  Also returns number of written bytes.
 *
 *  @param  source string
 *
 *  @return Number of written bytes
 */
static int Sensors_AddMessageID(char** pptr){
	int result = 0;

	strcpy(*pptr, (const char *) "/");
	*pptr += 1;

	strcpy(*pptr, (const char *) Sensors_JSON_GetStoredMsgId());

	result = strlen((const char *) *pptr) + 1;

	return result;
}

/**
*  @brief  Append sensor id into topic string
*
*  Appends sensor id and advances pointer to the end of new string.
*  Also returns number of written bytes.
*
*  @param  source string
*  @param  sensor data id
*
*  @return Number of written bytes
*/
static int Sensors_Add_SensorId(char** pptr, data_id_t data_id){
	int result;

	if (data_id > DATA_ID_DEV_IR)
		return 0;

	strcpy(*pptr, (const char *) "/");
	*pptr += 1;

	strcpy(*pptr, (const char *) Sensors_ID_GetSensorID(data_id));
	result = strlen((const char *) Sensors_ID_GetSensorID(data_id));
	*pptr += result;

	return (result + 1);
}

/**
*  @brief  Process firmware revision message from master ble
*
*  On power up master ble module sends its firmware revision.
*  This can be used to detect power up event on master ble.
*  Firmware revision will be stored and used later if requested.
*
*  @param  Pointer to master ble firmware rev string
*
*  @return void
*/
static void Sensors_Save_CentralFwRev(char* fwRev){
	Sensors_FormFrmHwRevStr(fwRev);
	strcpy(Sensors_GetBleFirmRevStr(), fwRev);

	// clear list of connected ble modules (on master ble reset)
	Sensors_ID_ClearList();

	// if we are running start master ble
	if (MQTT_Get_RunnigStatus())
		Sensor_Cfg_Run();
}

/**
*  @brief  Determines which characteristic is mentioned in topic message
*
*  Extract field id characteristics from topic string.
*
*  @param  Topic string
*
*  @return field id char
*/
static field_id_char_index_t Sensors_ExtractSensChar(char* topic){

	if      (strstr(topic, SENS_DOWN_CHAR_BEACONFREQ))
		return FIELD_ID_CHAR_SENSOR_BEACON_FREQUENCY;
	else if (strstr(topic, SENS_DOWN_CHAR_FREQUENCY))
		return FIELD_ID_CHAR_SENSOR_FREQUENCY;
	else if (strstr(topic, SENS_DOWN_CHAR_THRESHOLD))
		return FIELD_ID_CHAR_SENSOR_THRESHOLD;
	else if (strstr(topic, SENS_DOWN_CHAR_SENSCFG))
		return FIELD_ID_CHAR_SENSOR_CONFIG;
//	else if (strstr(topic, SENS_DOWN_MANUFACTURER_NAME))
//		return FIELD_ID_CHAR_MANUFACTURER_NAME;
	else if (strstr(topic, SENS_DOWN_HARDWARE_REV))
		return FIELD_ID_CHAR_HARDWARE_REVISION;
	else if (strstr(topic, SENS_DOWN_FIRMWARE_REV))
		return FIELD_ID_CHAR_FIRMWARE_REVISION;
	else if (strstr(topic, SENS_DOWN_LED_STATE))
		return FIELD_ID_CHAR_SENSOR_LED_STATE;
	else if (strstr(topic, SENS_DOWN_DATA))
		return FIELD_ID_CHAR_SENSOR_DATA_W;

	return 255;
}

/**
*  @brief  Append sensor characteristic subtopic to input string
*
*  Appends sensor characteristic and advances pointer to the end of new string.
*  Also returns number of written bytes.
*
*  @param  source string
*  @param  sensor filed id
*
*  @return Number of written bytes
*/
static int Sensors_AddSubtopic_SensChar(char** pptr, field_id_char_index_t field_id){
	int result;

	switch (field_id)
	{
	case FIELD_ID_CHAR_SENSOR_BEACON_FREQUENCY :
		strcpy(*pptr, SENS_UP_CHAR_BEACONFREQ);
		break;
	case FIELD_ID_CHAR_SENSOR_FREQUENCY :
		strcpy(*pptr, SENS_UP_CHAR_FREQUENCY);
		break;
	case FIELD_ID_CHAR_SENSOR_LED_STATE :
		strcpy(*pptr, SENS_UP_LED_STATE);
		break;
	case FIELD_ID_CHAR_SENSOR_THRESHOLD :
		strcpy(*pptr, SENS_UP_CHAR_THRESHOLD);
		break;
	case FIELD_ID_CHAR_SENSOR_CONFIG :
		strcpy(*pptr, SENS_UP_CHAR_SENSCFG);
		break;
	case FIELD_ID_CHAR_SENSOR_DATA_R :
		strcpy(*pptr, SENS_UP_DATA);
		break;
	case FIELD_ID_CHAR_SENSOR_DATA_W :
		strcpy(*pptr, SENS_DOWN_DATA);
		break;
	case FIELD_ID_CHAR_BATTERY_LEVEL :
		strcpy(*pptr, SENS_UP_CHAR_BATTERY_LEVEL);
		break;
	case FIELD_ID_CHAR_MANUFACTURER_NAME :
		strcpy(*pptr, SENS_UP_MANUFACTURER_NAME);
		break;
	case FIELD_ID_CHAR_HARDWARE_REVISION :
		strcpy(*pptr, SENS_UP_HARDWAREREV);
		break;
	case FIELD_ID_CHAR_FIRMWARE_REVISION :
		strcpy(*pptr, SENS_UP_FIRMWAREREV);
		break;

	default :
		return 0;
	}

	result = strlen((const char *) *pptr);
	*pptr += result;

	return result;
}

/**
*  @brief  Generate response string
*
*  Generates response string for publishing on mqtt.
*
*  @param  Response code
*  @param  Output buffer
*
*  @return True if response code is valid.
*/
static bool Sensors_ResponseHandlerBT(char resp, char* buf){
	char text[4];

	switch (resp)
	{
	case DATA_ID_RESPONSE_OK :
		strcpy(text, SENS_RESPONSE_ERROR_OK);
		break;

	case DATA_ID_RESPONSE_ERROR :
		strcpy(text, SENS_RESPONSE_ERROR_NOT_FOUND);
		break;

	case DATA_ID_RESPONSE_BUSY :
		strcpy(text, SENS_RESPONSE_ERROR_NOT_FOUND);
		break;

	case DATA_ID_RESPONSE_NOT_FOUND :
		strcpy(text, SENS_RESPONSE_ERROR_NOT_FOUND);
		break;

	case DATA_ID_RESPONSE_TIMEOUT :
		strcpy(text, SENS_RESPONSE_ERROR_TIMEOUT);
		break;

	default :
		return false;
	}

	sprintf(buf, Template_error_response, text);
	return true;
}
