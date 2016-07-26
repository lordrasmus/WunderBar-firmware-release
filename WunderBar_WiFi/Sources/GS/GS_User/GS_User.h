/** @file   GS_User.h
 *  @brief  File contains functions to handle GS module states.
 *  		Processes connection on WiFi network and communication over
 *  		tcp secured socket with mqtt server.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */



//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#define SOCKET_OPTIONS_MAX_RETRIES_SECONDS  30

#define GS_WAIT_TIMEOUT   				5000
#define GS_TRY_INTERVAL  				1000
#define GS_NUMBER_OF_RETRIES  			10
#define GS_NUMBER_OF_SSLOPEN_RETRIES 	GS_NUMBER_OF_RETRIES - 3           // must be less then GS_NUMBER_OF_RETRIES


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

typedef enum{
     GS_MAIN_STATE_INIT,
     GS_MAIN_STATE_TRY_TO_CONNECT,
     GS_MAIN_STATE_GET_SERVER_TIME,
     GS_MAIN_STATE_WAIT_SERVER_TIME,
     GS_MAIN_STATE_GET_CACERT,
     GS_MAIN_STATE_WAIT_CACERT,
     GS_MAIN_STATE_SWICH_TO_CLIENT_MODE,
     GS_MAIN_STATE_CHECK_CERT,
     GS_MAIN_STATE_CLIENT_MODE,
     GS_MAIN_STATE_LIMITED_AP
}MainState_t;


// public functions

/**
*  @brief  Gain Span main state machine
*
*  Processes WiFi network and connections.
*  Should be called frequently from low priority int or main loop
*
*  @return void
*/
void GS_Main_StateMachine();

/**
*  @brief  Prepare stack for onboarding process
*
*  Set state and flags to initiate onboarding process
*
*  @return void
*/
void GS_User_GoToLimitedAP();

/**
*  Process mqtt connect event
*
*  Will process mqtt connect event.
*  Should be called when connack message is received.
*
*  @return void
*/
void GS_ProcessMqttConnect();

/**
*  @brief  Process mqtt disconnect event
*
*  Should be called when ever mqtt disconnect is detected
*  Closes socket and set new state
*
*  @return void
*/
void GS_ProcessMqttDisconnect();
