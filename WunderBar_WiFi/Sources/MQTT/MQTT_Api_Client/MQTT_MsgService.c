/** @file   MQTT_MsgService.c
 *  @brief  File contains functions for servicing all mqtt messages
 *  		Mqtt messages (incoming and outgoing) are stored into a buffer,
 *  		and a messages from buffer are periodically processed.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */


#include <string.h>

#include <hardware/Hw_modules.h>

#include "MQTT_MsgService.h"
#include "MQTT_User.h"
#include "MQTT_Api.h"


// static declarations

static int MQTT_BytesWrtitten;

static MQTT_Api_Msg_t MQTT_Api_Messages[MQTT_API_MSG_BUFFER];

static struct MQTT_MsgProcessBusy_t {
	int 					InProcess;
	unsigned long long int 	LastAction;
} MQTT_MsgProcessBusy;

static char MQTT_Msg_CheckForTimeout(unsigned char handler);
static void MQTT_Msg_Retrasmit(unsigned char handler);
static void MQTT_Msg_Discard(unsigned char handler);
static void MQTT_Msg_UpdateLastActionTime(unsigned char handler);
static void MQTT_Msg_SendPublish(unsigned char handler, int* bytesWritten);
static void MQTT_Msg_SetState(unsigned char handler, MQTT_Msg_State_t state);
static void MQTT_Msg_SendPuback(unsigned char handler);
static unsigned char MQTT_Msg_StoreMessage(MQTT_User_Message_t* MyMessage, MQTT_Msg_State_t state);
static signed short MQTT_Msg_GetMessHandler(int messageID);
static char MQTT_Msg_StateMachine(unsigned char handler);
static void MQTT_Msg_SendPubrec(unsigned char handler);
static void MQTT_Msg_SendPubrel(unsigned char handler);
static void MQTT_Msg_SendPubcomp(unsigned char handler);
static void MQTT_Msg_ReceivedPublish(unsigned char handler);
static void MQTT_Msg_ExecuteMessage(unsigned char handler);
static char MQTT_Msg_TimeoutMsgInProgress();
static void MQTT_Msg_SetMsgInProgress();
static void MQTT_Msg_SetLastTimeMsgInProgress();
static void MQTT_Msg_ProcessSubscription(unsigned char handler);
static char MQTT_Msg_IsMsgPending(unsigned char handler);
static unsigned short MQTT_Msg_GetFreeMID();




	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////



/**
*  @brief  Processes Messages from buffer
*
*  Should be called frequently. Processes messages from buffer.
*  Message buffer holds messages scheduled for sending and received messages.
*
*  @return 1 if there is still messages for processing
*/
char MQTT_Msg_Process(){
	char i, r;
	unsigned int cnt = 0;

	MQTT_BytesWrtitten = 0;

	for (i = 0; i < MQTT_API_MSG_BUFFER; i ++)
	{
		r = MQTT_Msg_StateMachine(i);
		if (r == 255)
			return 1;	// still processing messages

		cnt += (unsigned int) r;
		if (MQTT_BytesWrtitten > MQTT_MSG_MAX_BYTES_TO_WRITE)
			break;
	}

	if ((cnt > 0) || (MQTT_MsgProcessBusy.InProcess))
		return 1;   	// still processing messages
	else
		return 0;		// empty buffer
}

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
char MQTT_Msg_PrepareForSend(MQTT_User_Message_t* MyMessage){
	MyMessage->messageType = PUBLISH_MESSAGE;
	MyMessage->messageID = MQTT_Msg_GetFreeMID();
	return MQTT_Msg_StoreMessage(MyMessage, MQTT_MSG_STATE_READY_TO_SEND);
}

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
char MQTT_Msg_PrepareForSub(MQTT_User_Message_t* MyMessage){
	MyMessage->messageType = SUBSCRIBE_MESSAGE;
	MyMessage->messageID = MQTT_Msg_GetFreeMID();
	return MQTT_Msg_StoreMessage(MyMessage, MQTT_MSG_STATE_READY_TO_SUBSCRIBE);
}

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
char MQTT_Msg_PrepareForUnsub(MQTT_User_Message_t* MyMessage){
	MyMessage->messageType = UNSUBSCRIBE_MESSAGE;
	MyMessage->messageID = MQTT_Msg_GetFreeMID();
	return MQTT_Msg_StoreMessage(MyMessage, MQTT_MSG_STATE_READY_TO_UNSUBSCRIBE);
}

