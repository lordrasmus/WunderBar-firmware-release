/** @file   Onboarding_Process.c
 *  @brief  File contains functions used for processing messages received during onboarding.
 *  		Processes both messages from WiFi client and master ble.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <stdint.h>
#include <string.h>

#include <hardware/Hw_modules.h>
#include <Common_Defaults.h>

#include "../JSON/JSON_Msg/JSON_Msg.h"
#include "../Sensors/Sensors_main.h"
#include "../Sensors/Sensors_SensID.h"
#include "../Sensors/wunderbar_common.h"
#include "../Sensors/Sensors_Cfg_Handler.h"

#include "Onboarding.h"



	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////


/**
 *  @brief  Process Onboarding configs from WiFi client
 *
 *  Process data received from WiFi client during onboarding process
 *  Configuration data should be sent in JSON in message format :
 *
 *  {
 *	"passkey":{
 *		"htu":"xxxxxx",
 *		"gyro":"xxxxxx",
 *		"light":"xxxxxx",
 *		"microphone":"xxxxxx",
 *		"bridge":"xxxxxx",
 *		"ir":"xxxxxx"
 *	},
 *	"wunderbar":{
 *		"wifi_ssid":"my_wifi_ssid",
 *		"wifi_pass":"my_wifi_pass",
 *		"master_id":[1,2,3,4,5,6,7,8,101,102,103,104,105,106,107,108],
 *		"wunderbar_security":"security0001"
 *	},
 *	"cloud":{
 *		"url":"mqtt.relayr.io"
 *	}
 * }
 *
 *  Received passkeys will be sent to mastel ble device over SPI
 *  and wunderbar and cloud configs will be saved in flash and used later as valid parameters.
 *
 *  @param  pointer to received text message from WiFi client
 *
 *  @return error code for parsed message
 */
unsigned int Onbrd_Process_msg(char* msg){
	ble_pass_t ble_pass;
	char *ptr;
	unsigned int result = 0;

	if (JSON_Msg_Parse(msg) <= 0)
		return CFG_BADJSON_FAILED_MASK;

	// we got JSON msg, we will wait for some more time
	Onbrd_UpdateCurrentProcessTime();
	Onbrd_SetStartProcessFlag();

	memset((void *) &ble_pass, 0, sizeof(ble_pass));

	//  parse ble passkeys form message
	if (JSON_Msg_FindToken(CFG_PASSKEY, 0) > 0)
	{
		if ((ptr = JSON_Msg_GetTokStr(JSON_Msg_FindToken(CFG_HTU, 0))) > 0)
		{	// htu passkey
			strcpy((char *) &ble_pass.pass_htu, (const char *) ptr);
			result |= (unsigned int) CFG_PASS_HTU_MASK;
		}
		if ((ptr = JSON_Msg_GetTokStr(JSON_Msg_FindToken(CFG_GYRO, 0))) > 0)
		{ 	// gyro passkey
			strcpy((char *) &ble_pass.pass_gyro, (const char *) ptr);
			result |= (unsigned int) CFG_PASS_GYRO_MASK;
		}
		if ((ptr = JSON_Msg_GetTokStr(JSON_Msg_FindToken(CFG_LIGHT, 0))) > 0)
		{ 	// light passkey
			strcpy((char *) &ble_pass.pass_light, (const char *) ptr);
			result |= (unsigned int) CFG_PASS_LIGHT_MASK;
		}
		if ((ptr = JSON_Msg_GetTokStr(JSON_Msg_FindToken(CFG_MICROPHONE, 0))) > 0)
		{	// microphone passkey
			strcpy((char *) &ble_pass.pass_mic, (const char *) ptr);
			result |= (unsigned int) CFG_PASS_MICROPHONE_MASK;
		}
		if ((ptr = JSON_Msg_GetTokStr(JSON_Msg_FindToken(CFG_BRIDGE, 0))) > 0)
		{	// bridge passkey
			strcpy((char *) &ble_pass.pass_bridge, (const char *) ptr);
			result |= (unsigned int) CFG_PASS_BRIDGE_MASK;
		}
		if ((ptr = JSON_Msg_GetTokStr(JSON_Msg_FindToken(CFG_IR, 0))) > 0)
		{	// ir passkey
			strcpy((char *) &ble_pass.pass_ir, (const char *) ptr);
			result |= (unsigned int) CFG_PASS_IR_MASK;
		}

		// send configs to master ble
		if (result & CFG_PASS_MASK)
			if (Sensors_Cfg_Upload(&ble_pass) == false)
				result |= CFG_PASS_FAILED_MASK;
	}

	// parse wunderbar configs from message
	if (JSON_Msg_FindToken(CFG_WUNDERBAR, 0) > 0)
	{
		char temp_id_str[37], temp_id[17];

		if ((JSON_Msg_ReadArray(CFG_WUNDERBARID, temp_id)) > 0)
		{	// master module id
			Sensors_ID_FormSensIdStr(temp_id_str, temp_id);
			strcpy((char *) &wunderbar_configuration.wunderbar.id, (const char *) temp_id_str);
			result |= (unsigned int) CFG_WUNDERBAR_ID_MASK;
		}
		if ((ptr = JSON_Msg_GetTokStr(JSON_Msg_FindToken(CFG_WUNDERBARPASS, 0))) > 0)
		{	// master module security
			strcpy((char *) &wunderbar_configuration.wunderbar.security, (const char *) ptr);
			result |= (unsigned int) CFG_WUNDERBAR_PASS_MASK;
		}
		if ((ptr = JSON_Msg_GetTokStr(JSON_Msg_FindToken(CFG_WIFI_SSID, 0))) > 0)
		{	// wifi ssid
			strcpy((char *) &wunderbar_configuration.wifi.ssid, (const char *) ptr);
			result |= (unsigned int) CFG_WIFI_SSID_MASK;
		}
		if ((ptr = JSON_Msg_GetTokStr(JSON_Msg_FindToken(CFG_WIFI_PASS, 0))) > 0)
		{	// wifi password
			strcpy((char *) &wunderbar_configuration.wifi.password, (const char *) ptr);
			result |= (unsigned int) CFG_WIFI_PASS_MASK;
		}
	}

	// parse cloud url from message
	if (JSON_Msg_FindToken(CFG_CLOUD, 0) > 0)
	{
		if ((ptr = JSON_Msg_GetTokStr(JSON_Msg_FindToken(CFG_CLOUD_URL, 0))) > 0)
		{	// mqtt url
			strcpy((char *) &wunderbar_configuration.cloud.url, (const char *) ptr);
			result |= (unsigned int) CFG_CLOUD_URL_MASK;
		}
	}

	return result;
}

