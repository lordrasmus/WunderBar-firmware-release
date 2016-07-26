/** @file   GS_Api_TCP.c
 *  @brief  File contains functions to handle GS module http and tcp connections.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */


#include <string.h>
#include <stdint.h>
#include "GS_Api_TCP.h"

// static declarations

static uint8_t* GS_Api_Cid[CID_COUNT];                ///< Array of pointers to cids



	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////



/**
*  @brief  Cloase all connections
*
*  @return void
*/
void GS_Api_CloseAll(){
	uint8_t i;

	GS_API_CloseAllConnections();
	for (i = 0; i < CID_COUNT; i ++)
		if (GS_Api_Cid[i])
			*GS_Api_Cid[i] = GS_API_INVALID_CID;

	memset(GS_Api_Cid, 0, sizeof(GS_Api_Cid));
}

/**
*  @brief  Disconnects from current TCP socket
*
*  @param  Connection id
*
*  @return void
*/
void GS_Api_Disconnect(char cid){
	// Close the client CID
	if(cid != GS_API_INVALID_CID)
	{
		GS_API_CloseConnection(cid);
		if (GS_Api_Cid[(uint8_t) cid])
			*GS_Api_Cid[(uint8_t) cid] = GS_API_INVALID_CID;
	}
}

/**
*  @brief  Close current http connection
*
*  @param  Connection id
*
*  @return void
*/
void GS_Api_Http_Close(char cid){
	// Close the client CID
	if(cid != GS_API_INVALID_CID)
	{
		GS_Api_HttpCloseConn(cid);
		if (GS_Api_Cid[(uint8_t) cid])
			*GS_Api_Cid[(uint8_t) cid] = GS_API_INVALID_CID;
	}
}

/**
*  @brief  Sends desired buffer content over established TCP socket
*
*  Send buffer data over desired socket.
*
*  @param  Connection id
*  @param  Output buffer
*  @param  Data length
*
*  @return True or false for successful transmission
*/
bool GS_Api_TCP_Send(char cid, char* send_buff, int send_buff_len){

	// Check for a valid Client Connection
  	if(cid != GS_API_INVALID_CID)
  	{
		if(!GS_API_SendTcpData((uint8_t) cid, (uint8_t*) send_buff, send_buff_len))
			return false;
		else
			return true;
	}

  	return false;
}

/**
*  @brief  Starts TCP client on desired server IP and port.
*
*  Start tcp client. Open tcp connection on desired server and desired port
*
*  @param  Pointer to connection ID
*  @param  Server port
*  @param  Server IP
*  @param  Function pointer to data handler
*
*  @return void
*/
void GS_Api_TCP_StartTcpClient(char* cid, char* serverIp, char* serverPort, GS_API_DataHandler DataHandler){
	uint8_t temp_cid;
	// Check to see if we are already connected
	if(*cid != (char) GS_API_INVALID_CID)
	{
		GS_API_Printf("TCP Client Already Started");
		return;
	}

	// We're not connected, so lets create a TCP client connection
	temp_cid = GS_API_CreateTcpClientConnection((char*)serverIp, (char*)serverPort, DataHandler);
	if (temp_cid != GS_API_INVALID_CID)
	{
		*cid = temp_cid;
		GS_Api_Cid[temp_cid] = (uint8_t*) cid;
	}
}

/**
*  @brief  Starts TCP server on desired  port
*
*  Start tcp server on desired port.
*
*  @param  Pointer to connection ID
*  @param  Server port
*  @param  Function pointer to data handler
*
*  @return void
*/
void GS_Api_TCP_StartTcpServer(char* cid, char* serverPort, GS_API_DataHandler DataHandler){
	uint8_t temp_cid;

	// Check to see if we are already connected
	if(*cid != (char) GS_API_INVALID_CID)
		return;

	// We're not connected, so lets create a TCP server connection
	temp_cid = GS_API_CreateTcpServerConnection((char*)serverPort, DataHandler);
	if (temp_cid != GS_API_INVALID_CID)
	{
		*cid = temp_cid;
		GS_Api_Cid[temp_cid] = (uint8_t*) cid;
	}
}

/**
*  @brief  Open http connection
*
*  Open http connection on desired host ip and host port.
*
*  @param  Pointer to connection ID
*  2param  Host ip
*  @param  Host port
*  @param  Function pointer to data handler
*
*  @return void
*/
void GS_Api_Http_Open(uint8_t* cid, char* host, int hostPort, GS_API_DataHandler cidDataHandler){
	uint8_t temp_cid;

	// Check to see if we are already connected
	if(*cid != (char) GS_API_INVALID_CID)
		return;

	// We're not connected, so lets create a TCP server connection
	temp_cid = GS_Api_HttpClientOpen(host, hostPort, cidDataHandler);
	if (temp_cid != GS_API_INVALID_CID)
	{
		*cid = temp_cid;
		GS_Api_Cid[temp_cid] = (uint8_t*) cid;
	}
}