/**
 *  @brief  Discards all messages in buffer and clears flags
 *
 *  @return void
 */
void MQTT_Msg_DiscardAllMsg(){
	char i;

	for (i = 0; i < MQTT_API_MSG_BUFFER; i ++)
	{
		if (MQTT_Msg_IsMsgPending(i))
			MQTT_Msg_Discard(i);
	}

	memset((void *) &MQTT_MsgProcessBusy, 0, sizeof(MQTT_MsgProcessBusy));
}

/**
*  @brief  Clear Message in progress flag
*
*  Clear in progress flag. After this new messages will be processed.
*
*  @return void
*/
void MQTT_Msg_ClearMsgInProgress(){
	MQTT_MsgProcessBusy.InProcess = 0;
}




	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////



/**
*  @brief  MQTT state machine, processes messages from buffer
*
*  All messages that are received or scheduled for sending, are stored in buffer
*  State machine processes state of each message and act accordingly.
*
*  Should be called frequently when mqtt is running
*
*  @param  MAessage handler
*
*  @return 0 if empty, 1 if action performed
*/
static char MQTT_Msg_StateMachine(unsigned char handler){

	MQTT_Msg_TimeoutMsgInProgress();

	switch (MQTT_Api_Messages[handler].MQTT_MsgState)
	{	// empty message (no action)
	case MQTT_MSG_STATE_EMPTY :
		return 0;
		break;

		// message is ready for publishing
	case MQTT_MSG_STATE_READY_TO_SEND :
		{
			int bytesWritten;

			MQTT_Msg_SendPublish(handler, &bytesWritten);
			if (bytesWritten < 0)
				return 255;
			MQTT_BytesWrtitten += bytesWritten;
		}
		break;

		// message is ready for subscription
	case MQTT_MSG_STATE_READY_TO_SUBSCRIBE :
		if (MQTT_Api_Messages[handler].MQTT_MyMessage.messageType == SUBSCRIBE_MESSAGE)
		{
			MQTT_Api_SubscribeTopic(&MQTT_Api_Messages[handler].MQTT_MyMessage);
			MQTT_Msg_UpdateLastActionTime(handler);
			MQTT_Msg_SetState(handler, MQTT_MSG_STATE_SUBACK_WAITING);

			MQTT_BytesWrtitten += 600;
		}
		else
			MQTT_Msg_Discard(handler);
		break;

		// message is ready for unsubscription
	case MQTT_MSG_STATE_READY_TO_UNSUBSCRIBE :
		if (MQTT_Api_Messages[handler].MQTT_MyMessage.messageType == UNSUBSCRIBE_MESSAGE)
		{
			MQTT_Api_UnsubscribeTopic(&MQTT_Api_Messages[handler].MQTT_MyMessage);
			MQTT_Msg_UpdateLastActionTime(handler);
			MQTT_Msg_SetState(handler, MQTT_MSG_STATE_UNSUBACK_WAITING);

			MQTT_BytesWrtitten += 600;

			MQTT_Msg_Discard(handler);			// if we do not want to wait unsuback
		}
		else
			MQTT_Msg_Discard(handler);
		break;

		// waiting for the response from server on my message
	case MQTT_MSG_STATE_PUBACK_WAITING :
	case MQTT_MSG_STATE_PUBREC_WAITING :
	case MQTT_MSG_STATE_PUBCOMP_WAITING :
	case MQTT_MSG_STATE_SUBACK_WAITING :
	case MQTT_MSG_STATE_UNSUBACK_WAITING :
		switch (MQTT_Msg_CheckForTimeout(handler))
		{
		case 0 :	// still waiting ...
			break;

		case 1 :    // retransmit last message (server did not respond)
			switch (MQTT_Api_Messages[handler].MQTT_MyMessage.messageType)
			{
			case PUBLISH_MESSAGE :
				MQTT_Msg_Retrasmit(handler);
				break;

			case SUBSCRIBE_MESSAGE :
				MQTT_Api_Messages[handler].MQTT_MyMessage.dup = 1;
				MQTT_Api_SubscribeTopic(&MQTT_Api_Messages[handler].MQTT_MyMessage);
				break;

			case UNSUBSCRIBE_MESSAGE :
				MQTT_Api_Messages[handler].MQTT_MyMessage.dup = 1;
				MQTT_Api_UnsubscribeTopic(&MQTT_Api_Messages[handler].MQTT_MyMessage);
				break;
			}
			break;

		case 2 :	// discard last message (too much time have passed)
			MQTT_Msg_Discard(handler);
			break;
		}
		break;

		// we are waiting for PUBREL response (qos = 2)
	case MQTT_MSG_STATE_PUBREL_WAITING :
		if (MQTT_Msg_CheckForTimeout(handler) == 2)
			MQTT_Msg_Discard(handler);
		break;

		// send PUBACK, as acknowledge on message (qos = 1)
	case MQTT_MSG_STATE_PUBACK_READY_TO_SEND :
		// acknowledge reception and discard message
		MQTT_Msg_SendPuback(handler);
		MQTT_Msg_Discard(handler);
		break;

		// send PUBREC (qos = 2)
	case MQTT_MSG_STATE_PUBREC_READY_TO_SEND :
		MQTT_Msg_SendPubrec(handler);
		MQTT_Msg_UpdateLastActionTime(handler);
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_PUBREL_WAITING);
		break;

		// send PUBCOMP (qos = 2)
	case MQTT_MSG_STATE_PUBCOMP_READY_TO_SEND :
		MQTT_Msg_SendPubcomp(handler);
		MQTT_Msg_Discard(handler);
		break;

		// send PUBREL for message (qos = 2)
	case MQTT_MSG_STATE_PUBREL_READY_TO_SEND :
		MQTT_Msg_SendPubrel(handler);
		MQTT_Msg_UpdateLastActionTime(handler);
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_PUBCOMP_WAITING);
		break;

		// received publish message from server
	case MQTT_MSG_STATE_PUBLISH_RECEIVED :
		MQTT_Msg_ReceivedPublish(handler);
		MQTT_Msg_SetMsgInProgress();
		break;

		// handle acknowledge messages from server
	case MQTT_MSG_STATE_SUBACK_RECEIVED :
