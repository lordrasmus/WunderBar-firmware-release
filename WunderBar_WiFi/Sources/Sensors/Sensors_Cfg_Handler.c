/** @file   Sensors_Cfg_Handler.c
 *  @brief  File contains functions for handling config messages between kinetis and
 *  		master ble during onboarding process. Handles messages in both directions.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <stdbool.h>
#include <string.h>

#include <hardware/Hw_modules.h>
#include "Common_Defaults.h"
#include "wunderbar_common.h"
#include "Sensors_main.h"
#include "Sensors_Cfg_Handler.h"
#include "../Onboarding/Onboarding.h"


// static declarations

static bool sensors_ack_rcv;

static void Sensors_Cfg_SetAck();
static void Sensors_Cfg_ClrAck();
static bool Sensors_Cfg_WaitAck();
static bool Sensors_Cfg_Send(char index, char* pass);



///////////////////////////////////////
/*         public functions          */
///////////////////////////////////////



/**
 *  @brief  Send config parameters to master ble
 *
 *  Passkeys that are received from wifi should be sent to master ble module.
 *
 *  @param  ble passkeys struct
 *
 *  @return True if all successful, false if at least one failed
 */
bool Sensors_Cfg_Upload(ble_pass_t* ble_pass){
	// htu passkey
	if (ble_pass->pass_htu[0] != 0)
		if (Sensors_Cfg_Send(FIELD_ID_CONFIG_HTU_PASS, 		(char *) &ble_pass->pass_htu) 	== false)
			return false;
	// gyro passkey
	if (ble_pass->pass_gyro[0] != 0)
		if (Sensors_Cfg_Send(FIELD_ID_CONFIG_GYRO_PASS, 	(char *) &ble_pass->pass_gyro)	== false)
			return false;
	// light passkey
	if (ble_pass->pass_light[0] != 0)
		if (Sensors_Cfg_Send(FIELD_ID_CONFIG_LIGHT_PASS, 	(char *) &ble_pass->pass_light) == false)
			return false;
	// microphone passkey
	if (ble_pass->pass_mic[0] != 0)
		if (Sensors_Cfg_Send(FIELD_ID_CONFIG_SOUND_PASS, 	(char *) &ble_pass->pass_mic) 	== false)
			return false;
	// bridge passkey
	if (ble_pass->pass_bridge[0] != 0)
		if (Sensors_Cfg_Send(FIELD_ID_CONFIG_BRIDGE_PASS,   (char *) &ble_pass->pass_bridge)== false)
			return false;
	// ir passkey
	if (ble_pass->pass_ir[0] != 0)
		if (Sensors_Cfg_Send(FIELD_ID_CONFIG_IR_PASS, 		(char *) &ble_pass->pass_ir) 	== false)
			return false;

	return true;
}

/**
 *  @brief  Process incoming config message from master ble
 *
 *  Configs are received by SPI in SPI frame format.
 *  Should be processed and stored wunderbar configuration global variable.
 *
 *  @param  SPI frame
 *
 *  @return void
 */
void Sensors_Cfg_ProcessBleMsg(spi_frame_t* spi_msg){
	switch (spi_msg->field_id)
	{
	case FIELD_ID_CONFIG_ACK :
		// we have received ack on our config write
		Sensors_Cfg_SetAck();
		break;

	case FIELD_ID_CONFIG_WIFI_SSID :
	case FIELD_ID_CONFIG_WIFI_PASS :
	case FIELD_ID_CONFIG_MASTER_MODULE_ID :
	case FIELD_ID_CONFIG_MASTER_MODULE_SEC :
	case FIELD_ID_CONFIG_MASTER_MODULE_URL :
		// process new config message from ble
		Onbrd_IncomingCfg(spi_msg->field_id, (char *) &spi_msg->data[0]);
		break;

		// completed config
	case FIELD_ID_CONFIG_COMPLETE :
		// we have received complete message from master ble
		Onbrd_MasterBleReceived();
		break;

	default :
		break;
	}
}




///////////////////////////////////////
/*         static functions          */
///////////////////////////////////////


/**
 *  @brief  Set acknowledge flag
 *
 *  Set ble acknowledge flag. After passkey is sent to master ble,
 *  it will acknowledge it after processing.
 *
 *  @return void
 */
static void Sensors_Cfg_SetAck(){
	sensors_ack_rcv = true;
}

/**
 *  @brief  Clear acknowledge flag
 *
 *  Clear ble acknowledge flag.
 *
 *  @return void
 */
static void Sensors_Cfg_ClrAck(){
	sensors_ack_rcv = false;
}

/**
 *  @brief  Wait until we receive ack from master ble module
 *
 *  After sending SPI message to master ble we should wait
 *  for acknowledge or timeout.
 *
 *  @return True if ack received
 */
static bool Sensors_Cfg_WaitAck(){
	unsigned long long timeout;

	timeout = MSTimerGet();
	Sensors_Cfg_ClrAck();

	while (sensors_ack_rcv == false)
	{
		if (MSTimerDelta(timeout) > CFG_PASSKEY_WRITE_TIMEOUT)
			return false;
	}

	return sensors_ack_rcv;
}

/**
 *  @brief  Send new config to the master ble device
 *
 *  @param  Sensor index
 *  @param  sensor passkey
 *
 *  @return True if successful
 */
static bool Sensors_Cfg_Send(char index, char* pass){
	spi_frame_t spi_msg;
	int cnt = 1000;

	// small delay
	while(cnt --)
		;

	spi_msg.data_id   = DATA_ID_CONFIG;
	spi_msg.field_id  = index;
	spi_msg.operation = OPERATION_READ;

	strcpy((char *) &spi_msg.data[0], (const char *) pass);

	Sensors_SPI_SendMsg(&spi_msg);

	return (Sensors_Cfg_WaitAck());
}
