/** @file   Sensors_gyro.c
 *  @brief  File contains functions handling messages from and to gyro sensor board
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
#define Template_gyro_data "{\"ts\":%s,\"gyro\":{\"x\":%s,\"y\":%s,\"z\":%s},\"accel\":{\"x\":%s,\"y\":%s,\"z\":%s}}"



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
void Sensors_gyro_Update(spi_frame_t* SPI_msg, char* buf){
	char temp_txt1[10];
	char temp_txt2[10];
	char temp_txt3[10];
	char temp_txt4[10];
	char temp_txt5[10];
	char temp_txt6[10];
	char time[30];

	switch (SPI_msg->field_id)
	{
		// post sensor data
	case FIELD_ID_CHAR_SENSOR_DATA_R :
		{
			sensor_gyro_data_t* myData;

			myData = (sensor_gyro_data_t*) &SPI_msg->data[0];

			Sensors_Convert_f_Str(temp_txt1, (int) myData->gyro.x);
			Sensors_Convert_f_Str(temp_txt2, (int) myData->gyro.y);
			Sensors_Convert_f_Str(temp_txt3, (int) myData->gyro.z);

			Sensors_Convert_f_Str(temp_txt4, (int) myData->acc.x);
			Sensors_Convert_f_Str(temp_txt5, (int) myData->acc.y);
			Sensors_Convert_f_Str(temp_txt6, (int) myData->acc.z);

			RTC_GetSystemTimeStr(time);

			sprintf(buf, Template_gyro_data, time, temp_txt1, temp_txt2, temp_txt3,
													temp_txt4, temp_txt5, temp_txt6);
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
int Sensors_gyro_ProcessData(spi_frame_t* SPI_msg, char* msg){
	sensor_gyro_t MyGyroSens;
	int r, result = -1;
	int cnt;

	r = JSON_Msg_Parse(msg);							// parse JSON message

	if (r <= 0)											// if no objects error
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
		if ((result = Sensors_Extract_BeaconFreq(&MyGyroSens.beaconFrequency)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyGyroSens.beaconFrequency, sizeof(beaconFrequency_t));
		break;

		// frequency
	case FIELD_ID_CHAR_SENSOR_FREQUENCY :
		if ((result = Sensors_Extract_Frequency(&MyGyroSens.frequency)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyGyroSens.frequency, sizeof(frequency_t));
		break;

		// led state
	case FIELD_ID_CHAR_SENSOR_LED_STATE :
		if ((result = Sensors_Extract_LedState(&MyGyroSens.led_state)) == 0)
			memcpy((void *) &SPI_msg->data[0], (const void *) &MyGyroSens.led_state, sizeof(led_state_t));
		break;

		// threshold
	case FIELD_ID_CHAR_SENSOR_THRESHOLD :
		{
			threshold_float_t f_temp;
			// extract gyroscope threshold
			cnt = JSON_Msg_FindToken(JSON_MSG_GYRO, 0);       // find gyro string
			if (cnt > 0)
			{
				if (Sensors_FloatReadThreshould(cnt, &f_temp) == 0)
				{
					Sensors_ConvertFloat_2_int32(f_temp, &MyGyroSens.threshold.gyro);
					result = 0;
				}
			}
			// extract accelerometer threshold
			cnt = JSON_Msg_FindToken(JSON_MSG_ACCEL, 0);       // find accel string
			if (cnt > 0)
			{
				if (Sensors_FloatReadThreshould(cnt, &f_temp) == 0)
					Sensors_ConvertFloat_2_int16(f_temp, &MyGyroSens.threshold.acc);
				else
					result = -1;
			}
			else
				result = -1;

			memcpy((void *) &SPI_msg->data[0], (const void *) &MyGyroSens.threshold, sizeof(sensor_gyro_threshold_t));
		}
		break;

		// configuration
	case FIELD_ID_CHAR_SENSOR_CONFIG :

		if ((cnt = JSON_Msg_FindToken(JSON_MSG_CONFIG, 0)) > 0)
		{
			int c, temp;
			// extract accel config
			if ((c = JSON_Msg_FindToken(JSON_MSG_ACCEL, 0)) > 0)
			{
				if (Sensors_JSON_ReadSingleIntValue(JSON_MSG_RANGE, c, &temp) == 0)
				{
					MyGyroSens.config.acc_full_scale = (sensor_gyro_AccFullScale_t) temp;
					result = 0;
				}
			}
			// extract gyro config
			if ((c = JSON_Msg_FindToken(JSON_MSG_GYRO, 0)) > 0)
			{
				if (Sensors_JSON_ReadSingleIntValue(JSON_MSG_RANGE, c, &temp) == 0)
					MyGyroSens.config.gyro_full_scale = (sensor_gyro_GyroFullScale_t) temp;
				else
					result = -1;
			}
			else
				result = -1;
		}

		memcpy((void *) &SPI_msg->data[0], (const void *) &MyGyroSens.config, sizeof(sensor_gyro_config_t));
		break;

		// firmware and hardware revision
	case FIELD_ID_CHAR_HARDWARE_REVISION :
	case FIELD_ID_CHAR_FIRMWARE_REVISION :
		result = 0;
		SPI_msg->operation = OPERATION_READ;
		break;

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