//	case MQTT_MSG_STATE_UNSUBACK_RECEIVED :
	case MQTT_MSG_STATE_PUBACK_RECEIVED :
		MQTT_Msg_Discard(handler);
		break;

		// received PUBCOMP for the message (qos = 2)
	case MQTT_MSG_STATE_PUBCOMP_RECEIVED :
		MQTT_Msg_Discard(handler);
		break;

		// received PUBREL (qos = 2)
	case MQTT_MSG_STATE_PUBREL_RECEIVED :
		MQTT_Msg_ExecuteMessage(handler);
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_PUBCOMP_READY_TO_SEND);
		MQTT_Msg_UpdateLastActionTime(handler);
		break;

	default :
		break;
	}

	return 1;
}

/**
*  @brief  Loads message after reception into buffer for further processing
*
*  After message is received save it into buffer for later process
*
*  @param  MQTT message struct
*
*  @return Message handler
*/
char MQTT_Msg_ProcessRecvMsg(MQTT_User_Message_t* MyMessage){
	return MQTT_Msg_StoreMessage(MyMessage, MQTT_MSG_STATE_PUBLISH_RECEIVED);
}

/**
*  @brief  Process received response from mqtt and update appropriate message status
*
*  Processes response from the mqtt server and sets appropriate message state.
*
*  @param  Message handler
*
*  @return void
*/
void MQTT_Msg_ProcessResponse(int msgType, int msgID, int msgDup){
	signed short handler;

	if ((handler = MQTT_Msg_GetMessHandler(msgID)) < 0)
		return;

	switch (msgType)
	{
	case PUBACK :
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_PUBACK_RECEIVED);
		break;

	case PUBREC :
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_PUBREL_READY_TO_SEND);
		break;

	case PUBREL :
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_PUBREL_RECEIVED);
		break;

	case PUBCOMP :
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_PUBCOMP_RECEIVED);
		break;

	case SUBACK :
		MQTT_Msg_ProcessSubscription(handler);
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_SUBACK_RECEIVED);
		break;

	case UNSUBACK :
		MQTT_Msg_ProcessSubscription(handler);
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_UNSUBACK_RECEIVED);
		break;

	default :
		break;
	}
}

