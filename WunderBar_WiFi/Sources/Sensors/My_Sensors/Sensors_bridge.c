/** @file   Sensors_bridge.c
 *  @brief  File contains functions handling messages from and to bridge sensor board
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <stdio.h>
#include "Sensors_common.h"
#include "../Sensors_SensID.h"
#include <hardware/Hw_modules.h>



/*
 *  Template for JSON message response
 *  Message on mqtt server will be in this format
 */
#define Template_bridge_data "{\"ts\":%s,\"up_ch_payload\":[%s]}"




/**
 *  @brief  Creates string array from array of bytes
 *
 *  Generate array of bytes. Needed to generate json array for response
 *
 *  @param  bridge sensor data
 *  @param  Pointer to text for return string
 *
 *  @return void
 */
void Sensors_FormBridgeArray(sensor_bridge_data_t* bridgeData, char* txt){
	char* ptr = txt;
	int i;

	for (i = 0; i < bridgeData->payload_length; i ++)
	{
		sprintf(ptr, "%d,", (int) bridgeData->payload[i]);
		ptr = strchr(ptr, ',');
		ptr ++;
	}
	*(ptr - 1) = 0;
}

/**
 *  @brief  Process incoming data from ble module
 *
 *  Processes data from ble module and prepare packet in json format for mqtt server
 *  Result data will be formed in json string format for publishing.
 *
 *  @param  Spi message frame
 *  @param  Pointer to buffer where to pack json formatted string
 *
 *  @return void
 */
void Sensors_bridge_Update(spi_frame_t* SPI_msg, char* buf){
	char temp_txt1[80];
	char time[30];

	switch (SPI_msg->field_id)
	{
		// post sensor data
	case FIELD_ID_CHAR_SENSOR_DATA_R :
		{
			sensor_bridge_data_t* myData;

			myData = (sensor_bridge_data_t*) &SPI_msg->data[0];

			RTC_GetSystemTimeStr(time);

			Sensors_FormBridgeArray(myData, temp_txt1);
			sprintf(buf, Template_bridge_data, time, (char *) temp_txt1);
		}
		break;

		// process sensor status
	case FIELD_ID_SENSOR_STATUS :
		Sensors_ID_Process((char *) &SPI_msg->data[0], SPI_msg->data_id, SPI_msg->operation);
		break;

		// post firmware revision
	case FIELD_ID_CHAR_FIRMWARE_REVISION :

		Sensors_FormFrmHwRevStr((char *) &SPI_msg->data[0]);
		RTC_GetSystemTimeStr(time);
		sprintf(buf, Template_firmware_rev, time, (char *) &SPI_msg->data[0]);

		break;

		// post hardware revision
	case FIELD_ID_CHAR_HARDWARE_REVISION :

		Sensors_FormFrmHwRevStr((char *) &SPI_msg->data[0]);
		RTC_GetSystemTimeStr(time);
		sprintf(buf, Template_hardware_rev, time, (char *) &SPI_msg->data[0]);

		break;

	default :
		break;
	}
}


/**
 *  @brief  Process incoming message from mqtt.
 *
 *  Process data received from mqtt server and prepare it in format for master ble module.
 *  Data will be packet in appropriate SPI frame
 *
 *  @param  Spi message frame
 *  @param  Pointer to json text message
 *
 *  @return Error code: 0 - OK;  -1 - fail
 */
int Sensors_bridge_ProcessData(spi_frame_t* SPI_msg, char* msg){
	sensor_bridge_t MyBridgeSens;
	int r, result = -1;

	r = JSON_Msg_Parse(msg);							// parse JSON message

	if (r <= 0)											// if no JSON objects error
	{
		return result;
	}

	if (Sensors_JSON_StoreMsgId() == 0)					// store msg_id
		return result;

	// process characteristic
	switch (SPI_msg->field_id)
	{
		// beacon freq
	case FIELD_ID_CHAR_SENSOR_BEACON_FREQUENCY :
		if ((result = Sensors_Extract_BeaconFreq(&MyBridgeSens.beaconFrequency)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyBridgeSens.beaconFrequency, sizeof(beaconFrequency_t));
		break;

		// led state
	case FIELD_ID_CHAR_SENSOR_LED_STATE :
		if ((result = Sensors_Extract_LedState(&MyBridgeSens.led_state)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyBridgeSens.led_state, sizeof(led_state_t));
		break;

		// sensor data down
	case FIELD_ID_CHAR_SENSOR_DATA_W :
		r = JSON_Msg_ReadArray(JSON_MSG_DOWN_BRIDGE, (char *) &MyBridgeSens.data_down.payload);
		MyBridgeSens.data_down.payload_length = r;
		if (r > 0)
		{
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyBridgeSens.data_down, r + 1);
			result = 0;
		}
		break;

		// configure baudrate for bridge sensor
	case FIELD_ID_CHAR_SENSOR_CONFIG :
		{
			int temp;

			if (Sensors_JSON_ReadSingleIntValue(JSON_MSG_BAUDRATE, 0, &temp) == 0)
			{
				MyBridgeSens.config.baud_rate = (uint32_t) temp;
				memcpy((void *) &SPI_msg->data[0], (const void *) &MyBridgeSens.config.baud_rate, sizeof(uint32_t));
				result = 0;
			}
		}
		break;

		// firmware and hardware revision
	case FIELD_ID_CHAR_HARDWARE_REVISION :
	case FIELD_ID_CHAR_FIRMWARE_REVISION :
		result = 0;
		SPI_msg->operation = OPERATION_READ;
		break;

		// invalid characteristics
	case FIELD_ID_CHAR_SENSOR_FREQUENCY :
	case FIELD_ID_CHAR_SENSOR_THRESHOLD :
	case FIELD_ID_CHAR_SENSOR_DATA_R :
	case FIELD_ID_CHAR_BATTERY_LEVEL :
	case FIELD_ID_CHAR_MANUFACTURER_NAME :
	case FIELD_ID_SENSOR_STATUS :
		result = -1;
		break;

	default :
		result = -1;
	}

	if (result == -1)
		Sensors_JSON_DiscardMsgId();

	return result;    // success
}
