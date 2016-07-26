/** @file   MQTT_MsgService.h
 *  @brief  File contains functions for servicing all mqtt messages
 *  		Mqtt messages (incoming and outgoing) are stored into a buffer,
 *  		and a messages from buffer are periodically processed.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#ifndef MQTT_MSG_SERVICE_H_
#define MQTT_MSG_SERVICE_H_


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#define PUBLISH_MESSAGE      0x01
#define SUBSCRIBE_MESSAGE    0x02
#define UNSUBSCRIBE_MESSAGE  0x03

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#define MQTT_API_MSG_BUFFER 200

#define MQTT_MSG_RETRASMIT_TIMEOUT 			30000   // 10s
#define MQTT_MSG_DISCARD_AFTER_RETRANSMITS  10   // 10 * 10s
#define MQTT_MSG_RESPONSE_WAIT_TIMEOUT 		4000
#define MQTT_MSG_MAX_BYTES_TO_WRITE			500


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// message buffer states

typedef enum {
    MQTT_MSG_STATE_EMPTY = 0,
    MQTT_MSG_STATE_READY_TO_SEND,
    MQTT_MSG_STATE_READY_TO_SUBSCRIBE,
    MQTT_MSG_STATE_READY_TO_UNSUBSCRIBE,
    MQTT_MSG_STATE_PUBREC_WAITING,
    MQTT_MSG_STATE_PUBCOMP_WAITING,
    MQTT_MSG_STATE_PUBACK_WAITING,
    MQTT_MSG_STATE_PUBREL_WAITING,
    MQTT_MSG_STATE_SUBACK_WAITING,
    MQTT_MSG_STATE_UNSUBACK_WAITING,
    MQTT_MSG_STATE_PUBACK_READY_TO_SEND,
    MQTT_MSG_STATE_PUBREC_READY_TO_SEND,
    MQTT_MSG_STATE_PUBCOMP_READY_TO_SEND,
    MQTT_MSG_STATE_PUBREL_READY_TO_SEND,
    MQTT_MSG_STATE_PUBLISH_RECEIVED,
    MQTT_MSG_STATE_PUBACK_RECEIVED,
    MQTT_MSG_STATE_PUBREL_RECEIVED,
    MQTT_MSG_STATE_PUBREC_RECEIVED,
    MQTT_MSG_STATE_PUBCOMP_RECEIVED,
    MQTT_MSG_STATE_SUBACK_RECEIVED,
    MQTT_MSG_STATE_UNSUBACK_RECEIVED
}MQTT_Msg_State_t;

// one mqtt message

typedef struct {
	char qos;
	char dup;
	unsigned short messageID;
	char retained;
	char messageType;
	int payloadlen;
	char topicStr[100];
	char payloadStr[200];
} MQTT_User_Message_t;

// one mqtt message with its state

typedef struct {
	MQTT_Msg_State_t MQTT_MsgState;
	unsigned long long int TimeOfLastAction;
	unsigned int retransmittions;
	MQTT_User_Message_t MQTT_MyMessage;
} MQTT_Api_Msg_t;


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/**
*  @brief  Processes Messages from buffer
*
*  Should be called frequently. Processes messages from buffer.
*  Message buffer holds messages scheduled for sending and received messages.
*
*  @return 1 if there is still messages for processing
*/
char MQTT_Msg_Process();

/**
*  @brief  Prepare message for sending.
*
*  Gets free MID and load message in buffer.
*  This will schedule message for sending.
*
*  @param  MQTT message struct
*
*  @return Message handler
*/
char MQTT_Msg_PrepareForSend(MQTT_User_Message_t* MyMessage);

/**
*  @brief  Prepare msg for subscribe.
*
*  Gets free MID and load message in buffer.
*  This will schedule message for sending.
*
*  @param  MQTT message struct
*
*  @return Message handler
*/
char MQTT_Msg_PrepareForSub(MQTT_User_Message_t* MyMessage);

/**
*  @brief  Prepare msg for unsubscribe.
*
*  Gets free MID and load message in buffer.
*  This will schedule message for sending.
*
*  @param  MQTT message struct
*
*  @return Message handler
*/
char MQTT_Msg_PrepareForUnsub(MQTT_User_Message_t* MyMessage);

/**
 *  @brief  Discards all messages in buffer and clears flags
 *
 *  @return void
 */
void MQTT_Msg_DiscardAllMsg();

/**
*  @brief  Clear Message in progress flag
*
*  Clear in progress flag. After this new messages will be processed.
*
*  @return void
*/
void MQTT_Msg_ClearMsgInProgress();

/**
*  @brief  Process received response from mqtt and update appropriate message status
*
*  Processes response from the mqtt server and sets appropriate message state.
*
*  @param  Message handler
*
*  @return void
*/
void MQTT_Msg_ProcessResponse(int msgType, int msgID, int msgDup);

/**
*  @brief  Loads message after reception into buffer for further processing
*
*  After message is received save it into buffer for later process
*
*  @param  MQTT message struct
*
*  @return Message handler
*/
char MQTT_Msg_ProcessRecvMsg(MQTT_User_Message_t* MyMessage);


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////


/**
*  @brief  Subscribe desired topic on mqtt server
*
*  Subscribes desired topic on mqtt server with requested qos
*
*  @param  Mqtt message struct
*
*  @return void
*/
void MQTT_Api_SubscribeTopic(MQTT_User_Message_t* MyMsg);

/**
*  @brief  Unsubscribe desired topic on mqtt server
*
*  Unsubscribes desired topic on mqtt server
*
*  @param  Mqtt message struct
*
*  @return void
*/
void MQTT_Api_UnsubscribeTopic(MQTT_User_Message_t* MyMsg);

/**
*  @brief  Reset state machine to initial value
*
*  @return void
*/
void MQTT_User_ResetState();


#endif // MQTT_MSG_SERVICE_H_
