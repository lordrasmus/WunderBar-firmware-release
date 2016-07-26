/** @file   Onboarding.c
 *  @brief  File contains functions used for onboarding process.
 *  		Initiate onboarding and handles state machine during onboarding
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <Common_Defaults.h>
#include <hardware/Hw_modules.h>
#include "User_init.h"
#include "Onboarding.h"
#include "../GS/API/GS_API.h"
#include "../GS/GS_User/GS_Limited_AP.h"
#include "../GS/GS_User/GS_User.h"
#include "../Sensors/Sensors_main.h"
#include "../FTFE/flash_FTFE.h" /* include flash driver header file */



// static declarations

static struct ButtonPress_t {
	unsigned long long int 	Time;
	char          		State;
} ButtonPress;

static struct OnboardingProcess_t {
	unsigned long long int 	Time;
	unsigned long long int 	Blink;
	char 				Started;
} OnboardingProcess;


static Onboarding_Process_State_t Onbrd_State;
static char Onboarding_ClientDisconnectFlag = 0;

static void Onbrd_LoadParameters(AP_Parameters_t* AP_params);
static Onboarding_Process_State_t Onbrd_GetState();
static void Onbrd_SetState(Onboarding_Process_State_t state);
static bool Onbrd_StoreWifiCfgInFlash(wcfg_t* wcfg);
static void Onbrd_SendClientResponse(unsigned int mask);
static void Onbrd_PrepareForOnboarding();
static void Onbrd_StartProcess();




	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////




/**
 *  @brief  Timer poll to detect 2 second hold and timeouts
 *
 *  Should be called from timer interrupt (TI2)
 *  Polls user button and detects short and long press of user button.
 *
 *  @return void
 */
void Onbrd_Poll(){

	if (ButtonPress.State == 1)
	{
		Sleep_Restore_Countdown();

		if (GPIO_GetButtonState() == false)  // onboarding button still pressed
		{
			if (Onbrd_State == ONBOARDING_OFF)
			{
				if (MSTimerDelta(ButtonPress.Time) > ONBOARDING_BUTTON_TIMEOUT)
				{
					// if button was pressed more      //
					// then 2 seconds do system reset  //
					// ------------------------------- //
					Onbrd_PrepareForOnboarding();
					CPU_System_Reset();
					// ------------------------------- //
					// after this we will reset into   //
					//        bootloader mode          //
					// bootloader mode can be entered  //
					//     only from run state or      //
					// by reseting while holding down  //
					//    user button at any moment    //
				}
			}
		}
		else
		{
			if (Onbrd_State == ONBOARDING_OFF)
			{
				// if the button was pressed less   //
				// then 2 seconds, start Onboarding //
				// -------------------------------- //
				Onbrd_PrepareForOnboarding();
				GS_User_GoToLimitedAP();
				Onbrd_SetState(ONBOARDING_START);
			}
			else
			{
				// if the button was pressed during //
				//         Onboarding mode          //
				//      reset into normal mode  	//
				// -------------------------------- //
				CPU_System_Reset();
			}
		}
	}

	if (Onbrd_State >= ONBOARDING_SERVER_UP)
	{
		if (MSTimerDelta(OnboardingProcess.Blink) >= ONBOARDING_LED_BLINK)
		{	// blink LED to signal AP mode running
			OnboardingProcess.Blink = MSTimerGet();
			GPIO_LedToggle();
		}
	}
}

/**
 *  @brief  Onboarding state machine.
 *
 *  Should be called frequently when onboarding process is activated.
 *  Starts Limited Access Point and tcp server at predefined port
 *  and waits for new configuration from WiFi client or from ble master module
 *
 *  When new configuration is received, saves it into flash.
 *
 *  @return void
 */
