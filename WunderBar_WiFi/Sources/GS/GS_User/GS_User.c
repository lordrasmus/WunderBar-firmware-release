/** @file   GS_User.c
 *  @brief  File contains functions to handle GS module states.
 *  		Processes connection on WiFi network and communication over
 *  		tcp secured socket with mqtt server.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <User_init.h>
#include <Common_Defaults.h>
#include <hardware/Hw_modules.h>

#include "../AT/AtCmdLib.h"
#include "GS_User.h"
#include "GS_Limited_AP.h"
#include "GS_Certificate.h"
#include "GS_Http.h"
#include "GS_Api_TCP.h"
#include "GS_TCP_mqtt.h"
#include "../../MQTT/MQTT_API_Client/MQTT_Api.h"
#include "../../Onboarding/Onboarding.h"
#include "../../Sensors/Sensors_main.h"



// static declarations

static struct RepeatCounter_t {
	unsigned long long int 	Time;
	char          		    Cnt;
} RepeatCounter;


static MainState_t MainState = GS_MAIN_STATE_INIT;
static char GS_LimitedAP_mode_flag;
static HOST_APP_NETWORK_CONFIG_T apiNetworkConfig;


static bool GS_User_Join_Network();
static bool GS_User_StartTCPTask();
static bool GS_DNS_Resolve(char* url, char* ip);
static void GS_ForgetServerIp();
static void GS_Load_Network_Parameters(HOST_APP_NETWORK_CONFIG_T *NetConf);
static bool GS_DNS_Resolve(char* url, char* ip);
static char GS_RepeatCounter_GetCnt();
static void GS_RepeatCounter_UpdateTime();
static bool GS_Wait();
static bool GS_Timeout(unsigned long long int timeout);
static void GS_User_SM_SetState(MainState_t state);
static void GS_SetLeds(bool led1, bool led2);
static void GS_User_SetSystemTime(void);




	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////



/**
*  @brief  Gain Span main state machine
*
*  Processes WiFi network and connections.
*  Should be called frequently from low priority int or main loop
*
*  @return void
*/
void GS_Main_StateMachine(){

	GS_API_CheckForData();

	switch (MainState){
	// ------------------------------------------------------------------------------------ //
	// -------------initialize module and load default parameters-------------------------- //
	case GS_MAIN_STATE_INIT :

		Sleep_Restore_Countdown();

		if (Chec_Wifi_Rst_Stable() == 0)								// check if wifi module is up and running
		{
			return;
		}

		GS_API_Init();													// init GS module

		// after wifi init jump to ap mode if requested or if the device is blank (not configured)
		if (( GS_LimitedAP_mode_flag ) || (Check_MainBoard_ID_Exists(&wunderbar_configuration) == 0))
		{
			Onbrd_GoToStart();
			GS_LimitedAP_mode_flag = 0;
			GS_User_SM_SetState( GS_MAIN_STATE_LIMITED_AP );
			return;
		}

		GS_SetLeds(true, true);											// Turn on leds

		GS_Load_Network_Parameters(&apiNetworkConfig);					// Load default parameters for wifi network
		GS_API_SetupWifiNetwork(&apiNetworkConfig);						// Set up network parameters
		MQTT_Api_ResetMqtt(true);										// reset mwtt stack

		GS_User_SM_SetState( GS_MAIN_STATE_TRY_TO_CONNECT );			// go to next state

		Sleep_Restore_Countdown();

		break;

	// ------------------------------------------------------------------------------------ //
	// ------------------ attempt association to wifi network ----------------------------- //
	case GS_MAIN_STATE_TRY_TO_CONNECT :

		if (GS_Wait())
		{
			if (GS_RepeatCounter_GetCnt() > GS_NUMBER_OF_RETRIES)		// if max retries, reset
				CPU_System_Reset();

			GS_HAL_ClearBuff();
			GS_SetLeds(true, true);

			if (GS_User_Join_Network() == true)							// Associate on wifi network
			{
				GS_SetLeds(false, true);								// set leds
				GS_User_SM_SetState( GS_MAIN_STATE_GET_SERVER_TIME );	// go to the next state
			}

			GS_RepeatCounter_UpdateTime();
		}

		Sleep_Restore_Countdown();

		break;

	// ------------------------------------------------------------------------------------ //
	// -------------------- ping mqtt server and obtain time ------------------------------ //
	case GS_MAIN_STATE_GET_SERVER_TIME :

		if (GS_Timeout(GS_TRY_INTERVAL))
		{
			// if max retries, reset
			if (GS_RepeatCounter_GetCnt() > GS_NUMBER_OF_RETRIES)
				CPU_System_Reset();

			// if we do not have ip, do dns look up
			if (wunderbar_configuration.cloud.ip[0] == 0xFF)
				if (GS_DNS_Resolve((char *) wunderbar_configuration.cloud.url, (char *) wunderbar_configuration.cloud.ip) == false)
					return;

			// ping server to get response with timestamp
			if (GS_Http_Get((char *) wunderbar_configuration.cloud.ip, MQTT_RELAYR_SERVER_PING_PORT, MQTT_RELAYR_SERVER_PING_ADDRESS) == true)
			{
				GS_User_SM_SetState( GS_MAIN_STATE_WAIT_SERVER_TIME );
			}

			GS_RepeatCounter_UpdateTime();
		}

		Sleep_Restore_Countdown();

		break;

	// ------------------------------------------------------------------------------------ //
	// --------------------- wait server time and set system time ------------------------- //
	case GS_MAIN_STATE_WAIT_SERVER_TIME :
		// if timeout, we should reset
		if (GS_Timeout(GS_WAIT_TIMEOUT))
		{
			GS_Http_CloseConn();
			CPU_System_Reset();
		}

		// if time loaded successfully go to next state
		if (GS_Http_LoadTime())
		{
			GS_User_SetSystemTime();				// set system time with new time obtained from web site

#ifdef __SSL__

			GS_User_SM_SetState( GS_MAIN_STATE_CHECK_CERT );
#else

			GS_User_SM_SetState( GS_MAIN_STATE_SWICH_TO_CLIENT_MODE );
#endif
		}

		Sleep_Restore_Countdown();

		break;

	// ------------------------------------------------------------------------------------ //
	// ------------------- get certificate with http get request -------------------------- //
	case GS_MAIN_STATE_GET_CACERT :

		Sleep_Restore_Countdown();

		if (GS_Timeout(GS_TRY_INTERVAL))					// try on every GS_TRY_INTERVAL miliseconds
		{
			// reset if max retries
			if (GS_RepeatCounter_GetCnt() > GS_NUMBER_OF_RETRIES)
				CPU_System_Reset();

			// get certificate from server
			if (GS_Http_Get((char *) wunderbar_configuration.cloud.ip, MQTT_RELAYR_SERVER_GET_CERT_PORT, MQTT_RELAYR_SERVER_GET_CERT_ADDRESS))
			{
				GS_User_SM_SetState( GS_MAIN_STATE_WAIT_CACERT );         	// go to next state
			}

			GS_RepeatCounter_UpdateTime();									// update time of last action
		}
		break;

	// ------------------------------------------------------------------------------------ //
	// --------------------- wait for the certificate ------------------------------------- //
	case GS_MAIN_STATE_WAIT_CACERT :

		Sleep_Restore_Countdown();

		if (GS_Timeout(GS_WAIT_TIMEOUT))   			// if timeout, we should reset
		{
			GS_Http_CloseConn();
			CPU_System_Reset();
		}

		if (GS_Http_DownloadCert())			// if certificate downloaded successfully from site, we should reset
		{
			CPU_System_Reset();
		}
		break;

	// ------------------------------------------------------------------------------------ //
	// --------------- check if there is a valid certificate already in flash ------------- //
	case GS_MAIN_STATE_CHECK_CERT :

		Sleep_Restore_Countdown();

		if (GS_Cert_LoadExistingCert())										// Load existing certificate from flash (if any)
		{
			GS_User_SM_SetState( GS_MAIN_STATE_SWICH_TO_CLIENT_MODE );		// go to next state
		}
		else
		{
			GS_User_SM_SetState( GS_MAIN_STATE_GET_CACERT );				// if there is no certificate, download new one
		}

		break;

	// ------------------------------------------------------------------------------------ //
	// ------------------- try to open tcp connection on mqtt server ---------------------- //
	case GS_MAIN_STATE_SWICH_TO_CLIENT_MODE :

		if (GS_Timeout(GS_TRY_INTERVAL))					// try on every GS_TRY_INTERVAL miliseconds
		{
			// reset if max retries
			if (GS_RepeatCounter_GetCnt() > GS_NUMBER_OF_RETRIES)
				CPU_System_Reset();

			GS_SetLeds(false, true);						// set led diodes

			if (GS_User_StartTCPTask() == true)				// try to open tcp socket
			{

#ifdef __SSL__
				if (GS_Cert_OpenSLLconn(GS_TCP_mqtt_GetClientCID()))		// try to open ssl socket
				{
					GS_ForgetServerIp();									// we do not need server ip any more

					GS_Api_SetUpSocket_MaxRT(GS_TCP_mqtt_GetClientCID(), SOCKET_OPTIONS_MAX_RETRIES_SECONDS);			// set up socket options

					GS_User_SM_SetState( GS_MAIN_STATE_CLIENT_MODE );		// go to next state
				}
				else
				{	// if max retries failed, get new certificate (most probably we can not connect because we have wrong cert)
					if (GS_RepeatCounter_GetCnt() > GS_NUMBER_OF_SSLOPEN_RETRIES)
					{
						GS_TCP_mqtt_Disconnect();
						GS_User_SM_SetState( GS_MAIN_STATE_GET_CACERT );
					}
				}
#else
				GS_ForgetServerIp();										// we do not need server ip any more

				GS_Api_SetUpSocket_MaxRT(GS_TCP_mqtt_GetClientCID(), SOCKET_OPTIONS_MAX_RETRIES_SECONDS);				// set up socket options

				GS_User_SM_SetState( GS_MAIN_STATE_CLIENT_MODE );			// go to next state
#endif

			}

			GS_RepeatCounter_UpdateTime();									// update time of last action

		}

		Sleep_Restore_Countdown();

		break;

	// ------------------------------------------------------------------------------------ //
	// -------------------- main loop for client mode  (mqtt connection) ------------------ //
	case GS_MAIN_STATE_CLIENT_MODE :

		if (GS_TCP_mqtt_GetClientCID() != GS_API_INVALID_CID)
		{
			GPIO_LedOff();
			if (MQTT_State_Machine() == 1)							// poll mqtt state machine
				Sleep_Restore_Countdown();
		}
		break;

	// ------------------------------------------------------------------------------------ //
	// ----------------------- main loop for client mode  (AP mode) ----------------------- //
	case GS_MAIN_STATE_LIMITED_AP :

		Sleep_Restore_Countdown();

		Onbrd_State_Machine();										// poll onboarding state machine

		break;

	// ------------------------------------------------------------------------------------ //
	// ----------------------- default - do nothing --------------------------------------- //
	default :
		break;
	}
}

