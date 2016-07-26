/** @file   MQTT_User.h
 *  @brief  File contains low level functions for servicing mqtt messages
 *          and ping mechanism
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */


#include "../MQTT_paho/MQTTPacket.h"
#include "MQTT_MsgService.h"


// public functions

/**
*  @brief  Send Connect message to server
*
*  Serialize connect message and send it on socket.
*  Gets predifined connect options, which will be used on connection on mqtt server
*
*  @return void
*/
void MQTT_User_Connect();

/**
*  @brief  Pings mqtt server at appropriate time interval
*
*  Serialize ping request message and send it on socket.
*  Set rtc alarm at ping interval time and count unanswered ping requests.
*  If too many, consider socket failure.
*
*  @return True if we sent ping request
*/
bool MQTT_User_PingReq();

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
void MQTT_User_Subscribe(int dup, int messID, int listMembers, MQTTString* topicList, int* qosList);

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
void MQTT_User_Unsubscribe(int dup, int messID, int listMembers, MQTTString* topicList);

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
int  MQTT_User_Publish(MQTT_User_Message_t* message);

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
void MQTT_User_SendPUBREL(int message_dup, int message_ID);

/**
*  @brief  Sends PUBACK message to server
*
*  Serialize puback message and send it on socket
*
*  @param  Message ID
*
*  @return void
*/
void MQTT_User_SendPUBACK(int message_ID);

/**
*  @brief  Sends PUBREC message to server
*
*  Serialize pubrec message and send it on socket
*
*  @param  Message ID
*
*  @return void
*/
void MQTT_User_SendPUBREC(int message_ID);

/**
*  @brief  Sends PUBCOMP message to server
*
*  Serialize pubcomp message and send it on socket
*
*  @param  Message ID
*
*  @return void
*/
void MQTT_User_SendPUBCOMP(int message_ID);

/**
*  @brief  Process ping response
*
*  Should be called when pinresp is received.
*  Reset ping retries counter and set rtc alarm at ping interval time.
*
*  @return void
*/
void MQTT_User_PingResp();

/**
*  @brief  Init mqtt ping mechanism
*
*  Do first ping and set alarm at next ping interval.
*  All pings will occur on RTC alarm event.
*
*  @return void
*/
void MQTT_User_InitPing();
