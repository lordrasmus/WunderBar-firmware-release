/** @file   Sensors_htu.c
 *  @brief  File contains functions handling messages from and to htu sensor board
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
#define Template_htu_data   "{\"ts\":%s,\"temp\":%s,\"hum\":%s}"


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
void Sensors_htu_Update(spi_frame_t* SPI_msg, char* buf){
	char text1[8];
	char text2[8];
	char time[30];

	switch (SPI_msg->field_id)
	{
	case FIELD_ID_CHAR_SENSOR_DATA_R :
		{
			sensor_htu_data_t* myData;

			myData = (sensor_htu_data_t*) &SPI_msg->data[0];

			Sensors_Convert_f_Str(text1, (int) myData->temperature);
			Sensors_Convert_f_Str(text2, (int) myData->humidity);

			RTC_GetSystemTimeStr(time);
			sprintf(buf, Template_htu_data, time, text1, text2);
		}
		break;

	case FIELD_ID_CHAR_BATTERY_LEVEL :
		{
			char myData;

			myData = (char) SPI_msg->data[0];
			sprintf(text1, "%d", (int) myData);
			RTC_GetSystemTimeStr(time);
			sprintf(buf, Template_battery_level, time, text1);
		}
		break;

	case FIELD_ID_SENSOR_STATUS :
		Sensors_ID_Process((char*) &SPI_msg->data[0], SPI_msg->data_id, SPI_msg->operation);
		break;

	case FIELD_ID_CHAR_FIRMWARE_REVISION :

		Sensors_FormFrmHwRevStr((char* ) &SPI_msg->data[0]);
		RTC_GetSystemTimeStr(time);
		sprintf(buf, Template_firmware_rev, time, (char* ) &SPI_msg->data[0]);

		break;

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
int Sensors_htu_ProcessData(spi_frame_t* SPI_msg, char* msg){
	sensor_htu_t MyHtuSens;
	int r, result = -1;
	int cnt;

	r = JSON_Msg_Parse(msg);						// parse JSON message

 	if (r <= 0)										// if no JSON objects error
	{
		return result;
	}

	if (Sensors_JSON_StoreMsgId() == 0)				// store msg_id
		return result;

	// process characteristic
	switch (SPI_msg->field_id)
	{
		// beacon freq
	case FIELD_ID_CHAR_SENSOR_BEACON_FREQUENCY :
		if ((result = Sensors_Extract_BeaconFreq(&MyHtuSens.beaconFrequency)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyHtuSens.beaconFrequency, sizeof(beaconFrequency_t));
		break;

		// frequency
	case FIELD_ID_CHAR_SENSOR_FREQUENCY :
		if ((result = Sensors_Extract_Frequency(&MyHtuSens.frequency)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyHtuSens.frequency, sizeof(frequency_t));
		break;

		// led state
	case FIELD_ID_CHAR_SENSOR_LED_STATE :
		if ((result = Sensors_Extract_LedState(&MyHtuSens.led_state)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyHtuSens.led_state, sizeof(led_state_t));
		break;

		// threshold
	case FIELD_ID_CHAR_SENSOR_THRESHOLD :
		{
			threshold_float_t f_temp;
			// extract temperature threshold
			cnt = JSON_Msg_FindToken(JSON_MSG_TEMPERATURE, 0);  	// find temperature string
			if (cnt > 0)
			{
				if (Sensors_FloatReadThreshould(cnt, &f_temp) == 0)
				{
					Sensors_ConvertFloat_2_int16(f_temp, &MyHtuSens.threshold.temperature);
					result = 0;
				}
			}
			// extract humidity threshold
			cnt = JSON_Msg_FindToken(JSON_MSG_HUMIDITY, 0); 		// find humidity string
			if (cnt > 0)
			{
				if (Sensors_FloatReadThreshould(cnt, &f_temp) == 0)
					Sensors_ConvertFloat_2_int16(f_temp, &MyHtuSens.threshold.humidity);
				else
					result = -1;
			}
			else
				result = -1;

			memcpy((void *) &SPI_msg->data[0], (const void *) &MyHtuSens.threshold, sizeof(sensor_htu_threshold_t));
		}
		break;

		// configuration
	case FIELD_ID_CHAR_SENSOR_CONFIG :
		{
			int temp;

			if (JSON_Msg_FindToken(JSON_MSG_CONFIG, 0) > 0)
			{
				if (Sensors_JSON_ReadSingleIntValue(JSON_MSG_RESOLUTION, 0, &temp) == 0)
				{
					MyHtuSens.config = (sensor_htu_config_t) temp;
					result = 0;
				}
			}

			memcpy((void *) &SPI_msg->data[0], (const void *) &MyHtuSens.config, sizeof(sensor_htu_config_t));
		}
		break;

		// firmware and hardware revision
	case FIELD_ID_CHAR_FIRMWARE_REVISION :
	case FIELD_ID_CHAR_HARDWARE_REVISION :
		result = 0;
		SPI_msg->operation = OPERATION_READ;
		break;

		// unhandled characteristics
	case FIELD_ID_CHAR_SENSOR_DATA_R :
	case FIELD_ID_CHAR_SENSOR_DATA_W :
	case FIELD_ID_CHAR_BATTERY_LEVEL :
	case FIELD_ID_CHAR_MANUFACTURER_NAME :
	case FIELD_ID_SENSOR_STATUS :
		result = -1;
		break;

	default :
		result = -1;
	}

	if (result == -1)							// if failed
		Sensors_JSON_DiscardMsgId();

	return result;    // success
}
