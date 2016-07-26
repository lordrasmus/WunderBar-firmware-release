/** @file   Sensors_SensID.c
 *  @brief  File contains functions for handling connected sensors list
 *  		and sensors IDs and processing list for subsriptions.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "Common_Defaults.h"
#include "Sensors_main.h"
#include "Sensors_SensID.h"
#include "MQTT/MQTT_Api_Client/MQTT_Api.h"


// static declarations

static SensId_t MySensorList[NUMBER_OF_SENSORS];


static void Sensors_ID_CreateSubPath(char* buf, char* path, unsigned char index);
static void Sensors_ID_StoreSensor(char* id, unsigned char index);
static void Sensors_ID_ScheduleForSub(unsigned char index);
static void Sensors_ID_ScheduleForUnsub(unsigned char index);
static void Sensors_ID_ByteToHex(char* txt, char x);
static void Sensors_ID_SubscribeMainBoard();
static void Sensors_ID_SetActive(unsigned char index);
static void Sensors_ID_Clear(unsigned char index);
static void Sensors_ID_SetNeedUpdate(unsigned char index);
static void Sensors_ID_ClrNeedUpdate(unsigned char index);




///////////////////////////////////////
/*         public functions          */
///////////////////////////////////////



/**
 *  @brief  Gets Sensor ID string for desired sensor
 *
 *  Returns pointer to sensor id string stored in sensor list
 *
 *  @param  sonsor index
 *
 *  @return Pointer to sensor ID string
 */
char* Sensors_ID_GetSensorID(unsigned char index){
	return ((char *) &MySensorList[index].SensorIDstr);
}

/**
 *  @brief  Gets active status of sensor
 *
 *  @param  sonsor index
 *
 *  @return Sensor active status (1 if active)
 */
char Sensors_ID_GetActiveStatus(unsigned char index){
	return MySensorList[index].active;
}

/**
 *  @brief  Process connection status received from ble master
 *
 *  When sensor is connected to master ble module, it will send sensor id
 *  which should be processed and saved in sensor list.
 *  Also when sensor is disconnected, it should be removed froms ensor list.
 *
 *  @param  sensor id string
 *  @param  sonsor index
 *  @param  sensor connection status
 *
 *  @return void
 */
void Sensors_ID_Process(char* id, char sensIndex, char connStatus){
	// if connection
	if (connStatus == 0)
	{
		Sensors_ID_StoreSensor(id, (unsigned char) sensIndex);
		Sensors_ID_ScheduleForSub((unsigned char) sensIndex);
	}
	else if (connStatus == 1)  // if disconnected
	{
		if (Sensors_ID_GetActiveStatus((unsigned char) sensIndex) == 1)
		{
			Sensors_ID_ScheduleForUnsub((unsigned char) sensIndex);
		}
		Sensors_ID_Clear((unsigned char) sensIndex); // delete sensor from list
	}
}

/**
 *  @brief  Process successful subscription or unsubscription
 *
 *  When subscription or unsubscription is acknowledged clear need update flag
 *
 *  @param  subscribe topic
 *
 *  @return void
 */
void Sensors_ID_ProcessSuccessfulSubscription(char* topic){
	unsigned char cnt;

	for (cnt = 0; cnt < NUMBER_OF_SENSORS; cnt ++)
	{
		if (MySensorList[cnt].needUpdate == 1)
		{
			if (strstr((const char *) topic, (const char *) MySensorList[cnt].SensorIDstr))
				Sensors_ID_ClrNeedUpdate(cnt);
		}
	}
}

/**
 *  @brief  Finds device name index from sensor ID string
 *
 *  @param  sensor id string
 *
 *  @return data id of desired sensor
 */
