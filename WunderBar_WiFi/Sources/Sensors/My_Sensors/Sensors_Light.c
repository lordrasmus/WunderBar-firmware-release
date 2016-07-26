/** @file   Sensors_light.c
 *  @brief  File contains functions handling messages from and to light sensor board
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
#define Template_light_data "{\"ts\":%s,\"light\":%d,\"clr\":{\"r\":%d,\"g\":%d,\"b\":%d},\"prox\":%d}"


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
void Sensors_light_Update(spi_frame_t* SPI_msg, char* buf){
	char temp_txt1[5];
	char time[30];

	switch (SPI_msg->field_id)
	{
		// post sensor data
	case FIELD_ID_CHAR_SENSOR_DATA_R :
		{
			sensor_lightprox_data_t* myData;

			myData = (sensor_lightprox_data_t*) &SPI_msg->data[0];

			RTC_GetSystemTimeStr(time);

			sprintf(buf, Template_light_data, time, (int) myData->white,
													 (int) myData->r, (int) myData->g, (int) myData->b,
												      (int) myData->proximity);
		}
		break;

		// post battery level
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
int Sensors_light_ProcessData (spi_frame_t* SPI_msg, char* msg){
	sensor_lightprox_t MyLightSens;
	int r, result = -1;
	int cnt;

	r = JSON_Msg_Parse(msg);						// parse JSON message

	if (r == 0)										// if no objects error
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
		if ((result = Sensors_Extract_BeaconFreq(&MyLightSens.beaconFrequency)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyLightSens.beaconFrequency, sizeof(beaconFrequency_t));
		break;

		// frequency
	case FIELD_ID_CHAR_SENSOR_FREQUENCY :
		if ((result = Sensors_Extract_Frequency(&MyLightSens.frequency)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyLightSens.frequency, sizeof(frequency_t));
		break;

		// led state
	case FIELD_ID_CHAR_SENSOR_LED_STATE :
		if ((result = Sensors_Extract_LedState(&MyLightSens.led_state)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyLightSens.led_state, sizeof(led_state_t));
		break;

		// threshold
	case FIELD_ID_CHAR_SENSOR_THRESHOLD :
		// extract light threshold
		cnt = JSON_Msg_FindToken(JSON_MSG_LIGHT, 0);       // find light string
		if (cnt > 0)
		{
			if (Sensors_IntReadThreshold(cnt, &MyLightSens.threshold.white) == 0)
				result = 0;
		}
		// extract proximity threshold
		cnt = JSON_Msg_FindToken(JSON_MSG_PROX, 0);       // find proxy string
		if (cnt > 0)
		{
			if (Sensors_IntReadThreshold(cnt, &MyLightSens.threshold.proximity) != 0)
				result = -1;
		}
		else
			result = -1;

		memcpy((void *) &SPI_msg->data[0], (const void *) &MyLightSens.threshold, sizeof(sensor_lightprox_threshold_t));

		break;

		// sensor config
	case FIELD_ID_CHAR_SENSOR_CONFIG :
		// extract config
		if ((cnt = JSON_Msg_FindToken(JSON_MSG_CONFIG, 0)) > 0)
		{
			int temp;

			Sensors_JSON_ReadSingleIntValue(JSON_MSG_RGBC_GAIN, cnt, (int*) &temp);
			MyLightSens.config.rgbc_gain = (sensor_lightprox_rgbc_gain_t) temp;
			Sensors_JSON_ReadSingleIntValue(JSON_MSG_PROX_DRIVE, cnt, (int*) &temp);
			MyLightSens.config.prox_drive = (sensor_lightprox_prox_drive_t) temp;

			result = 0;
		}

		memcpy((void *) &SPI_msg->data[0], (const void *) &MyLightSens.config, sizeof(sensor_lightprox_config_t));
		break;

		// firmware and hardware revision
	case FIELD_ID_CHAR_FIRMWARE_REVISION :
	case FIELD_ID_CHAR_HARDWARE_REVISION :
		result = 0;
		SPI_msg->operation = OPERATION_READ;
		break;

		// unhandled characteristics
	case FIELD_ID_CHAR_SENSOR_ID :
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

	if (result == -1)								// if failed
		Sensors_JSON_DiscardMsgId();

	return result;    // success

}