/**
*  @brief  Handle error messages from GS module
*
*  Handle error messages from GS module.
*
*  @param  Error message
*
*  @return void
*/
void App_HandleErrorMessage(int error_message){
	switch (error_message)
	{
	case HOST_APP_MSG_ID_ERROR_SOCKET_FAIL :
		GS_ProcessMqttDisconnect();				// disconnect if socket failure detected
		break;

	case HOST_APP_MSG_ID_UNEXPECTED_WARM_BOOT :
	case HOST_APP_MSG_ID_APP_RESET :
	case HOST_APP_MSG_ID_DISASSOCIATION_EVENT :
		CPU_System_Reset();
		break;

	case HOST_APP_MSG_ID_DISCONNECT :
		{										// process disconnect message
			uint8_t cid;

			cid = GS_Api_ParseDisconnectCid();	// get cid from disconnect message

			if (GS_TCP_mqtt_GetClientCID() == cid)
			{
			    GS_ProcessMqttDisconnect();
			}

			if (GS_LAP_GetClientCid() == cid)
			{
				Onbrd_ClientDisconnected();
			}
		}
		break;

	default :
		break;
	}
}

/**
*  @brief  Event on completed tcp bulk transfer
*
*  @param  Connection ID
*
*  @return void
*/
void App_ProcessCompletedBulkTransferEvent(uint8_t cid){

	if (GS_Api_mqtt_CompletedBulkTransfer(cid) == true)
	{	// process mqtt data
		MQTT_Api_OnCompletedBulkTransfer();
		GS_TCP_mqtt_ResetBuffer();
		return;
	}

	if (GS_LAP_CompletedBulkTransfer(cid) == true)
	{   // if AP client received, process data
		Onbrd_WifiReceived();
		return;
	}
}

