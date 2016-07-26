/** @file   Sensors_mainboard.c
 *  @brief  File contains functions handling messages from and to mainboard
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */


#include <stdio.h>
#include "Sensors_common.h"
#include <hardware/Hw_modules.h>
#include "Common_Defaults.h"

static char FirmwareRev[20];       				// firmware revision of the master ble module. This string is sent by master ble on power up

void MainBoard_Update_FwRev(spi_frame_t* SPI_msg);


/*
 *  Template for JSON message response
 *  message on mqtt server will be in this format
 */
#define Template_main_firmware_rev   "{\"ts\":%s,\"kinetis\":\"%s\",\"master ble\":\"%s\"}"
#define Template_main_hardware_rev   "{\"ts\":%s,\"hardware\":\"%s\"}"

/**
 *  @brief  Get ble firmware revision string
 *
 *  @return pointer to ble firmware version string
 */
char* Sensors_GetBleFirmRevStr(){
	return ((char *) FirmwareRev);
}


/**
 *  @brief  Prepare firmware or hardware rev of main board
 *
 *  Prepare packet in json format for mqtt server
 *  Result data will be formed in json string format for publishing.
 *
 *  @param  Spi message frame
 *  @param  Pointer to buffer where to pack json formatted string
 *
 *  @return void
 */
void MainBoard_Update(spi_frame_t* SPI_msg, char* buf){

	char time[30];

	switch (SPI_msg->field_id)
	{
		// post firmware revision
	case FIELD_ID_CHAR_FIRMWARE_REVISION :

		RTC_GetSystemTimeStr(time);
		sprintf(buf, Template_main_firmware_rev, time, KINETIS_FIRMWARE_REV, FirmwareRev);

		break;

		// post hardware revision
	case FIELD_ID_CHAR_HARDWARE_REVISION :

		RTC_GetSystemTimeStr(time);
		sprintf(buf, Template_main_hardware_rev, time, MAIN_BOARD_HW_REV);

		break;

	default :
		break;
	}
}

/*
 *  @brief  Process incoming message from mqtt.
 *
 *  Process data received from mqtt server and prepare response.
 *  Data will be packet in appropriate SPI frame
 *
 *  @param  Spi message frame
 *  @param  Pointer to json text message
 *
 *  @return Error code: 0 - OK;  -1 - fail
 */
int MainBoard_ProcessData(spi_frame_t* SPI_msg, char* msg){
	if (SPI_msg->field_id == FIELD_ID_CHAR_FIRMWARE_REVISION || SPI_msg->field_id == FIELD_ID_CHAR_HARDWARE_REVISION)
	{
		SPI_msg->operation = OPERATION_READ;
		MainBoard_Update_FwRev(SPI_msg);
	}

	return -1;
}