data_id_t Sensors_ID_FindSensorID(char* topic){
	unsigned char cnt;

	for (cnt = 0; cnt < NUMBER_OF_SENSORS; cnt ++)
	{
		if (Sensors_ID_GetActiveStatus(cnt))
		{
			if (strstr((const char *) topic, (const char *) &MySensorList[cnt].SensorIDstr))
				return cnt;
		}
	}

	if (strstr(topic, (char *) wunderbar_configuration.wunderbar.id))
		return DATA_ID_DEV_CENTRAL;

	return 255;
}

/**
 *  @brief  Form ID string in string
 *
 *  @param  return text string of sensor id
 *  @param  byte array od sensor id
 *
 *  @return void
 */
void Sensors_ID_FormSensIdStr(char* sensIdStr, char* sensIdArr){
	char *ptr = sensIdStr, temp_txt[2];
	unsigned char cnt;

	// load template for sensor ID string
	memcpy((void *) ptr, (const void *) sensor_id_template, sizeof(SensorIDstr_t));

	for (cnt = 0; cnt < SENSOR_ID_LEN; cnt ++)
	{
		if (*ptr == '-')
			ptr ++;

		Sensors_ID_ByteToHex((char *) &temp_txt[0], (char) sensIdArr[cnt]);
		memcpy((void *) ptr, (const void *) temp_txt, 2);
		ptr += 2;
	}
	// add zero termination
	*ptr ++ = 0;
	*ptr ++ = 0;
}

/**
 *  @brief  Goes through entire list of sensors and subscribes on all connected sensors
 *
 *  @return void
 */
void Sensors_ID_CheckSubList(){
	unsigned char i;

	for (i = 0; i < NUMBER_OF_SENSORS; i ++)
	{
		if (Sensors_ID_GetActiveStatus(i))
			Sensors_ID_ScheduleForSub(i);
	}

	Sensors_ID_SubscribeMainBoard();
}

/**
 *  @brief  Clear list of connected sensors
 *
 *  Should b called when master ble reset is detected
 *
 *  @return void
 */
void Sensors_ID_ClearList(){
	unsigned char i;

	for (i = 0; i < NUMBER_OF_SENSORS; i ++)
	{
		if (Sensors_ID_GetActiveStatus(i))
			Sensors_ID_ScheduleForUnsub(i);
		Sensors_ID_Clear(i);
	}
}




///////////////////////////////////////
/*         static functions          */
///////////////////////////////////////



/**
 *  @brief  Activate sensor
 *
 *  Set active flag for desired sensor in sensor list
 *
 *  @param  sensor index
 *
 *  @return void
 */
static void Sensors_ID_SetActive(unsigned char index){
	MySensorList[index].active = 1;
}

/**
 *  @brief  Creates subtopic sub path with sensor ID and desired subtopic
 *
 *  Create subscribe topic with desired sensor id and subtopic
 *
 *  @param  pointer to output buffer for result string
 *  @param  desired path
 *  @param  sensor index
 *
 *  @return void
 */
static void Sensors_ID_CreateSubPath(char* buf, char* path, unsigned char index){
	sprintf(buf, MQTT_TOPIC_PREFIX "/%s" "%s" , Sensors_ID_GetSensorID(index), path);
}

/**
 *  @brief  Clears desired sensor id from list
 *
 *  @param  sensor index
 *
 *  @return void
 */
static void Sensors_ID_Clear(unsigned char index){
	memset((void *) &MySensorList[index], (int) 0, sizeof(SensId_t));
}

/**
 *  @brief  Sets Need update flag for desired sensor
 *
 *  @param  sensor index
 *
 *  @return void
 */
static void Sensors_ID_SetNeedUpdate(unsigned char index){
	MySensorList[index].needUpdate = 1;
}

/*
 *  @brief  Clears Need update flag for desired sensor
 *
 *  @param  sensor index
 *
 *  @return void
 */
static void Sensors_ID_ClrNeedUpdate(unsigned char index){
	MySensorList[index].needUpdate = 0;
}

/**
 *  @brief  Stores sensor id in sensors list.
 *
 *  Creates Sensor id string from 16 byte array, and stores it into sensor list
 *  under desired index (sensor type)
 *
 *  @param  pointer to byte array (sensor id)
 *  @param  sensor index
 *
 *  @return void
 */
