/** @file   GS_Api_TCP.h
 *  @brief  File contains functions to handle GS module http and tcp connections.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <stdbool.h>
#include "../API/GS_API.h"

// public functios

/**
*  @brief  Disconnects from current TCP socket
*
*  @param  Connection id
*
*  @return void
*/
void GS_Api_Disconnect(char cid);

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
void GS_Api_TCP_StartTcpClient(char* cid, char* serverIp, char* serverPort, GS_API_DataHandler DataHandler);

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
void GS_Api_TCP_StartTcpServer(char* cid, char* serverPort, GS_API_DataHandler DataHandler);

/**
*  @brief  Cloase all connections
*
*  @return void
*/
void GS_Api_CloseAll();

/**
*  @brief  Close current http connection
*
*  @param  Connection id
*
*  @return void
*/
void GS_Api_Http_Close(char cid);

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
void GS_Api_Http_Open(uint8_t* cid, char* host, int hostPort, GS_API_DataHandler cidDataHandler);

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
bool GS_Api_TCP_Send(char cid, char* send_buff, int send_buff_len);