/**
*  @brief  Event on completed http bulk transfer
*
*  @param  Connection ID
*
*  @return void
*/
void App_ProcessCompletedHttpBulkTransferEvent(uint8_t cid){
	GS_Http_OnComplete(cid);
}

/**
*  @brief  Prepare stack for onboarding process
*
*  Set state and flags to initiate onboarding process
*
*  @return void
*/
void GS_User_GoToLimitedAP(){
	// turn off both leds
	MQTT_Api_ResetMqtt(true);
	GS_LimitedAP_mode_flag = 0;
	GS_User_SM_SetState( GS_MAIN_STATE_INIT );
	GS_LimitedAP_mode_flag = 1;
}

/**
*  Process mqtt connect event
*
*  Will process mqtt connect event.
*  Should be called when connack message is received.
*
*  @return void
*/
void GS_ProcessMqttConnect(){
	Sensor_Cfg_Run();			// send signal to master ble device to start
	GS_SetLeds(false, false);	// turn off leds
	GS_HAL_ClearBuff();			// clear buffer
}

/**
*  @brief  Process mqtt disconnect event
*
*  Should be called when ever mqtt disconnect is detected
*  Closes socket and set new state
*
*  @return void
*/
void GS_ProcessMqttDisconnect(){

    MSTimerDelay((unsigned long long int) 100);

    GS_HAL_ClearBuff();							// clear rx buff
	GS_API_CommWorking();						// send <AT> to module

	GS_TCP_mqtt_Disconnect();					// close connections
    GS_Api_CloseAll();

	MSTimerDelay(5000);							// apply small delay
	Sleep_Restore_Countdown();

	GS_HAL_ClearBuff();							// clear rx buff
	MQTT_Api_ResetMqtt(false);

	// check if the module is still associated on desired wifi network
	if (GS_API_IsAssociated((uint8_t *) wunderbar_configuration.wifi.ssid) == true)
		GS_User_SM_SetState( GS_MAIN_STATE_SWICH_TO_CLIENT_MODE );
	else
		GS_User_SM_SetState( GS_MAIN_STATE_TRY_TO_CONNECT );
}


	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////