/**
 *  @brief  Checks if desired message is pending for processing
 *
 *  Checks status of the message. If message with desired handler is empty
 *  it will return 0.
 *
 *  @param  Message handler
 *
 *  @return 1 if msg is pending,  0 if there is no msg
 */
static char MQTT_Msg_IsMsgPending(unsigned char handler){
	if (MQTT_Api_Messages[handler].MQTT_MsgState != MQTT_MSG_STATE_EMPTY)
		return 1;
	return 0;
}

/**
*  @brief  Discards desired message in buffer
*
*  Should be called when publish was successful, or msg was processed.
*  It will delete message from buffer.
*
*  @param  Message handler
*
*  @return void
*/
static void MQTT_Msg_Discard(unsigned char handler){
	// delete message, and set message state to empty
	memset( (void *) &MQTT_Api_Messages[handler], 0, sizeof(MQTT_Api_Msg_t));
}

/**
*  @brief  Updates time of last action performed
*
*  Used to determine timeouts for each message and action.
*
*  @param  Message handler
*
*  @return void
*/
static void MQTT_Msg_UpdateLastActionTime(unsigned char handler){
	MQTT_Api_Messages[handler].TimeOfLastAction = MSTimerGet();
}

/**
*  @brief  Publish message from buffer
*
*  Publish message with desired handler from buffer.
*  Depending on desired QOS, send message and go to next state.
*  In case QOS = 0, just send the message and discard it.
*  In case QOS = 1, send the message and go to wait for puback state
*  In case WOS = 2, send the message and go to wait for pubrec state
*
*  @param  Message handler
*  @param  retur value of written bytes
*
*  @return void
*/
static void MQTT_Msg_SendPublish(unsigned char handler, int* bytesWritten){
	// publish msg
	*bytesWritten = MQTT_User_Publish(&MQTT_Api_Messages[handler].MQTT_MyMessage);

	switch (MQTT_Api_Messages[handler].MQTT_MyMessage.qos)
	{
	case 0 :
		// delete msg (we do not need it any nore)
		MQTT_Msg_Discard(handler);
		break;
	case 1 :
		// wait for puback, and set time of last action
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_PUBACK_WAITING);
		MQTT_Msg_UpdateLastActionTime(handler);
		break;
	case 2 :
		// wait for pubrec, and set time of last action
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_PUBREC_WAITING);
		MQTT_Msg_UpdateLastActionTime(handler);
		break;
	}
}

/**
*  @brief  Checks for timeout from previous action
*
*  Checks if the timeout occurred from last actions for the message.
*  Detects retransmit timeout (action should be repeated), and max number of retransmits.
*
*  @param  Message handler
*
*  @return 0 - no timeout; 1 - retransmit timeout, 2 - we should discard message
*/
static char MQTT_Msg_CheckForTimeout(unsigned char handler){

	if (MQTT_Api_Messages[handler].retransmittions > MQTT_MSG_DISCARD_AFTER_RETRANSMITS)
	{
		return 2;
	}
	else if (MSTimerDelta(MQTT_Api_Messages[handler].TimeOfLastAction) > MQTT_MSG_RETRASMIT_TIMEOUT)
	{
		MQTT_Msg_UpdateLastActionTime(handler);
		MQTT_Api_Messages[handler].retransmittions ++;
		return 1;
	}
	else
		return 0;
}

