/** @file   GS_TCP_mqtt.c
 *  @brief  File contains functions to handle tcp connection
 *  		for the purposes of mqtt.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */


#include <string.h>

#include "GS_Api_TCP.h"
#include "GS_User.h"


// static declarations

typedef struct {
	uint8_t  line[1500];
	uint16_t currentWritePos;
	uint16_t currentReadPos;
	uint16_t totalData;
	uint16_t lastSuccessPos;
	uint8_t  busy;
} TCP_Incoming_Buffer_t;

static TCP_Incoming_Buffer_t Client_TCP_Buffer;          // Buffer to store data that comes from mqtt server
static uint8_t tcpClientCID = GS_API_INVALID_CID; 		///< Connection ID for TCP client

static void GS_TCP_mqtt_HandleTcpClientData(uint8_t cid, uint8_t data);
static void GS_TCP_mqtt_ResetIncomingBuffer(unsigned int* count);


	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////



/**
*  @brief  Get remaining bytes in tcp buffer
*
*  @return Bytes left for reading
*/
int GS_TCP_mqtt_GetRemBytes(){
	return ((int) (Client_TCP_Buffer.totalData - Client_TCP_Buffer.currentReadPos));
}

/**
*  @brief  Set last successful position
*
*  Set last successful read position when complete mqtt package is read.
*  Next mqtt process should go from this position.
*
*  @return void
*/
void GS_TCP_mqtt_UpdatePtr(){
	Client_TCP_Buffer.lastSuccessPos = Client_TCP_Buffer.currentReadPos;
}

/**
*  @brief  Reset incoming buffer
*
*  Reset incoming buffer. Check if there are unprocessed bytes and copy them at the begging of the buffer.
*  This will happen only if mqtt package has been split into two tcp packages.
*
*  @return void
*/
void GS_TCP_mqtt_ResetBuffer(){
	int bytesRem;

	bytesRem = Client_TCP_Buffer.totalData - Client_TCP_Buffer.lastSuccessPos;

	if (bytesRem > 0)
		memcpy((void *) &Client_TCP_Buffer.line[0], (const void *) &Client_TCP_Buffer.line[Client_TCP_Buffer.lastSuccessPos], bytesRem);

	Client_TCP_Buffer.currentWritePos = bytesRem;
	Client_TCP_Buffer.currentReadPos = 0;
	Client_TCP_Buffer.totalData = bytesRem;
	Client_TCP_Buffer.lastSuccessPos = 0;
	Client_TCP_Buffer.busy  = 0;
}

/**
*  @brief  Reads data from library input buffer
*
*  Read desired number of bytes from input buffer.
*
*  @param  Pointer to destination buffer
*  @param  Number of bytes to read
*
*  @return Number of bytes read
*/
int GS_TCP_mqtt_GetData(char* buf, int count){
	int data = 0;
	// break if there is no enough bytes in buffer (next tcp packet should arrive promptly)
	if (count > GS_TCP_mqtt_GetRemBytes())
		return 0;

	// read requested number of bytes
	while (count --)
	{
		if (Client_TCP_Buffer.currentReadPos <= Client_TCP_Buffer.totalData)
		{
			*buf ++ = Client_TCP_Buffer.line[Client_TCP_Buffer.currentReadPos ++];
			data ++;
		}
	}

	return data;
}

/**
*  @brief  Gets pointer to current client CID for established TCP connection
*
*  @return cid
*/
char GS_TCP_mqtt_GetClientCID(){
	return (char) tcpClientCID;
}

/**
*  @brief  Close TCP connection which we use for mqtt
*
*  Closes SSL connection and then close tcp connection.
*
*  @return void
*/
void GS_TCP_mqtt_Disconnect(){
	if (tcpClientCID != GS_API_INVALID_CID)
	{
		GS_API_CloseSSLconnection(tcpClientCID);

		GS_Api_Disconnect(tcpClientCID);
	}
}

/**
*  @brief  Starts TCP task with desired server IP and server port
*
*  Open tcp connection as tcp client at desired server ip and port.
*
*  @param  Server port
*  @param  Server IP
*
*  @return True or false depending on successful action
*/
bool GS_TCP_mqtt_StartTcpTask(char* server_ip, char* server_port){

	GS_Api_TCP_StartTcpClient((char*) &tcpClientCID, server_ip, server_port, GS_TCP_mqtt_HandleTcpClientData);

	if(tcpClientCID != GS_API_INVALID_CID){
		return true;
	}
	return false;
}

/**
*  @brief  Send packet over TCP socket to mqtt server
*
*  Sends a package over established tcp connection.
*  If sending fails, try again before considering socket dead.
*
*  @param  Pointer to buffer
*  @param  Packet length
*
*  @return True or false depending on successful action
*/
bool GS_Api_mqtt_SendPacket(char* buf, int buflen){

	if (GS_Api_TCP_Send((char) tcpClientCID, buf, buflen) == false)
	{
		GS_API_CommWorking();
		GS_API_CommWorking();

		if (GS_Api_TCP_Send((char) tcpClientCID, buf, buflen) == false)
		{
			GS_Api_Disconnect(tcpClientCID);
			GS_ProcessMqttDisconnect();
			return false;
		}
	}

	return true;
}

/**
*  @brief  Process completed bulk transfer event.
*
*  Will be called when bulk data is completely received from matching cid.
*  After this we should process data.
*
*  @param  Connection id
*
*  @return True if successful
*/
bool GS_Api_mqtt_CompletedBulkTransfer(uint8_t cid){
	unsigned int cnt;

	if (cid == tcpClientCID)
	{
		GS_TCP_mqtt_ResetIncomingBuffer(&cnt);
		Client_TCP_Buffer.busy = 1;
		return true;
	}
	return false;
}



	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////




/**
*  @brief  Handles incoming data for the TCP Client
*
*  All incoming data bytes over tcp will be stored in buffer for matching cid.
*  Function will be called from AT lib and it will load byte per byte until reception is completed.
*
*  @param  Connection ID
*  @param  Byte received
*
*  @return void
*/
static void GS_TCP_mqtt_HandleTcpClientData(uint8_t cid, uint8_t data){
	// Save the data to the line buffer
	if (cid == tcpClientCID)
	{
		if (Client_TCP_Buffer.currentWritePos < 1500)
		{
			Client_TCP_Buffer.line[Client_TCP_Buffer.currentWritePos++] = data;
			Client_TCP_Buffer.totalData = 0;
		}
	}
}

/**
*  @brief  Resets library incoming buffer pointers
*
*  Reset buffer pointers and set total bytes in buffer available for read.
*
*  @param  return value for number of bytes for reading
*
*  @return void
*/
static void GS_TCP_mqtt_ResetIncomingBuffer(unsigned int* count){
	Client_TCP_Buffer.totalData = Client_TCP_Buffer.currentWritePos;

	Client_TCP_Buffer.currentWritePos = 0;
	Client_TCP_Buffer.currentReadPos = 0;

	*count = Client_TCP_Buffer.totalData;
}