/**
*  @brief  Load WiFI parameters for connection
*
*  Load predefined parameters fr connection.
*  WiFi ssid and password will be as onboarded by user
*/
static void GS_Load_Network_Parameters(HOST_APP_NETWORK_CONFIG_T *NetConf){

	sprintf(NetConf->security, 		WIFI_DEFAULT_SECURITY_CFG);
	sprintf(NetConf->dhcpEnabled, 	WIFI_DEFAULT_DHCPDENABLED_CFG);
	sprintf(NetConf->connType, 		WIFI_DEFAULT_CONNTYPE_CFG);
	sprintf(NetConf->wepId, 		WIFI_DEFAULT_WEPID_CFG);
	sprintf(NetConf->channel, 		WIFI_DEFAULT_CHANNEL);

	sprintf(NetConf->ssid, 			(char *) &wunderbar_configuration.wifi.ssid);
	sprintf(NetConf->passphrase, 	(char *) &wunderbar_configuration.wifi.password);
}


/**
*  @brief  Starts tcp connection with predefined parameters
*
*  Parameters:
*  destination ip   : wunderbar_configuration.cloud.ip
*  destination port : MQTT_RELAYR_SERVER_PORT
*
*  if wunderbar_configuration.cloud.ip is empty, it will perform
*  dns look up in order to resolve new server ip address.
*
*  @return True if successful
*/
static bool GS_User_StartTCPTask(){

	// if we do not have ip, do dns resolve
	if (wunderbar_configuration.cloud.ip[0] == 0xFF)
		if (GS_DNS_Resolve((char *) wunderbar_configuration.cloud.url, (char *) wunderbar_configuration.cloud.ip) == false)
			return false;

	// open tcp connection
	return GS_TCP_mqtt_StartTcpTask( (char *) wunderbar_configuration.cloud.ip, MQTT_RELAYR_SERVER_PORT );
}

/**
*  @brief  Joins network with predefined settings
*
*  @return True or false regarding of successful connection
*/
static bool GS_User_Join_Network() {
	 bool result = false;
     // Test the module first
     if (!GS_API_CommWorking())
     {
          // no AT response
          return result;
     }

     // clear uart receive buffer before association
     GS_HAL_ClearBuff();
     // try to associate
	 if(GS_API_JoinWiFiNetwork(&apiNetworkConfig))
	 {
		 MSTimerDelay((unsigned long long int) 100);

		 if (GS_API_IsAssociated((uint8_t* ) apiNetworkConfig.ssid) == true)
			 result = true;
	 }

     return result;
}

