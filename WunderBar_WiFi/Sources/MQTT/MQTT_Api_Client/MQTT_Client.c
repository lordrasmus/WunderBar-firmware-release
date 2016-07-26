/** @file   MQTT_Client.c
 *  @brief  File contains functions for handling mqtt state machine
 *  		and processing incoming messages
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */


#include <stddef.h>
#include <string.h>

#include <Common_Defaults.h>
#include <hardware/Hw_modules.h>

#include "../../GS/GS_User/GS_TCP_mqtt.h"

#include "../MQTT_paho/MQTTPacket.h"
#include "../MQTT_paho/MQTTPublish.h"
#include "../MQTT_paho/MQTTConnect.h"

#include "MQTT_User.h"
#include "MQTT_MsgService.h"
#include "MQTT_Api.h"


// static declarations

typedef struct {
	int message_type;
	int dup;
	int messageID;
} MQTT_Response_t;

typedef enum{
     MQTT_STATE_BEGIN,
     MQTT_STATE_WAITING_FOR_CONNACK,
     MQTT_STATE_CONNECTED,
     MQTT_STATE_RUNNING
}MQTT_State_t;

static MQTT_State_t MQTT_State = MQTT_STATE_BEGIN;
static char MQTT_Running_Flag = 0;
static char MQTT_read_buf[512];
static int  MQTT_read_buflen = sizeof(MQTT_read_buf);

static bool MQTT_User_Receive();
static char MQTT_User_ProcessAck(MQTT_Response_t* response);
static char MQTT_User_ProcessConnack();
static int  MQTT_User_ReadMessage();
static void MQTT_User_ProcessPublish(MQTT_User_Message_t* message, MQTT_Response_t* response);
static char MQTT_User_ProcessSuback(MQTT_Response_t* response);
static char MQTT_User_ProcessUnsuback(MQTT_Response_t* response);
static void MQTT_User_ProcessResp(MQTT_User_Message_t* message, MQTT_Response_t* response);
static bool MQTT_User_WaitForResponse(unsigned long long int time);
static char MQTT_User_GetState();



	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////


/**
*  @brief  State Machine for MQTT Client
*
*  Process states of mqtt client. Will initiate connection, and maintain connection
*  while processing incoming and outgoing messages.
*
*  @return 0 - if no action done.
*/
char MQTT_State_Machine(){
	static unsigned long long int timeout;
	char result = 1;

	switch (MQTT_State)
	{
	// send connect message to mqtt
	case MQTT_STATE_BEGIN :

		MQTT_User_Connect();							// try to connect
		MQTT_State = MQTT_STATE_WAITING_FOR_CONNACK;	// now wait for the connack
		timeout = MSTimerGet();							// get current time

		break;

	// wait utill we get connack or timeout
	case MQTT_STATE_WAITING_FOR_CONNACK :

		// now wait for the connack response
		if (MQTT_User_WaitForResponse(timeout))
			MQTT_OnDisconnectEvent();						// if no connack received

		break;

	// if connected to mqtt server, subscribe to predefined topics
	case MQTT_STATE_CONNECTED :

		MQTT_User_InitPing();							// initiate ping
		MQTT_Api_CheckSubList();						// check if there are any connected sensors
		MQTT_OnConnectEvent();							// generate connected on mqtt server event
		MQTT_Running_Flag = 1;

		MQTT_State = MQTT_STATE_RUNNING;				// go to next state

		break;

	// process ping mechanism and message buffer
	case MQTT_STATE_RUNNING :

		if (MQTT_User_PingReq())						// keep alive connection
			GPIO_LedOn();								// signal successful action

		if ((result = MQTT_Msg_Process()) != 0)			// process mqtt messages
			GPIO_LedOn();								// signal successful action

		break;

	default :
		break;
	}

	return result;
}

/**
*  @brief  Event on completed bulk transfer
*
*  Event generated on completed bulk transfer over tcp socket.
*  Parses incoming buffer for mqtt messages and process them.
*
*  @return True if at least one mqtt message processed.
*/
bool MQTT_Api_OnCompletedBulkTransfer(){
    // process received message
    if (MQTT_User_GetState() > MQTT_STATE_BEGIN)
    {
    	while (GS_TCP_mqtt_GetRemBytes())
    	{
    		if (MQTT_User_Receive() == 1)
    			GS_TCP_mqtt_UpdatePtr();
    		else
    			break;
    	}
    	// at least one message received
    	return true;
    }
    return false;
}

