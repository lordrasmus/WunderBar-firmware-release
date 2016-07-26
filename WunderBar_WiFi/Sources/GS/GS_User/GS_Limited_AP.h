/** @file   GS_Limited_AP.h
 *  @brief  File contains functions to handle Limited Access Point feature
 *  		of GS module.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <stdbool.h>

// public functions

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
bool GS_LAP_StartServer(char* provSsid, char* ip, char* subnetMask);

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
bool GS_LAP_SendPacket(char* buf, int buflen);

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
int GS_LAP_GetData(char* buf, int count);

/**
*  @brief  Resets library incoming buffer pointer
*
*  Reset incoming buffer pointer and returns number of available bytes for read
*
*  @param  Return value for number of bytes available for read
*
*  @return void
*/
void GS_LAP_ResetIncomingBuffer(unsigned int* count);

/**
*  @brief  Start TCP server on desired port
*
*  @param  Desired server port
*
*  @return True if successful
*/
bool GS_LAP_StartTcpServer(char* serverPort);

/**
*  @brief  Get Buffer for WiFi client data
*
*  Return pointer to buffer where incoming data from WiFi client is stored.
*
*  @return Pointer to first byte in buffer
*/
char* GS_LAP_GetBuffer();

/**
*  @brief  Process completed bulk transfer event.
*
*  Will be called when bulk data is completely received from matching cid.
*  After this we should process data.
*/
bool GS_LAP_CompletedBulkTransfer(uint8_t cid);

/**
*  @brief  Close Wifi tcp client connection
*
*  @return void
*/
void GS_LAP_CloseClientConnection();

/**
*  @brief  Returns wifi tcp client cid
*
*  @return cid for tcp client connection
*/
char GS_LAP_GetClientCid();