/**
*  @brief  Retransmit last message with duplicate flag
*
*  Should be called when retransmission timeout occur for desired message.
*  Set duplicate flag and send the message again.
*
*  @param  Message handler
*
*  @return void
*/
static void MQTT_Msg_Retrasmit(unsigned char handler){
	switch (MQTT_Api_Messages[handler].MQTT_MsgState)
	{
	// if pubrec was not detected
	case MQTT_MSG_STATE_PUBREC_WAITING :
		// send duplicate msg
	case MQTT_MSG_STATE_PUBACK_WAITING :
		{
			int bytesWritten;
			MQTT_Api_Messages[handler].MQTT_MyMessage.dup = 1;
			MQTT_Msg_SendPublish(handler, &bytesWritten);
			MQTT_BytesWrtitten += bytesWritten;
		}
		break;
		// send duplicate of pubrel message
	case MQTT_MSG_STATE_PUBCOMP_WAITING :
		MQTT_Api_Messages[handler].MQTT_MyMessage.dup = 1;
		MQTT_Msg_SendPubrel(handler);
		break;

	default :
		break;
	}
}

/**
*  @brief  Sets state for desired msg in buffer
*
*  @param  Message handler
*  @param  Desired new message state
*
*  @return void
*/
static void MQTT_Msg_SetState(unsigned char handler, MQTT_Msg_State_t state){
	MQTT_Api_Messages[handler].MQTT_MsgState = state;
}

/**
*  @brief  Sends puback to server as acknowledge for the received message
*
*  Should be called when received message is processed to acknowledge reception.
*  QOS = 1
*
*  @param  Message handler
*
*  @return void
*/
static void MQTT_Msg_SendPuback(unsigned char handler){
	MQTT_User_SendPUBACK(MQTT_Api_Messages[handler].MQTT_MyMessage.messageID);
}

/**
*  @brief  Sends pubrec to server as acknowledge for the received message
*
*  Should be called when received message is processed to acknowledge reception.
*  QOS = 2
*
*  @param  Message handler
*
*  @return void
*/
static void MQTT_Msg_SendPubrec(unsigned char handler){
	MQTT_User_SendPUBREC(MQTT_Api_Messages[handler].MQTT_MyMessage.messageID);
}

/**
*  @brief  Sends pubcomp to server as acknowledge for the pubrel
*
*  Should be called when pubrel is received
*	QOS = 2
*
*  @param  Message handler
*
*  @return void
*/
static void MQTT_Msg_SendPubcomp(unsigned char handler){
	MQTT_User_SendPUBCOMP(MQTT_Api_Messages[handler].MQTT_MyMessage.messageID);
}

/**
*  @brief  Sends pubrel to server as acknowledge for the pubrec
*
*  Should be called when received message is processed to acknowledge pubrec
*  QOS = 2
*
*  @param  Message handler
*
*  @return void
*/
static void MQTT_Msg_SendPubrel(unsigned char handler){
	MQTT_User_SendPUBREL(MQTT_Api_Messages[handler].MQTT_MyMessage.dup,
							MQTT_Api_Messages[handler].MQTT_MyMessage.messageID);
}

/**
*  @brief  Publish reception.
*
*  Should be called when publish message is received. Handles message according to its QOS level.
*  If the previous message is still in progress it will return (message will be processed later).
*
*  @param  Message handler
*
*  @return void
*/
static void MQTT_Msg_ReceivedPublish(unsigned char handler){
	// message in progress
	if (MQTT_Msg_TimeoutMsgInProgress() == 1)
		return;

	switch (MQTT_Api_Messages[handler].MQTT_MyMessage.qos)
	{
	case 0 :
		MQTT_Msg_ExecuteMessage(handler);
		MQTT_Msg_Discard(handler);
		break;

	case 1 :
		MQTT_Msg_ExecuteMessage(handler);
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_PUBACK_READY_TO_SEND);
		MQTT_Msg_UpdateLastActionTime(handler);
		break;

	case 2 :
		MQTT_Msg_SetState(handler, MQTT_MSG_STATE_PUBREC_READY_TO_SEND);
		MQTT_Msg_UpdateLastActionTime(handler);
		break;
	}
}