/**
*  @brief  Reset state machine to initial value
*
*  @return void
*/
void MQTT_User_ResetState(){
	MQTT_State = MQTT_STATE_BEGIN;
	MQTT_Running_Flag = 0;
}

/**
*  @brief  Return mqtt status
*
*  @return 1 if running, 0 if not connected
*/
char MQTT_Get_RunnigStatus(){
	return MQTT_Running_Flag;
}

/**
*  @brief  Subscribe desired topic on mqtt server
*
*  Subscribes desired topic on mqtt server with requested qos
*
*  @param  Mqtt message struct
*
*  @return void
*/
void MQTT_Api_SubscribeTopic(MQTT_User_Message_t* MyMsg){
	MQTTString myTopic = MQTTString_initializer;
	int myQos = MyMsg->qos;

	myTopic.cstring = &MyMsg->topicStr[0];

	MQTT_User_Subscribe(MyMsg->dup, MyMsg->messageID, 1, &myTopic, &myQos);
}

/**
*  @brief  Unsubscribe desired topic on mqtt server
*
*  Unsubscribes desired topic on mqtt server
*
*  @param  Mqtt message struct
*
*  @return void
*/
void MQTT_Api_UnsubscribeTopic(MQTT_User_Message_t* MyMsg){
	MQTTString myTopic = MQTTString_initializer;

	myTopic.cstring = &MyMsg->topicStr[0];

	MQTT_User_Unsubscribe(MyMsg->dup, MyMsg->messageID, 1, &myTopic);
}





	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////



/**
*  @brief  Parses message from mqtt server.
*
*  Parses response message and if it is valid, acts accordingly
*  Should be called after a complete message from mqtt server is received.
*
*  @return false - no mqtt msg,  true  - we have received mqtt msg
*/
static bool MQTT_User_Receive(){
	char result = 0;
	MQTT_Response_t MQTT_User_Response;
	MQTT_User_Message_t MQTT_Received_Message;

	switch (MQTT_User_ReadMessage()){
	// just received connack message
	case CONNACK :
		result = MQTT_User_ProcessConnack(&MQTT_User_Response);
		break;

	// just received message from subscribed topic, we should process it
	case PUBLISH :
		MQTT_User_ProcessPublish(&MQTT_Received_Message, &MQTT_User_Response);
		result = 1;
		break;

	// just received Publish Acknowledgement, do nothing
	case PUBACK :
		result = MQTT_User_ProcessAck(&MQTT_User_Response);
		break;

	// just received PUBREC
	case PUBREC :
		result = MQTT_User_ProcessAck(&MQTT_User_Response);
		break;

	// just received PUBREL
	case PUBREL :
		result = MQTT_User_ProcessAck(&MQTT_User_Response);
		break;

	// just received PUBCOMP
	case PUBCOMP :
		result = MQTT_User_ProcessAck(&MQTT_User_Response);
		break;

	// just received SUBACK
	case SUBACK :
		result = MQTT_User_ProcessSuback(&MQTT_User_Response);
		break;

	// just received UNSUBACK
	case UNSUBACK :
		result = MQTT_User_ProcessUnsuback(&MQTT_User_Response);
		break;

	// just received ping response, reset ping timer interval
	case PINGRESP :
		MQTT_User_PingResp();
		result = 1;
		break;

	default :
		result = 0;
	}

	if (result)
		MQTT_User_ProcessResp(&MQTT_Received_Message, &MQTT_User_Response);

	return (result > 0);
}

/**
*  @brief  Wait for the response for predefined number of seconds (MQTT_SERVER_RESPONSE_TIMEOUT)
*
*  Non blocking call. Will return true if timeout occurred, and false if response is still waiting.
*
*  @param  desired time in milliseconds to wait for response
*
*  @return false - still waiting;  true - timeout
*/
static bool MQTT_User_WaitForResponse(unsigned long long int time){
	return ((MSTimerDelta(time) >= MQTT_SERVER_RESPONSE_TIMEOUT));
}

