/** @file   Sensors_sound.c
 *  @brief  File contains functions handling messages from and to sound sensor board
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
 *  message on mqtt server will be in this format
 */
#define Template_sound_data "{\"ts\":%s,\"snd_level\":%d}"



/*
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
void Sensors_sound_Update(spi_frame_t* SPI_msg, char* buf){
	char temp_txt1[5];
	char time[30];

	switch (SPI_msg->field_id)
	{
	case FIELD_ID_CHAR_SENSOR_DATA_R :
		{
			sensor_microphone_data_t* myData;

			myData = (sensor_microphone_data_t*) &SPI_msg->data[0];

			RTC_GetSystemTimeStr(time);

			sprintf(buf, Template_sound_data, time, (int) myData->mic_level);
		}
		break;

	case FIELD_ID_CHAR_BATTERY_LEVEL :
		{
			char myData;

			myData = (char) SPI_msg->data[0];
			sprintf(temp_txt1, "%d", (int) myData);
			RTC_GetSystemTimeStr(time);
			sprintf(buf, Template_battery_level, time, temp_txt1);
		}
		break;

		// process sensor status
	case FIELD_ID_SENSOR_STATUS :
		Sensors_ID_Process((char*) &SPI_msg->data[0], SPI_msg->data_id, SPI_msg->operation);
		break;

		// post firmware revision
	case FIELD_ID_CHAR_FIRMWARE_REVISION :

		Sensors_FormFrmHwRevStr((char* ) &SPI_msg->data[0]);
		RTC_GetSystemTimeStr(time);
		sprintf(buf, Template_firmware_rev, time, (char* ) &SPI_msg->data[0]);

		break;

		// post hardware revision
	case FIELD_ID_CHAR_HARDWARE_REVISION :

		Sensors_FormFrmHwRevStr((char* ) &SPI_msg->data[0]);
		RTC_GetSystemTimeStr(time);
		sprintf(buf, Template_hardware_rev, time, (char* ) &SPI_msg->data[0]);

		break;

	default :
		break;

	}
}

/*
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
int Sensors_sound_ProcessData(spi_frame_t* SPI_msg, char* msg){
	sensor_microphone_t MyMicSens;
	int r, result = -1;

	r = JSON_Msg_Parse(msg);							// parse JSON message

	if (r == 0)											// if no JSON objects error
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
		if ((result = Sensors_Extract_BeaconFreq(&MyMicSens.beaconFrequency)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyMicSens.beaconFrequency, sizeof(beaconFrequency_t));
		break;

		// frequency
	case FIELD_ID_CHAR_SENSOR_FREQUENCY :
		if ((result = Sensors_Extract_Frequency(&MyMicSens.frequency)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyMicSens.frequency, sizeof(frequency_t));
		break;

		// led state
	case FIELD_ID_CHAR_SENSOR_LED_STATE :
		if ((result = Sensors_Extract_LedState(&MyMicSens.led_state)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyMicSens.led_state, sizeof(led_state_t));
		break;

		// threshold
	case FIELD_ID_CHAR_SENSOR_THRESHOLD :
		{
			threshold_int16_t int16_temp;

			if (Sensors_IntReadThreshold(0, &int16_temp) == 0)
				result = 0;

			memcpy((void *) &SPI_msg->data[0], (const void *) &int16_temp, sizeof(sensor_microphone_threshold_t));
		}
		break;

		// firmware and hardware revision
	case FIELD_ID_CHAR_HARDWARE_REVISION :
	case FIELD_ID_CHAR_FIRMWARE_REVISION :
		result = 0;
		SPI_msg->operation = OPERATION_READ;
		break;

		// unhandled characteristics
	case FIELD_ID_CHAR_SENSOR_DATA_R :
	case FIELD_ID_CHAR_SENSOR_DATA_W :
	case FIELD_ID_CHAR_BATTERY_LEVEL :
	case FIELD_ID_CHAR_MANUFACTURER_NAME :
	case FIELD_ID_SENSOR_STATUS :
	case FIELD_ID_CHAR_SENSOR_CONFIG :
		result = -1;
		break;

	default :
		result = -1;
	}

	if (result == -1)								// if failed
		Sensors_JSON_DiscardMsgId();

	return result;    // success
}

