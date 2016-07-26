/** @file   Sensors_SensID.h
 *  @brief  File contains functions for handling connected sensors list
 *  		and sensors IDs and processing list for subsriptions.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#ifndef SENSORS_SENSID_H_
#define SENSORS_SENSID_H_

#include "wunderbar_common.h"

#define SENSOR_ID_LEN  				16

typedef char SensorIDstr_t[38];

typedef struct {
	SensorIDstr_t   SensorIDstr;
	char	 		needUpdate;
	char			active;
} SensId_t;


static const char hex_array[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
static const SensorIDstr_t sensor_id_template = "ffffffff-ffff-ffff-ffff-ffffffffffff";

// public functions

/**
 *  @brief  Goes through entire list of sensors and subscribes on all connected sensors
 *
 *  @return void
 */
void Sensors_ID_CheckSubList();

/**
 *  @brief  Clear list of connected sensors
 *
 *  Should b called when master ble reset is detected
 *
 *  @return void
 */
void Sensors_ID_ClearList();

/**
 *  @brief  Form ID string in string
 *
 *  @param  return text string of sensor id
 *  @param  byte array od sensor id
 *
 *  @return void
 */
void Sensors_ID_FormSensIdStr(char* sensIdStr, char* sensIdArr);

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
void Sensors_ID_Process(char* id, char sensIndex, char connStatus);

/**
 *  @brief  Process successful subscription or unsubscription
 *
 *  When subscription or unsubscription is acknowledged clear need update flag
 *
 *  @param  subscribe topic
 *
 *  @return void
 */
void Sensors_ID_ProcessSuccessfulSubscription(char* topic);

/**
 *  @brief  Gets active status of sensor
 *
 *  @param  sonsor index
 *
 *  @return Sensor active status (1 if active)
 */
char Sensors_ID_GetActiveStatus(unsigned char index);

/**
 *  @brief  Finds device name index from sensor ID string
 *
 *  @param  sensor id string
 *
 *  @return data id of desired sensor
 */
data_id_t Sensors_ID_FindSensorID(char* topic);

/**
 *  @brief  Gets Sensor ID string for desired sensor
 *
 *  Returns pointer to sensor id string stored in sensor list
 *
 *  @param  sonsor index
 *
 *  @return Pointer to sensor ID string
 */
char* Sensors_ID_GetSensorID(unsigned char index);

#endif // SENSORS_SENSID_H_
