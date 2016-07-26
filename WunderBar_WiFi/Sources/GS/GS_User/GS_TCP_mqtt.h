/** @file   GS_TCP_mqtt.h
 *  @brief  File contains functions to handle tcp connection
 *  		for the purposes of mqtt.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <stdbool.h>

// public functions

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
bool GS_TCP_mqtt_StartTcpTask(char* server_ip, char* server_port);

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
bool GS_Api_mqtt_SendPacket(char* buf, int buflen);

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
bool GS_Api_mqtt_CompletedBulkTransfer(char cid);

/**
*  @brief  Gets pointer to current client CID for established TCP connection
*
*  @return cid
*/
char GS_TCP_mqtt_GetClientCID();

/**
*  @brief  Close TCP connection which we use for mqtt
*
*  Closes SSL connection and then close tcp connection.
*
*  @return void
*/
void GS_TCP_mqtt_Disconnect();

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
int  GS_TCP_mqtt_GetData(char* buf, int count);

/**
*  @brief  Reset incoming buffer
*
*  Reset incoming buffer. Check if there are unprocessed bytes and copy them at the begging of the buffer.
*  This will happen only if mqtt package has been split into two tcp packages.
*
*  @return void
*/
void GS_TCP_mqtt_ResetBuffer();

/**
*  @brief  Get remaining bytes in tcp buffer
*
*  @return Bytes left for reading
*/
int GS_TCP_mqtt_GetRemBytes();

/**
*  @brief  Set last successful position
*
*  Set last successful read position when complete mqtt package is read.
*  Next mqtt process should go from this position.
*
*  @return void
*/
void GS_TCP_mqtt_UpdatePtr();
