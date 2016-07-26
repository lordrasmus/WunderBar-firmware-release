/** @file   GS_Http.h
 *  @brief  File contains functions to handle Http client connecton
 *  		Stores received bytes and process them.
 *  		Http is used to perform get request to ping server (get time)
 *  		and to get latest certificate.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <stdbool.h>

#define HTTP_CLIENT_USER_A  	"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebkit/534.7 (KHTML, like Gecko) Chrome/7.0.517.44 Safari/534.7"
#define HTTP_CLIENT_CON_TYPE  	"application/x-www-form-urlencoded"
#define HTTP_CLIENT_CONN      	"keep-alive"
#define HTPP_BUFFER_LENGTH      2048

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/**
*  @brief  Reads and parses certificate from http buffer. And saves cert into flash.
*
*  Should be called when http data is received
*
*  @return void
*/
bool GS_Http_DownloadCert();

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
bool GS_Http_Get(char* hostIp, int hostPort, char* page);

/**
*  @brief  Reads and parses time from http buffer. And loads time into GS module.
*
*  Should be called when http data is received
*
*  @return void
*/
bool GS_Http_LoadTime();

/**
*  @brief  Close http connection
*
*  @return void
*/
void GS_Http_CloseConn();

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
bool GS_Http_OnComplete(uint8_t cid);
