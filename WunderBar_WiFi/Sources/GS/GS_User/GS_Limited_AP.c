/** @file   GS_Limited_AP.c
 *  @brief  File contains functions to handle Limited Access Point feature
 *  		of GS module.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include "../AT/AtCmdLib.h"
#include "GS_Api_TCP.h"
#include "GS_Limited_AP.h"


// static declarations

#define LIMITED_AP_BUF_MAX_SIZE 1024

static uint8_t tcpServerCID = GS_API_INVALID_CID; ///< Connection ID for TCP client
static uint8_t tcpServerClientCID = GS_API_INVALID_CID;

static uint8_t Limitted_AP_buffer[LIMITED_AP_BUF_MAX_SIZE];
static uint8_t*  Limitted_AP_bufferPtr = Limitted_AP_buffer;


void GS_TCP_Server_HandleData(uint8_t cid, uint8_t data);



	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////



/**
*  @brief  Start Limited Access Point
*
*  Start Limited access point server on GS1500M module
*  User should provide ssid, server ip, and subnet mask
*
*  @param  AP ssid
*  @param  AP ip
*  @param  AP subnet
*
*  @return True if successful
*/
bool GS_LAP_StartServer(char* provSsid, char* ip, char* subnetMask){
	return GS_API_StartProvisioning(provSsid, "", ip, subnetMask, "");
}

/**
*  @brief  Returns wifi tcp client cid
*
*  @return cid for tcp client connection
*/
char GS_LAP_GetClientCid(){
	return ((char) tcpServerClientCID);
}

/**
*  @brief  Close Wifi tcp client connection
*
*  @return void
*/
void GS_LAP_CloseClientConnection(){
	if (tcpServerClientCID != GS_API_INVALID_CID)
	{
		GS_Api_Disconnect(tcpServerClientCID);
		tcpServerClientCID = GS_API_INVALID_CID;
	}
}

/**
*  @brief  Resets library incoming buffer pointer
*
*  Reset incoming buffer pointer and returns number of available bytes for read
*
*  @param  Return value for number of bytes available for read
*
*  @return void
*/
void GS_LAP_ResetIncomingBuffer(unsigned int* count){
	*count = Limitted_AP_bufferPtr - Limitted_AP_buffer;

	Limitted_AP_bufferPtr = Limitted_AP_buffer;
}

/**
*  @brief  Reads data from library input buffer
*
*  Reads desired number of bytes from incoming buffer.
*
*  @param  Pointer to destination buffer
*  @param  Number of bytes to read
*
*  @return Number of bytes read
*/
int GS_LAP_GetData(char* buf, int count){
	int data = 0;

	while (count --)
	{
		if ((Limitted_AP_bufferPtr - Limitted_AP_buffer) < LIMITED_AP_BUF_MAX_SIZE)
		{
			*buf ++ = *Limitted_AP_bufferPtr ++;
			data ++;
		}
	}
	return data;
}

/**
*  @brief  Start TCP server on desired port
*
*  @param  Desired server port
*
*  @return True if successful
*/
bool GS_LAP_StartTcpServer(char* serverPort){
	GS_Api_TCP_StartTcpServer((char *) &tcpServerCID, serverPort, GS_TCP_Server_HandleData);

	if(tcpServerCID != GS_API_INVALID_CID){
		return true;
	}
	return false;
}

/**
*  @brief  Send packet over TCP socket to client
*
*  Sends packet to tcp client over established connection.
*
*  @param  Pointer to buffer
*  @param  Packet length
*
*  @return True or false depending on successful action
*/
bool GS_LAP_SendPacket(char* buf, int buflen){
	if (!GS_Api_TCP_Send((char) tcpServerClientCID, buf, buflen))
	{
		tcpServerClientCID = GS_API_INVALID_CID;
		return false;
	}

	return true;
}

/**
*  @brief  Process completed bulk transfer event.
*
*  Will be called when bulk data is completely received from matching cid.
*  After this we should process data.
*/
bool GS_LAP_CompletedBulkTransfer(uint8_t cid){
	if (GS_API_INVALID_CID != tcpServerCID)
	{
		tcpServerClientCID = cid;
		return true;
	}
	return false;
}

/**
*  @brief  Get Buffer for WiFi client data
*
*  Return pointer to buffer where incoming data from WiFi client is stored.
*
*  @return Pointer to first byte in buffer
*/
char* GS_LAP_GetBuffer(){
	return ((char *) &Limitted_AP_buffer[0]);
}




	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////



/**
*  @brief  Handles incoming data for the TCP Server
*
*  Receives bulk data from tcp client and stores them in buffer.
*
*  @param  Connection ID
*  @param  Byte received
*
*  @return void
*/
void GS_TCP_Server_HandleData(uint8_t cid, uint8_t data){
	// Save the data to the line buffer
	*Limitted_AP_bufferPtr++ = data;
}