/**
*  @brief  dns resolve url address.
*
*  @param  url address to be resolved
*  @param  return ip string with result (if successful)
*
*  @return True if ip resolved
*/
static bool GS_DNS_Resolve(char* url, char* ip){
	char temp_ip[16];

	if (GS_API_DNSResolve((int8_t*) url, (uint8_t*) temp_ip) == true)
	{
		strcpy((char *) ip, (const char *) temp_ip);
		return true;
	}

	return false;
}

/**
*  @brief  Forget current IP
*
*  Delete ip address from ram, we do not need it any more.
*  Before next connection we will do dns look up to get new ip
*
*  @return void
*/
static void GS_ForgetServerIp(){
	memset((void *) wunderbar_configuration.cloud.ip, (int) 0xFF, sizeof(wunderbar_configuration.cloud.ip));
}

/**
*  @brief  Reads current time from GS module and load it into RTC module
*
*  @return void
*/
static void GS_User_SetSystemTime(){
	char 					timeStr[15];
	unsigned long long int 	time = 0;
	char 					*ptr = timeStr;

	GS_Api_GetSystemTime(timeStr);
	while (*ptr)
		time = time * 10 + ((*ptr++) - '0');

	RTC_SetTime(time);
}

/**
*  @brief  Gets predefined delay depending on repeat counter
*
*  Returns delay in milliseconds which gets larger with increasing repeated counter
*
*  @return delay in milliseconds
*/
static unsigned long long int GS_GetDelay(){
	char DelayArr[] = {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 5, 5, 5, 5, 5};

	if (RepeatCounter.Cnt < sizeof (DelayArr))
		return (((unsigned long long int) DelayArr[(uint8_t) RepeatCounter.Cnt]) * 1000);
	else
		return (((unsigned long long int) DelayArr[sizeof(DelayArr) - 1]) * 1000);
}

/**
*  @brief  Increment repeat counter
*
*  @return  void
*/
static void GS_RepeatCounter_IncrementCnt(){
	if (RepeatCounter.Cnt < 100)
		RepeatCounter.Cnt ++;
}

/**
*  @brief  Updates time of last action
*
*  @return void
*/
static void GS_RepeatCounter_UpdateTime(){
	RepeatCounter.Time = MSTimerGet();
}

/**
*  @brief  Get repeat counter value
*
*  @return repeat counter
*/
static char GS_RepeatCounter_GetCnt(){
	return RepeatCounter.Cnt;
}

/**
*  @brief  Checks if timeout occurred since last action
*
*  @param  Time of last action
*
*  @return True if timeout occurred
*/
static bool GS_Timeout(unsigned long long int timeout){
	if (MSTimerDelta(RepeatCounter.Time) > timeout)
	{
		GS_RepeatCounter_IncrementCnt();
		GS_RepeatCounter_UpdateTime();
		return true;
	}
	else
		return false;
}

/**
*  @brief  Wait for for predefined number of seconds before passing
*
*  @return True if time passed, false if we still wait
*/
static bool GS_Wait(){
	if (MSTimerDelta(RepeatCounter.Time) > GS_GetDelay(RepeatCounter.Cnt))
	{
		GS_RepeatCounter_IncrementCnt();
		return 1;
	}
	else
		return 0;
}

/**
*  @brief  Set two led diodes to signal current state
*
*  Set two leds to indicate states. First led is controlled by kinetis and seconds by GS module
*
*  @param  State of GS module led
*  @param  State of kinetis led
*
*  @return void
*/
static void GS_SetLeds(bool led1, bool led2){

	GS_Api_Gpio30_Set(led1);

	if (led2 == true)
		GPIO_LedOn();
	else
		GPIO_LedOff();
}

/**
*  @brief  Set new state and updates time of last action
*
*  Set new desired state and update time of last action.
*  If we are in Onboarding mode, avoid switching states.
*  From onboarding we exit on reset.
*
*  @param  New desired state
*
*  @return void
*/
static void GS_User_SM_SetState(MainState_t state){
	// if AP mode requested, do not change state
	if (GS_LimitedAP_mode_flag == 0)
	{
		GS_RepeatCounter_UpdateTime();
		RepeatCounter.Cnt = 0;
		MainState = state;
	}
}