void Onbrd_State_Machine(){

	switch (Onbrd_State)
	{
	// ------------------- onboarding is off, return do nothing ---------------------- //
	case ONBOARDING_OFF :
		break;

	// ------------------- init access point mode ------------------------------------ //
	case ONBOARDING_START :

		Onbrd_StartProcess();
		Onbrd_SetState( ONBOARDING_SERVER_UP );	 // go to next state (try to start server)

		break;

	// ------------------- start access point server, and start tcp server ----------- //
	case ONBOARDING_SERVER_UP :
		{
			AP_Parameters_t AP_Parameters;

			Onbrd_LoadParameters(&AP_Parameters);		// load AP parameters that we will use

			// Start limited Access Point
			if (GS_LAP_StartServer((char *) &AP_Parameters.ssid, (char *) &AP_Parameters.ip, (char *) &AP_Parameters.subnetMask) == true)
			{
				// Start tcp server at desired port
				if (GS_LAP_StartTcpServer((char *) &AP_Parameters.port) == true)
				{
					Sensor_Cfg_Start();						// signal ble master module to start config mode
					Onbrd_SetState(ONBOARDING_WAIT);		// wait for new configuration
				}
			}
			else
			{	// if failed try again
				Onbrd_PrepareForOnboarding();
				GS_User_GoToLimitedAP();
				Onbrd_SetState(ONBOARDING_START);
			}
		}
		break;

	// ------------------- waiting for data from connected device -------------------- //
	case ONBOARDING_WAIT :
		// wait for the data from WiFi client  //
		//     or from master ble module       //
		//            do nothing               //
		// ----------------------------------- //
		break;

	// ------------------- received data from wifi client ---------------------------- //
	case ONBOARDING_AP_RECV :
		{
			unsigned int cnt, mask;

			// new configuration is received by WiFi client
			mask = Onbrd_Process_msg(GS_LAP_GetBuffer());
			// store configs in flash if there was no errors
			if ((mask & CFG_FAILED_MASK) == 0)
			{
				if (Onbrd_StoreWifiCfgInFlash(&wunderbar_configuration) == false)
				{
					Onbrd_SetState(ONBOARDING_FAILED);
					mask |= CFG_FLWR_FAILED_MASK;
				}
			}

			GS_LAP_ResetIncomingBuffer(&cnt);			// reset incoming buffer
			Onbrd_SendClientResponse(mask);				// send report to wifi client

			if (mask & CFG_FAILED_MASK)
				Onbrd_SetState(ONBOARDING_FAILED);		// onboarding failed
			else
				Onbrd_SetState(ONBOARDING_SUCCESS);		// onboarding success

			Onbrd_UpdateCurrentProcessTime();
		}
		break;

	// ------------------- received data from master ble ----------------------------- //
	case ONBOARDING_BLE_RECV :

		// save new configuration that we have received from ble master
		if (Onbrd_StoreWifiCfgInFlash(&wunderbar_configuration) == false)
			Onbrd_SetState(ONBOARDING_FAILED);
		else
		{
			Onbrd_SetState(ONBOARDING_SUCCESS);
			Onboarding_ClientDisconnectFlag = 1;
		}
		Onbrd_UpdateCurrentProcessTime();

		break;

	// ------------------- onboarding successful ------------------------------------- //
	case ONBOARDING_SUCCESS :
		{
			// wait before reset for client disconnection or timeout interval
			if ((MSTimerDelta(OnboardingProcess.Time) > ONBOARDING_SUCCESS_TIMEOUT) || (Onboarding_ClientDisconnectFlag))
			{
				GS_API_CloseAllConnections();
				// do system reset on success         //
				// ---------------------------------- //
				CPU_System_Reset();
				// ---------------------------------- //
				// after this we will reset into      //
				// normal mode with new configuration //
			}
		}
		break;

	// ------------------- onboarding failed ----------------------------------------- //
	case ONBOARDING_FAILED:
		{
			// stay blocked if error occurred 	   //
			// led will signal that error occurred //
			// ----------------------------------- //
			GPIO_LedOn();
			while (true)
				;
			// ----------------------------------- //
			// only hardware reset to start again  //
			// user should repeat configuration    //
		}
		break;

	// ------------------- default, do nothing --------------------------------------- //
	default :
		break;
	}
}

/**
 *  @brief  On board button press event
 *
 *  Should be called when user (onboarding) button is pressed.
 *  Set pressed state and starts measuring time that the button is pressed.
 *
 *  @return void
 */
void Onbrd_ButtonPressEvent(){
	ButtonPress.Time = MSTimerGet();
	ButtonPress.State = 1;
}

/**
 *  @brief  Update current time
 *
 *  Updates current time of the process. Used to detect timeout.
 *
 *  @return void
 */
void Onbrd_UpdateCurrentProcessTime(){
	OnboardingProcess.Time = MSTimerGet();
}

/**
 *  @brief  Set onboarding start flag.
 *
 *  Signal that onboarding process has started. Can be set from WiFi AP mode of ble mode.
 *  Used to detect that onboarding has been started and eventually error condition
 *
 *  @return void
 */
void Onbrd_SetStartProcessFlag(){
	OnboardingProcess.Started = 1;
}

/**
 *  @brief  Initiate onboarding process.
 *
 *  Signal that Onboarding process should be initiated.
 *  Used to send board into onboarding mode on start, if the board is blank (never configured)
 *
 *  @return void
 */
void Onbrd_GoToStart(){
	Onbrd_SetState(ONBOARDING_START);
}

/**
 *  @brief  Received config from wifi client.
 *
 *  Called when new configuration is received from WiFi client.
 *  After this we should process new data.
 *
 *  @return void
 */
void Onbrd_WifiReceived(){
	if (Onbrd_GetState() != ONBOARDING_OFF)
		Onbrd_SetState(ONBOARDING_AP_RECV);
}

/**
 *  @brief  WiFi client closed connection
 *
 *  Called when WiFi client is disconnected from tcp server.
 *  Will be catch only if client sent data.
 *
 *  @return void
 */
void Onbrd_ClientDisconnected(){
	Onbrd_SetState(ONBOARDING_SUCCESS);
	Onboarding_ClientDisconnectFlag = 1;
}

