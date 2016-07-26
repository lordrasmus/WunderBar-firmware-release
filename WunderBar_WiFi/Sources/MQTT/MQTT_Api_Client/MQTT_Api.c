/** @file   MQTT_Api.c
 *  @brief  File contains function for handling mqtt protocol
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <string.h>

#include <Common_Defaults.h>

#include "../../Sensors/Sensors_SensID.h"
#include "../../Sensors/Sensors_main.h"
#include "../../GS/GS_User/GS_User.h"

#include "../MQTT_paho/MQTTPacket.h"

#include "MQTT_User.h"
#include "MQTT_Api.h"


// static declaration

static void MQTT_Api_GetDefaultMsgOpt(MQTT_User_Message_t* msg);



	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////



/**
*  MQTT call back definition
*
*  Set call back function for mqtt reception messages
*  It will be called when new mqtt publish message should be processed
*
*  @param  Function pointer to mqtt message processing function
*
*  @return void
*/
void MQTT_Api_SetReceiveCallBack(void (*MQTT_User_CallBack)(MQTT_User_Message_t* userMessage)){
	MQTT_Api_ProcessReceivedMessage = MQTT_User_CallBack;
}

/**
*  @brief  MQTT Publish
*
*  Publishes desired message on desired topic on mqtt server
*
*  @param  Message struct with topic, payload and option flags
*
*  @return Message handler
*/
char MQTT_Api_Publish(MQTT_User_Message_t* msg){
	MQTT_Api_GetDefaultMsgOpt(msg);

	return MQTT_Msg_PrepareForSend(msg);
}

/**
*  @brief  MQTT Subscribe
*
*  Subscrbes on desired topic on mqtt server with desired QOS level
*
*  @param  Topic string
*  @param  Desired qos level
*
*  @return Message handler
*/
char MQTT_Api_Subscr(char* topic, int qos ){
	MQTT_User_Message_t msg;

	msg.qos 			= qos;
	msg.messageType 	= SUBSCRIBE_MESSAGE;
	msg.dup 			= MQTT_MSG_OPT_DUP;
	msg.retained 		= MQTT_MSG_OPT_RETAINED;
	msg.payloadlen 		= strlen(topic);
	strcpy(msg.topicStr, topic);

	return MQTT_Msg_PrepareForSub(&msg);
}

/**
*  @brief  MQTT Unsubscribe
*
*  Unsubscribes desired topic from mqtt server
*
*  @param  Desired topic string
*
*  @return Message handler
*/
char MQTT_Api_Unsubscr(char* topic){
	MQTT_User_Message_t msg;

	msg.messageType = UNSUBSCRIBE_MESSAGE;
	msg.dup = 0;
	msg.retained = 0;
	msg.payloadlen = strlen(topic);
	strcpy(msg.topicStr, topic);

	return MQTT_Msg_PrepareForUnsub(&msg);
}

/**
*  @brief  Process subscription response (suback/unsuback) from mqtt server
*
*  It will update status in sensor ID list
*
*  @param  Sub topic
*
*  @return void
*/
void MQTT_Api_ProcessSubscription(char* topic){
	Sensors_ID_ProcessSuccessfulSubscription(topic);
}

/**
*  @brief  Check subscription list
*
*  Check if there are any topics for subscription and schedule them.
*
*  @return void
*/
void MQTT_Api_CheckSubList(){
	Sensors_ID_CheckSubList();
}

/**
*  @brief  Reset mqtt stack
*
*  Reset mqtt state machine and depending on input parameter, reset message buffer.
*
*  @param  Clean start flag, if set clear message buffer
*
*  @return void
*/
void MQTT_Api_ResetMqtt(bool clean_start){
	MQTT_User_ResetState();
	if (clean_start)
		MQTT_Msg_DiscardAllMsg();
}

/**
*  MQTT on connect event
*
*  Will be called when connack message is received.
*
*  @return void
*/
void MQTT_OnConnectEvent(){
	GS_ProcessMqttConnect();
}

/**
*  MQTT on disconnect event
*
*  Will be called when disconnect from mqtt is detected
*  Reset mqtt state machine, and keeps message buffer
*
*  @return void
*/
void MQTT_OnDisconnectEvent(){
	MQTT_Api_ResetMqtt(false);
	GS_ProcessMqttDisconnect();
}

/**
*  @brief  MQTT on received message response timeout
*
*  When new mqtt message is received and processed it will wait for defined
*  period of milliseconds to be processed before discarding message.
*  This way user can signal that the message was processed successfully
*
*  @return void
*/
void MQTT_OnMsgResponseTimeout(){
	Sensors_ProcessTimeout();
}

/**
*  @brief  Load Options to be used when connecting to MQTT Server
*
*  Load default predefined connect options for mqtt client.
*
*  @param  Mqtt client options struct
*
*  @return void
*/
void MQTT_User_Get_Options(MQTTPacket_connectData* Client_Options){
	Client_Options->MQTTVersion       = MQTT_MQTTVERSION;
	Client_Options->keepAliveInterval = MQTT_KEEPALIVEINTERVAL;
	Client_Options->cleansession      = MQTT_CLEANSESSION;

	Client_Options->clientID.cstring  = (char *) &wunderbar_configuration.wunderbar.id;
	Client_Options->username.cstring  = (char *) &wunderbar_configuration.wunderbar.id;
	Client_Options->password.cstring  = (char *) &wunderbar_configuration.wunderbar.security;
}




	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////



/**
*  @brief  Get message options for mqtt message
*
*  Set default mqtt options for publish message
*
*  @param  Pointer to struct of mqtt message
*
*  @return void
*/
static void MQTT_Api_GetDefaultMsgOpt(MQTT_User_Message_t* msg){

	msg->qos 			= MQTT_MSG_OPT_QOS_PUB;
	msg->dup 			= MQTT_MSG_OPT_DUP;
	msg->retained 		= MQTT_MSG_OPT_RETAINED;
	msg->messageID 		= 0;
	msg->messageType 	= PUBLISH_MESSAGE; // publish
}