/**
 *  @brief  Processes Onboarding configs from master ble module
 *
 *  Process data received from master ble module during Onboarding process.
 *  Parses data received and stores it into wunderbar configs global variable.
 *
 *  @param  index which determine config parameter
 *  @param  pointer to text with config
 *
 *  @return void
 */
void Onbrd_IncomingCfg(char index, char* cfg){
	char temp_id_str[38];

	switch (index)
	{	// received wifi ssid
	case FIELD_ID_CONFIG_WIFI_SSID :
		strcpy((char *) &wunderbar_configuration.wifi.ssid, (const char *) cfg);
		break;

		// received wifi password
	case FIELD_ID_CONFIG_WIFI_PASS :
		strcpy((char *) &wunderbar_configuration.wifi.password, (const char *) cfg);
		break;

		// received master module id
	case FIELD_ID_CONFIG_MASTER_MODULE_ID :
		Sensors_ID_FormSensIdStr(temp_id_str, cfg);
		strcpy((char *) &wunderbar_configuration.wunderbar.id, (const char *) temp_id_str);
		break;

		// received master module security
	case FIELD_ID_CONFIG_MASTER_MODULE_SEC :
		memcpy((void *) &wunderbar_configuration.wunderbar.security, (const void *) cfg, WUNDERBAR_SECURITY_LENGTH);
		wunderbar_configuration.wunderbar.security[WUNDERBAR_SECURITY_LENGTH] = '\0';
		break;

		// received mqtt url
	case FIELD_ID_CONFIG_MASTER_MODULE_URL :
		strcpy((char *) &wunderbar_configuration.cloud.url, (const char *) cfg);
		break;

	default :
		break;
	}

	// we got msg from ble
	Onbrd_UpdateCurrentProcessTime();
	// signal that downloading process has started on ble side
	Onbrd_SetStartProcessFlag();
}
