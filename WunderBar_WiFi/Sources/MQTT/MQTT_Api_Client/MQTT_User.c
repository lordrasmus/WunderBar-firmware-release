/** @file   MQTT_User.c
 *  @brief  File contains low level functions for servicing mqtt messages
 *          and ping mechanism
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */


#include <stddef.h>
#include <string.h>

#include <hardware/Hw_modules.h>
#include <Common_Defaults.h>

#include "../../GS/GS_User/GS_TCP_mqtt.h"

#include "../MQTT_API_Client/MQTT_User.h"
#include "../MQTT_paho/MQTTPacket.h"
#include "../MQTT_paho/MQTTPublish.h"
#include "../MQTT_paho/MQTTConnect.h"

#include "MQTT_Api.h"


#define MQTT_PING_MAX_RETRIES   				10
#define MQTT_PING_INTERVAL_IF_NO_PINGRESP  		3

	typedef enum {
		PING_IDLE,
		PING_TIME_TO_PING,
		PING_SENT_WAITING_PINGRESP
	} MQTT_Ping_t;

	static MQTT_Ping_t MQTT_TimeToPing = PING_IDLE;
	static char MQTT_Ping_Retries = 0;



	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////


/**
*  @brief  Sends PUBREL message to server
*
*  Serialize pubrel message and send it on socket
*
*  @param  Message dup flag
*  @param  Message ID flag
*
*  @return void
*/
void MQTT_User_SendPUBREL(int message_dup, int message_ID){
	char MQTT_buf[512];
	int MQTT_buflen = sizeof(MQTT_buf);
	int len = 0;

	len = MQTTSerialize_pubrel(MQTT_buf, MQTT_buflen, message_dup, message_ID);
	GS_Api_mqtt_SendPacket(MQTT_buf, len);
}

/**
*  @brief  Sends PUBACK message to server
*
*  Serialize puback message and send it on socket
*
*  @param  Message ID
*
*  @return void
*/
void MQTT_User_SendPUBACK(int message_ID){
	char MQTT_buf[512];
	int MQTT_buflen = sizeof(MQTT_buf);
	int len = 0;

	len = MQTTSerialize_puback(MQTT_buf, MQTT_buflen, message_ID);
	GS_Api_mqtt_SendPacket(MQTT_buf, len);
}

/**
*  @brief  Sends PUBREC message to server
*
*  Serialize pubrec message and send it on socket
*
*  @param  Message ID
*
*  @return void
*/
void MQTT_User_SendPUBREC(int message_ID){
	char MQTT_buf[512];
	int MQTT_buflen = sizeof(MQTT_buf);
	int len = 0;

	len = MQTTSerialize_pubrec(MQTT_buf, MQTT_buflen, message_ID);
	GS_Api_mqtt_SendPacket(MQTT_buf, len);
}

/**
*  @brief  Sends PUBCOMP message to server
*
*  Serialize pubcomp message and send it on socket
*
*  @param  Message ID
*
*  @return void
*/
void MQTT_User_SendPUBCOMP(int message_ID){
	char MQTT_buf[512];
	int MQTT_buflen = sizeof(MQTT_buf);
	int len = 0;

	len = MQTTSerialize_pubcomp(MQTT_buf, MQTT_buflen, message_ID);
	GS_Api_mqtt_SendPacket(MQTT_buf, len);
}

/**
*  @brief  Publishes desired message on server
*
*  Serialize publish message and send it on socket
*  Set led on as indicator and set RTC alarm on ping interval
*
*  @param  MQTT Message struct
*
*  @return bytes written, -1 if sending failed
*/
int MQTT_User_Publish(MQTT_User_Message_t* message){
	char MQTT_buf[512];
	int MQTT_buflen = sizeof(MQTT_buf);
	int len = 0;
	MQTTString topicString = MQTTString_initializer;

	topicString.cstring = message->topicStr;

	len = MQTTSerialize_publish(MQTT_buf, MQTT_buflen, message->dup, message->qos, message->retained,
								message->messageID, topicString, message->payloadStr, message->payloadlen);

	if (GS_Api_mqtt_SendPacket(MQTT_buf, len) == false)
		return -1;

	RTC_SetAlarm(MQTT_PING_INTERVAL);			// delay ping request

	return len;
}