/*
 *  @brief  Received config from ble master module.
 *
 *  Called when new configuration is received from ble master.
 *  After this we should process new data.
 *
 *  @return void
 */
void Onbrd_MasterBleReceived(){
	if (Onbrd_GetState() != ONBOARDING_OFF)
		Onbrd_SetState(ONBOARDING_BLE_RECV);
}





	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////



/**
 *  @brief  Load predefined Onboarding parameters
 *
 *  Loads parameters for Access Point which will be used during Onboarding
 *
 *  @param  AP parameter struct
 *
 *  @return void
 */
static void Onbrd_LoadParameters(AP_Parameters_t* AP_params){
	strcpy(AP_params->ssid, 		(const char *) AP_PARAMETER_SSID);
	strcpy(AP_params->ip,			(const char *) AP_PARAMETER_IP);
	strcpy(AP_params->subnetMask, 	(const char *) AP_PARAMETER_SUBNET);
	strcpy(AP_params->port, 		(const char *) AP_PARAMETER_PORT);
}

/**
 *  @brief  Sets default flag values for Onboarding process
 *
 *  Reset flag values for onboarding process.
 *
 *  @return void
 */
static void Onbrd_StartProcess(){
	ButtonPress.State = 0;
	GPIO_LedOn();

	Onboarding_ClientDisconnectFlag = 0;
	OnboardingProcess.Blink = MSTimerGet();
}

/**
 *  @brief  Reset nRF and WiFi modules before entering Onboarding mode
 *
 *  Reset modules to have clean start for Onboarding mode.
 *
 *  @return void
 */
static void Onbrd_PrepareForOnboarding(){
	GS_Api_SetResponseTimeoutHandle(100);

	Reset_Wifi();
	Reset_Nordic();

	ButtonPress.State = 0;
}

/**
 *  @brief  Store new configuration in flash
 *
 *  When new configuration is obtained, save it in flash on predefined location.
 *  After saving, compare stored content with current one, and return true if they match
 *
 *  @param  configuration struct to save
 *
 *  @return true if configuration saving was successful
 */
static bool Onbrd_StoreWifiCfgInFlash(wcfg_t* wcfg){
	const wcfg_t *p_const_wcfg = (void *) FLASH_CONFIG_IMAGE_ADDR;
	wcfg_t wcfg_cmp;

	// do flash erase / write
	Cpu_DisableInt();
	Flash_SectorErase(FLASH_CONFIG_IMAGE_ADDR);
	Flash_ByteProgram(FLASH_CONFIG_IMAGE_ADDR, (uint32_t *) wcfg, sizeof(wcfg_t));
	Cpu_EnableInt();

	wcfg_cmp = *p_const_wcfg;

	// compare stored configuration with current
	if (memcmp((const void *) &wcfg_cmp, (const void *) wcfg, sizeof(wcfg_t)) == 0)
		return true;

	return false;
}

/**
 *  @brief  Set desired state for Onboarding state machine
 *
 *  @param  desired new state
 *
 *  @return void
 */
static void Onbrd_SetState(Onboarding_Process_State_t state){
	Onbrd_State = state;
}

/**
 *  @brief  Get state from Onboarding state machine
 *
 *  @return Current Onboarding state
 */
static Onboarding_Process_State_t Onbrd_GetState(){
	return Onbrd_State;
}

/**
 *  @brief  Send response to Onboarding WiFI client
 *
 *  Return error code for WiFi Access Point, Onboarding process
 *  Following error codes are supported:
 *  200  - ERROR OK
 *  404  - NO Config detected
 *  405  - Bad Json format
 *  406  - Failed to write pass keys into ble master module
 *  407  - error saving new configuration
 *
 *  @param  Error code mask for new configuration
 *
 *  @return void
 */
static void Onbrd_SendClientResponse(unsigned int mask){
	char msg[20];

	if      (mask == 0)
		sprintf(msg, ONBOARDING_WIFI_RESPONSE, (int) WIFI_RESP_ERR_NO_CFG);   				// no config written (possible non recognized json tokens) - this is not considered as error
	else if (mask & CFG_BADJSON_FAILED_MASK)
		sprintf(msg, ONBOARDING_WIFI_RESPONSE, (int) WIFI_RESP_ERR_INVALID_JSON);   		// invalid json format message
	else if (mask & CFG_PASS_FAILED_MASK)
		sprintf(msg, ONBOARDING_WIFI_RESPONSE, (int) WIFI_RESP_ERR_PASSKEY_WRITE_FAIL);   	// failed to write passkey config to ble master
	else if (mask & CFG_FLWR_FAILED_MASK)
		sprintf(msg, ONBOARDING_WIFI_RESPONSE, (int) WIFI_RESP_ERR_CFG_SAVE_FAIL);   		// error saving new configuration
	else if (!(mask & CFG_FAILED_MASK))
		sprintf(msg, ONBOARDING_WIFI_RESPONSE, (int) WIFI_RESP_ERR_OK);   					// success

	GS_LAP_SendPacket(msg, strlen((const char *) msg));
}