static void Sensors_ID_StoreSensor(char* id, unsigned char index){
	Sensors_ID_FormSensIdStr((char *) &MySensorList[index].SensorIDstr, id);
	Sensors_ID_SetNeedUpdate(index);
	Sensors_ID_SetActive(index);
}

/**
 *  @brief  Subscribe main board with main board id
 *
 *  @return void
 */
static void Sensors_ID_SubscribeMainBoard(){
	char temp_buf[75];

	sprintf(temp_buf, MQTT_TOPIC_PREFIX "/%s" MQTT_SENS_SUBTOPICS_CMD_PING, (char *) wunderbar_configuration.wunderbar.id);
	MQTT_Api_Subscr(temp_buf, MQTT_MSG_OPT_QOS_SUB);
}

/**
 *  @brief  Schedule topics for subscription
 *
 *  For desired sensor (found in list by index) create subscription topics
 *  and schedule them for sending to mqt server.
 *
 *  @param  Sensor index in sensor list
 *
 *  @return void
 */
static void Sensors_ID_ScheduleForSub(unsigned char index){
	char temp_buf[75];

	Sensors_ID_CreateSubPath(temp_buf, MQTT_SENS_SUBTOPICS_CONFIG,   index);
	MQTT_Api_Subscr(temp_buf, MQTT_MSG_OPT_QOS_SUB);
	Sensors_ID_CreateSubPath(temp_buf, MQTT_SENS_SUBTOPICS_CMD_LED,  index);
	MQTT_Api_Subscr(temp_buf, MQTT_MSG_OPT_QOS_SUB);
	Sensors_ID_CreateSubPath(temp_buf, MQTT_SENS_SUBTOPICS_CMD_PING, index);
	MQTT_Api_Subscr(temp_buf, MQTT_MSG_OPT_QOS_SUB);

	if ((index == DATA_ID_DEV_IR) || (index == DATA_ID_DEV_BRIDGE))
	{
		Sensors_ID_CreateSubPath(temp_buf, MQTT_SENS_SUBTOPICS_CMD_DATA, index);
		MQTT_Api_Subscr(temp_buf, MQTT_MSG_OPT_QOS_SUB);
	}
}

/**
 *  @brief  Schedule topics for unsubscription
 *
 *  For desired sensor (found in list by index) create unsubscription topics
 *  and schedule them for sending to mqt server.
 *
 *  @param  Sensor index in sensor list
 *
 *  @return void
 */
static void Sensors_ID_ScheduleForUnsub(unsigned char index){
	char temp_buf[75];

	Sensors_ID_CreateSubPath(temp_buf, MQTT_SENS_SUBTOPICS_CONFIG,   index);
	MQTT_Api_Unsubscr(temp_buf);
	Sensors_ID_CreateSubPath(temp_buf, MQTT_SENS_SUBTOPICS_CMD_LED,  index);
	MQTT_Api_Unsubscr(temp_buf);
	Sensors_ID_CreateSubPath(temp_buf, MQTT_SENS_SUBTOPICS_CMD_PING, index);
	MQTT_Api_Unsubscr(temp_buf);
	if ((index == DATA_ID_DEV_IR) || (index == DATA_ID_DEV_BRIDGE))
	{
		Sensors_ID_CreateSubPath(temp_buf, MQTT_SENS_SUBTOPICS_CMD_DATA, index);
		MQTT_Api_Unsubscr(temp_buf);
	}
}

/**
 *  @brief  Gets hex representation of one byte
 *
 *  Transform one byte into string hex representation.
 *  For example 161 -> "a1"
 *
 *  @param  return text of byte value
 *  @param  byte to transform
 *
 *  @return void
 */
static void Sensors_ID_ByteToHex(char* txt, char x){
	txt[0] = hex_array[(x >> 4) & 0x0F];
	txt[1] = hex_array[x & 0x0F];
}