/**
*  @brief  Sends subscribe message to mqtt server
*
*  Serialize subscribe message and send it on socket
*
*  @param  Message duplicate flag
*  @param  Message ID
*  @param  number of members
*  @param  Topic name string list
*  @param  Requested qos list
*
*  @return void
*/
void MQTT_User_Subscribe(int dup, int messID, int listMembers, MQTTString* topicList, int* qosList){
	char MQTT_buf[512];
	int MQTT_buflen = sizeof(MQTT_buf);
	int len = 0;

	len = MQTTSerialize_subscribe(MQTT_buf, MQTT_buflen, dup, messID, listMembers, topicList, qosList);

	GS_Api_mqtt_SendPacket(MQTT_buf, len);
}

/**
*  @brief  Sends unsubscribe message to mqtt server
*
*  Serialize unsubscribe message and send it on socket
*
*  @param  Message duplicate flag
*  @param  Message ID
*  @param  number of members
*  @param  Topic name string list
*
*  @return void
*/
void MQTT_User_Unsubscribe(int dup, int messID, int listMembers, MQTTString* topicList){
	char MQTT_buf[512];
	int MQTT_buflen = sizeof(MQTT_buf);
	int len = 0;

	len = MQTTSerialize_unsubscribe(MQTT_buf, MQTT_buflen, dup, messID, listMembers, topicList);

	GS_Api_mqtt_SendPacket(MQTT_buf, len);
}

/**
*  @brief  Pings mqtt server at appropriate time interval
*
*  Serialize ping request message and send it on socket.
*  Set rtc alarm at ping interval time and count unanswered ping requests.
*  If too many, consider socket failure.
*
*  @return True if we sent ping request
*/
bool MQTT_User_PingReq(){
	char MQTT_buf[512];
	int MQTT_buflen = sizeof(MQTT_buf);
	int len = 0;

	// if there is no responses on ping for max retries number, disconnect
	if (MQTT_Ping_Retries > MQTT_PING_MAX_RETRIES)
		MQTT_OnDisconnectEvent();

	if (MQTT_TimeToPing == PING_TIME_TO_PING)
	{
		len = MQTTSerialize_pingreq(MQTT_buf, MQTT_buflen);
		GS_Api_mqtt_SendPacket(MQTT_buf, len);
		RTC_SetAlarm(MQTT_PING_INTERVAL_IF_NO_PINGRESP);       // ping in 3 seconds again, if the pingresp is not received.
		MQTT_Ping_Retries ++;
		MQTT_TimeToPing = PING_SENT_WAITING_PINGRESP;

		return true;
	}

	return false;
}

/**
 *  @brief  Set Time to ping flag
 *
 *  Force ping request.
 *
 *  @return void
 */
void MQTT_SetPingFlag(){
	MQTT_TimeToPing = PING_TIME_TO_PING;
}

/**
*  @brief  Process ping response
*
*  Should be called when pinresp is received.
*  Reset ping retries counter and set rtc alarm at ping interval time.
*
*  @return void
*/
void MQTT_User_PingResp(){

	MQTT_Ping_Retries = 0;
	MQTT_TimeToPing = PING_IDLE;
	RTC_SetAlarm(MQTT_PING_INTERVAL);
}

/**
*  @brief  Init mqtt ping mechanism
*
*  Do first ping and set alarm at next ping interval.
*  All pings will occur on RTC alarm event.
*
*  @return void
*/
void MQTT_User_InitPing(){

	MQTT_TimeToPing = PING_TIME_TO_PING;
	MQTT_Ping_Retries = 0;
	RTC_SetAlarm(MQTT_PING_INTERVAL);
}

/**
*  @brief  Send Connect message to server
*
*  Serialize connect message and send it on socket.
*  Gets predifined connect options, which will be used on connection on mqtt server
*
*  @return void
*/
void MQTT_User_Connect(){
	char MQTT_buf[512];
	int MQTT_buflen = sizeof(MQTT_buf);
	int len = 0;
	MQTTPacket_connectData MQTT_Client_Options = MQTTPacket_connectData_initializer;

	MQTT_User_Get_Options(&MQTT_Client_Options);

	len = MQTTSerialize_connect(MQTT_buf, MQTT_buflen, &MQTT_Client_Options);
	GS_Api_mqtt_SendPacket(MQTT_buf, len);
}