/**
*  @brief  Process mqtt acknowledge message
*
*  @param  Response struct
*
*  @return 0 failed; 1 - success
*/
static char MQTT_User_ProcessAck(MQTT_Response_t* response){
	return (MQTTDeserialize_ack(&response->message_type, &response->dup, &response->messageID, MQTT_read_buf, MQTT_read_buflen));
}

/**
*  @brief  Process mqtt  subscribe acknowledge message
*
*  @param  Response struct
*
*  @return 0 failed; 1 - success
*/
static char MQTT_User_ProcessSuback(MQTT_Response_t* response){
	int cnt, gqos;

	response->message_type = SUBACK;
	return (MQTTDeserialize_suback(&response->messageID, 2, &cnt, &gqos, MQTT_read_buf, MQTT_read_buflen));
}

/**
*  @brief  Process mqtt unsubscribe acknowledge message
*
*  @param  Response struct
*
*  @return 0 failed; 1 - success
*/
static char MQTT_User_ProcessUnsuback(MQTT_Response_t* response){

	response->message_type = UNSUBACK;
    return (MQTTDeserialize_unsuback(&response->messageID, MQTT_read_buf, MQTT_read_buflen));
}

/**
*  @brief  Reads entire mqtt message and stores it in buffer
*
*  Reads mqtt message from buffer and stores it in given buffer.
*  Decrypt message type and read entire message.
*
*  @param  Read buffer
*  @param  Read buffer length
*  @param  Function pointer for get data procedure (used to read received data)
*
*  @return Mwtt message type
*/
static int MQTT_User_ReadMessage(){
	return MQTTPacket_read(MQTT_read_buf, MQTT_read_buflen, GS_TCP_mqtt_GetData);
}

/**
*  @brief  Process connection acknowledge message
*
*  @param  Response struct
*
*  @return 0 failed; 1 - success
*/
static char MQTT_User_ProcessConnack(MQTT_Response_t* response){
	int rc;

	if (MQTTDeserialize_connack(&rc, MQTT_read_buf, MQTT_read_buflen) != 1 || rc != 0)
		return 0; // error no connack detected
	else {
		response->message_type = CONNACK;
		return 1; // connack!
	}
}

/**
*  @brief  Reads received message, and stores it in a buffer, for later use
*
*  @param  mqtt message struct
*  @param  response struct
*
*  @return void
*/
static void MQTT_User_ProcessPublish(MQTT_User_Message_t* message, MQTT_Response_t* response){
	MQTTString topic;
	char* payload_in;

	MQTTDeserialize_publish((int*) &message->dup, (int*) &message->qos, (int*) &message->retained, (int*) &message->messageID,
			                     &topic, &payload_in, (int*) &message->payloadlen, MQTT_read_buf, MQTT_read_buflen);

	// copy topic string
	memcpy((void *) message->topicStr, (const void *) topic.lenstring.data, topic.lenstring.len);
	message->topicStr[topic.lenstring.len] = '\0';  // add zero termination

	// copy payload string
	memcpy((void *) message->payloadStr, (const void *) payload_in, message->payloadlen);
	message->payloadStr[message->payloadlen] = '\0'; // add zero termination

	response->message_type = PUBLISH;
	response->dup = message->dup;
	response->messageID = message->messageID;
}

/**
*  @brief  Process response mqtt message, depending on message type
*
*  @param  mqtt message struct
*  @param  response struct
*
*  @return void
*/
static void MQTT_User_ProcessResp(MQTT_User_Message_t* message, MQTT_Response_t* response){

	switch (response->message_type)
	{
	case 0 :
		break;
	case CONNACK:
		MQTT_State = MQTT_STATE_CONNECTED;
		break;
	case PUBLISH:
		GPIO_LedOn();
		MQTT_Msg_ProcessRecvMsg(message);
		break;
	default :
		MQTT_Msg_ProcessResponse(response->message_type, response->messageID, response->dup);
		break;
	}
}

/**
*  @brief  Returns current state of mqtt process
*
*  @return Current mqtt state
*/
static char MQTT_User_GetState(){
	return MQTT_State;
}
