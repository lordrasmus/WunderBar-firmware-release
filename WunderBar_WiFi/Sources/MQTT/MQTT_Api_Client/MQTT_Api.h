/** @file   MQTT_Api.h
 *  @brief  File contains function for handling mqtt protocol
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include "MQTT_MsgService.h"
#include "../MQTT_paho/MQTTPacket.h"

// function pointer for message handler function
void (*MQTT_Api_ProcessReceivedMessage)(MQTT_User_Message_t* message);

// public functions

/**
*  @brief  MQTT Publish
*
*  Publishes desired message on desired topic on mqtt server
*
*  @param  Message struct with topic, payload and option flags
*
*  @return Message handler
*/
char MQTT_Api_Publish(MQTT_User_Message_t* msg);

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
char MQTT_Api_Subscr(char* topic, int qos );

/**
*  @brief  MQTT Unsubscribe
*
*  Unsubscribes desired topic from mqtt server
*
*  @param  Desired topic string
*
*  @return Message handler
*/
char MQTT_Api_Unsubscr(char* topic);

/**
*  @brief  Process subscription response (suback/unsuback) from mqtt server
*
*  It will update status in sensor ID list
*
*  @param  Sub topic
*
*  @return void
*/
void MQTT_Api_ProcessSubscription(char* topic);

/**
*  @brief  Reset mqtt stack
*
*  Reset mqtt state machine and depending on input parameter, reset message buffer.
*
*  @param  Clean start flag, if set clear message buffer
*
*  @return void
*/
void MQTT_Api_ResetMqtt(bool clean_start);

/**
*  MQTT on connect event
*
*  Will be called when connack message is received.
*
*  @return void
*/
void MQTT_OnConnectEvent();

/**
*  MQTT on disconnect event
*
*  Will be called when disconnect from mqtt is detected
*  Reset mqtt state machine, and keeps message buffer
*
*  @return void
*/
void MQTT_OnDisconnectEvent();

/**
*  @brief  Check subscription list
*
*  Check if there are any topics for subscription and schedule them.
*
*  @return void
*/
void MQTT_Api_CheckSubList();

/**
*  @brief  MQTT on received message response timeout
*
*  When new mqtt message is received and processed it will wait for defined
*  period of milliseconds to be processed before discarding message.
*  This way user can signal that the message was processed successfully
*
*  @return void
*/
void MQTT_OnMsgResponseTimeout();

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/**
*  @brief  Return mqtt status
*
*  @return 1 if running, 0 if not connected
*/
char MQTT_Get_RunnigStatus();

/**
*  @brief  Event on completed bulk transfer
*
*  Event generated on completed bulk transfer over tcp socket.
*  Parses incoming buffer for mqtt messages and process them.
*
*  @return True if at least one mqtt message processed.
*/
bool MQTT_Api_OnCompletedBulkTransfer();

/**
*  @brief  State Machine for MQTT Client
*
*  Process states of mqtt client. Will initiate connection, and maintain connection
*  while processing incoming and outgoing messages.
*
*  @return 0 - if no action done.
*/
char MQTT_State_Machine();



//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/**
*  @brief  Clear Message in progress flag
*
*  Clear in progress flag. After this new messages will be processed.
*
*  @return void
*/
void MQTT_Msg_ClearMsgInProgress();

/**
 *  @brief  Set Time to ping flag
 *
 *  Force ping request.
 *
 *  @return void
 */
void MQTT_SetPingFlag();

/**
*  @brief  Load Options to be used when connecting to MQTT Server
*
*  Load default predefined connect options for mqtt client.
*
*  @param  Mqtt client options struct
*
*  @return void
*/
void MQTT_User_Get_Options(MQTTPacket_connectData* Client_Options);

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
void MQTT_Api_SetReceiveCallBack(void (*MQTT_User_CallBack)(MQTT_User_Message_t* userMessage));