/**
*  @brief  Call user defined function for processing msgs received from mqtt server
*
*  Invoke callback function for processing received messages.
*  And updates message in progress time parameter.
*
*  @param  Message handler
*
*  @return void
*/
static void MQTT_Msg_ExecuteMessage(unsigned char handler){
	MQTT_Api_ProcessReceivedMessage(&MQTT_Api_Messages[handler].MQTT_MyMessage);
	MQTT_Msg_SetLastTimeMsgInProgress();
}

/**
*  @brief  Saves current message into first empty slot in buffer
*
*  Finds first empty slot in message buffer and saves message.
*  Sets starting state of the message.
*  Returns message buffer index as message handler.
*
*  @param  MQTT message struct
*  @param  Desired message state
*
*  @return Message handler, 255 if buffer is full
*/
static unsigned char MQTT_Msg_StoreMessage(MQTT_User_Message_t* MyMessage, MQTT_Msg_State_t state){
	unsigned char count = 255;

	// find first empty message slot in buffer
	for (count = 0; count < MQTT_API_MSG_BUFFER; count ++)
	{
		if (MQTT_Api_Messages[count].MQTT_MsgState == MQTT_MSG_STATE_EMPTY)
			break;
	}

	if (count == MQTT_API_MSG_BUFFER)
		return 255;           // error no more space for messages

	// save message
	MQTT_Api_Messages[count].MQTT_MsgState 	  = state;
	memcpy((void *) &MQTT_Api_Messages[count].MQTT_MyMessage, (const void *) MyMessage, sizeof(MQTT_User_Message_t));
	MQTT_Api_Messages[count].TimeOfLastAction = MSTimerGet();

	return count;
}

/**
*  @brief  Get unused message ID
*
*  Look in message buffer for currently unused message ID.
*
*  @return Message ID
*/
static unsigned short MQTT_Msg_GetFreeMID(){
	unsigned char i, j, success;

	for (i = 1; i < MQTT_API_MSG_BUFFER + 1; i ++)
	{
		success = 0;
		for (j = 0; j < MQTT_API_MSG_BUFFER; j ++)
		{
			if (i != MQTT_Api_Messages[j].MQTT_MyMessage.messageID)
				success ++;
		}
		if (success == MQTT_API_MSG_BUFFER)
			return i;
	}
	return 0xFFFF;
}

/**
*  @brief  Gets message handler for given message ID
*
*  @param  Message ID
*
*  @return Message handler
*/
static signed short MQTT_Msg_GetMessHandler(int messageID){
	signed short count;

	for (count = 0; count < MQTT_API_MSG_BUFFER; count ++)
	{
		if (MQTT_Api_Messages[count].MQTT_MyMessage.messageID == messageID)
			return count;
	}

	return -1;
}

/**
*  @brief
*/
static void MQTT_Msg_ProcessSubscription(unsigned char handler){
	MQTT_Api_ProcessSubscription(MQTT_Api_Messages[handler].MQTT_MyMessage.topicStr);
}

/**
*  @brief  Set Message in progress flag
*
*  After processing new message, set in progress flag.
*  No new messages will be processed until we get response on this message
*
*  @return void
*/
static void MQTT_Msg_SetMsgInProgress(){
	MQTT_MsgProcessBusy.InProcess = 1;
}

/**
*  @brief  Set time of last action to detect in progress timeout.
*
*  @return void
*/
static void MQTT_Msg_SetLastTimeMsgInProgress(){
	MQTT_MsgProcessBusy.LastAction = MSTimerGet();
}

/**
*  @brief  Process in progress timeout
*
*  Checks time since in progress flag is set. If timeout is detected,
*  appropriate action should be done and in progress flag should be cleared.
*
*  @return In progress flag
*/
static char MQTT_Msg_TimeoutMsgInProgress(){
	if (MQTT_MsgProcessBusy.InProcess)
	{
		if (MSTimerDelta(MQTT_MsgProcessBusy.LastAction) > MQTT_MSG_RESPONSE_WAIT_TIMEOUT)
		{
			MQTT_OnMsgResponseTimeout();
			MQTT_Msg_ClearMsgInProgress();   // timeout
		}
	}

	return MQTT_MsgProcessBusy.InProcess;
}
