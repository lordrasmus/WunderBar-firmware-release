/** @file   GS_Http.c
 *  @brief  File contains functions to handle Http client connecton
 *  		Stores received bytes and process them.
 *  		Http is used to perform get request to ping server (get time)
 *  		and to get latest certificate.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <string.h>

#include "../AT/AtCmdlib.h"

#include "GS_Api_TCP.h"
#include "GS_Certificate.h"
#include "GS_Http.h"


// static declarations

static uint8_t httpClient_CID = GS_API_INVALID_CID; // Connection ID for http client
static char HttpBuffer[HTPP_BUFFER_LENGTH];         // here we will store data received from time server
static char* HttpBufferPtr = HttpBuffer;      		// pointer to TimeServerBuffer

static char GS_Http_Status = 0;

static void GS_Http_DataHandler(uint8_t cid, uint8_t data);
static void GS_Http_ResetIncomingBuffer();
static bool GS_Http_IsValidCid(uint8_t cid);
static bool GS_Http_SetHttp(char* serverIp);
static bool GS_Http_Open(char* host, int hostPort);
static bool GS_Http_ParseTime(char *timeStr);



	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////



/**
*  @brief  Process http on complete event
*
*  Should be called when http reception is completed
*  Set flag that data is ready for process.
*
*  @param  Cid of current http connection
*
*  @return True if successful
*/
bool GS_Http_OnComplete(uint8_t cid){
	if (cid == httpClient_CID)
	{
		GS_Http_Status = 1;
		return true;
	}
	return false;
}

/**
*  @brief  Close http connection
*
*  @return void
*/
void GS_Http_CloseConn(){
	GS_Api_Http_Close(httpClient_CID);
}

/**
*  @brief  Do http GET request
*
*  Perform http get request on desired host IP, port and page
*
*  @param  Host ip address (in format "xxx.xxx.xxx.xxx:)
*  @param  Host port
*  @param  Page to request (string)
*
*  @return True if successful
*/
bool GS_Http_Get(char* hostIp, int hostPort, char* page){
	// reset done flag
	GS_Http_Status = 0;
	// set http
	if (!GS_Http_SetHttp(hostIp))
		return false;
	// http open on desired ip and port
	if (!GS_Http_Open(hostIp, hostPort))
		return false;
	// GET request to obtain time
	if (!GS_Api_HttpGet(httpClient_CID, page))
	{
		GS_Http_CloseConn();
		return false;
	}

	return true;
}

/**
*  @brief  Reads and parses certificate from http buffer. And saves cert into flash.
*
*  Should be called when http data is received
*
*  @return void
*/
bool GS_Http_DownloadCert(){

	if (GS_Http_Status == 1)
	{
		uint32_t size;

		if (memcmp(HttpBuffer, "200 OK\r\n", 8) != 0)
			return false;

		size = HttpBufferPtr-HttpBuffer-8;
		memcpy((void *) (HttpBuffer+4), (void *) &size, 4);

		GS_Cert_StoreInFlash(HttpBuffer+4);
		GS_Http_CloseConn();
		return true;
	}

	return false;
}

/**
*  @brief  Reads and parses time from http buffer. And loads time into GS module.
*
*  Should be called when http data is received
*
*  @return void
*/
bool GS_Http_LoadTime(){
	char time[30];

	if (GS_Http_Status == 1)
	{
		GS_Http_CloseConn();
		GS_Http_ParseTime(time);
		return GS_API_SetTime((uint8_t *) time);
	}

	return false;
}


	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////

/**
*  @brief  Handles incoming data for the TCP Client
*
*  Just fills byte per byte incoming bytes from http connection
*
*  @param  Connection ID
*  @param  Byte received
*
*  @return void
*/
static void GS_Http_DataHandler(uint8_t cid, uint8_t data){
	// Save the data to the buffer
	if ((HttpBufferPtr - &HttpBuffer[0]) < HTPP_BUFFER_LENGTH)
		*HttpBufferPtr ++ = data;
}

/**
*  @brief  Resets library incoming buffer pointer
*
*  @return void
*/
static void GS_Http_ResetIncomingBuffer(){

	HttpBufferPtr = &HttpBuffer[0];
}

/**
*  @brief  Check if the return cid is in valid range
*
*  @return True if cid is valid
*/
static bool GS_Http_IsValidCid(uint8_t cid){
	if ((cid >= 0) && (cid <= 16))
		return true;
	else
		return false;
}

/**
*  @brief  Setup http parameters.
*
*  Set up predefined http parameters.
*
*  @param  server ip address
*
*  @return True if successful
*/
static bool GS_Http_SetHttp(char* serverIp){

	if (GS_Api_HttpClientConfig( ATLIBGS_HTTP_HE_USER_A, HTTP_CLIENT_USER_A) == false)
		return false;

	if (GS_Api_HttpClientConfig( ATLIBGS_HTTP_HE_CON_TYPE, HTTP_CLIENT_CON_TYPE) == false)
		return false;

	if (GS_Api_HttpClientConfig( ATLIBGS_HTTP_HE_CONN, HTTP_CLIENT_CONN ) == false)
		return false;

	if (GS_Api_HttpClientConfig( ATLIBGS_HTTP_HE_HOST, serverIp ) == false)
		return false;

	return true;
}

/**
*  @brief  Opens http connection on desired host ip address and port
*
*  @param  Host ip address (string XXX.XXX.XXX.XXX)
*  @param  Host port
*
*  @return True if successful
*/
static bool GS_Http_Open(char* host, int hostPort){
	GS_Http_ResetIncomingBuffer();

	GS_Api_Http_Open(&httpClient_CID, host, hostPort, GS_Http_DataHandler);

	if(httpClient_CID == GS_API_INVALID_CID){
		return false;
	}

	return GS_Http_IsValidCid(httpClient_CID);
}

/**
*  @brief  Parse time response message
*
*  Parses time response message required by pinging mqtt ip
*
*  @param  Return time string
*
*  @return True if successful
*/
static bool GS_Http_ParseTime(char *timeStr){
	char* ptr;
	const char text_del[] = "(UTC)";

	if (memcmp(HttpBuffer, "200 OK\r\n", 8) != 0)
		return false;

	ptr = strstr((const char *) HttpBuffer, text_del);

	if (ptr != NULL)
	{
		strcpy(timeStr, (const char *) (ptr + strlen(text_del)));
		return true;
	}

	return false;
}
