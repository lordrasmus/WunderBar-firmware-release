/**
 * @file GS_API_network.c
 *
 * Methods for handling network related functionality 
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "GS_API.h"
#include "GS_API_private.h"
#include "../AT/AtCmdLib.h"
#include <hardware/Hw_modules.h>


/** Private Macros **/
#define TIMEOUT_RESPONSE_INTERVAL_HIGH    30000
#define TIMEOUT_RESPONSE_INTERVAL_LOW      5000
#define CID_INT_TO_HEX(cidInt) (cidIntToHex[cidInt]) ///< Converts integer value to hex ascii character

/** Private Variables **/
static const char cidIntToHex[17] = "0123456789ABCDEF"; ///< Lookup table for integer to hex ascii conversion
static GS_API_DataHandler cidDataHandlers[CID_COUNT]; ///< Array of function pointers for CID data handlers
static char tcpServerClientPort[PORT_STRING_LENGTH]; ///< port of incoming TCP server client connection
static char tcpServerClientIp[IP_STRING_LENGTH]; ///< IP address of incoming TCP server client connection

/** Private Method Declaration **/
static uint8_t gs_api_parseCidStr(uint8_t* cidStr);
static bool gs_api_handle_cmd_resp(HOST_APP_MSG_ID_E msg);
static bool GS_Api_IsCidValid(uint8_t cid);
static void gs_api_setCidDataHandler(uint8_t cid, GS_API_DataHandler cidDataHandler);
static GS_API_DataHandler gs_api_getCidDataHandler(uint8_t cid);

/** Public Method Implementation **/

/**
*  @brief  Disassociate from current WiFI network
*
*  @return void
*/
void GS_API_DisconnectNetwork(void){
     AtLibGs_DisAssoc();
}

/**
*  @brief  Set up WiFi network
*
*  initialize and set up WiFi parameters for desired WiFi network
*
*  @param  Network Configuration struct
*
*  @return True if successful
*/
bool GS_API_SetupWifiNetwork(HOST_APP_NETWORK_CONFIG_T* apiNetCfg) {
     bool set = false;
     uint8_t security = atoi(apiNetCfg->security);
     // Set all the configuration options

     // Check for DHCP Enabled
     if (atoi(apiNetCfg->dhcpEnabled)) {
          if (!gs_api_handle_cmd_resp(AtLibGs_DHCPSet(1)))
          {
        	  return set;
          }
     } else {
          if (!gs_api_handle_cmd_resp(AtLibGs_DHCPSet(0)))
               return set;

          if (!gs_api_handle_cmd_resp(
                   AtLibGs_IPSet((int8_t *) apiNetCfg->staticIp,
                                 (int8_t *) apiNetCfg->subnetMask,
                                 (int8_t *) apiNetCfg->gatewayIp)))
               return set;
     }

     // Check security
     switch (security) {
     case 0:
          // Set all
          // Don't check these since some might not be valid anyway
          if (!gs_api_handle_cmd_resp(
        		  AtLibGs_SetPassPhrase((int8_t *) apiNetCfg->passphrase)))
        	  return set;

          if (!gs_api_handle_cmd_resp(AtLibGs_SetAuthMode(0)))
        	  return set;
          break;

     case 1:
          // Set none
          break;

     case 2:
          // Set WEP
          if (!gs_api_handle_cmd_resp(
                   AtLibGs_SetWepKey(atoi(apiNetCfg->wepId),
                		   	   	   	   	   apiNetCfg->wepKey)))
               return set;

          if (!gs_api_handle_cmd_resp(AtLibGs_SetAuthMode(0)))
               return set;
          break;

     case 4:
     case 8:
     case 16:
     case 32:
     case 64:
          // Set WPA
          if (!gs_api_handle_cmd_resp(AtLibGs_SetPassPhrase((int8_t *) apiNetCfg->passphrase)))
               return set;
          break;

     default:
          // nothing
          break;
     }

     // Set the security mode
     if(!gs_api_handle_cmd_resp(AtLibGs_SetSecurity(security)))
          return set;

     // Set adhoc or infrastructure
     if(!gs_api_handle_cmd_resp(AtLibGs_Mode(atoi(apiNetCfg->connType))))
          return set;

     return set;
}

/**
*  @brief  Join WiFi netwrk
*
*  @param  Network Configuration struct
*
*  @return True if successfully joined
*/
bool GS_API_JoinWiFiNetwork(HOST_APP_NETWORK_CONFIG_T* apiNetCfg){
	bool joined = false;
	unsigned int response_timeout_temp;

	response_timeout_temp = GS_Api_GetResponseTimeoutHandle();
	GS_Api_SetResponseTimeoutHandle(TIMEOUT_RESPONSE_INTERVAL_HIGH);

	AtLib_FlushIncomingMessage();
	// Associate
	if(gs_api_handle_cmd_resp(AtLibGs_Assoc((int8_t *) apiNetCfg->ssid,(int8_t*)"", (int8_t *) apiNetCfg->channel)))
	{
	     if (gs_api_handle_cmd_resp(AtLibGs_DHCPSet(1)))
	     {
	    	 char ip[16];

	    	 if (AtLib_ParseWlanConnIp((uint8_t *) ip) == true)
	    		 joined = true;
	     }
	     // Clear all data handler functions
	     memset(cidDataHandlers, 0, sizeof(cidDataHandlers));
	}

	GS_Api_SetResponseTimeoutHandle(response_timeout_temp);
	return joined;
}

/**
*  @brief Start provisioning
*
*  Used to start Limited Access Point
*
*  @param  AP ssid
*  @param  AP WiFi channel
*  @param  AP Host IP (string)
*  @param  AP Host subnet mask (string)
*  @param  AP Host name
*
*  @return True if successful
*/
bool GS_API_StartProvisioning(char* provSsid, char* provChannel, char* ip,
								char* subnetMask, char* hostName) {

	 GS_Api_SetResponseTimeoutHandle(TIMEOUT_RESPONSE_INTERVAL_HIGH);

     // Disable DHCP
     if (!gs_api_handle_cmd_resp(AtLibGs_DHCPSet(0)))
          return false;

     // Set Static IP
     if (!gs_api_handle_cmd_resp(
              AtLibGs_IPSet((int8_t*) ip, (int8_t*) subnetMask, (int8_t*) ip)))
          return false;

     // Enable DHCP Server
     if (!gs_api_handle_cmd_resp(AtLibGs_SetDHCPServerMode(1)))
    	 return false;

     // Enable Limited AP
     if (!gs_api_handle_cmd_resp(AtLibGs_Mode(2)))
          return false;

     // Set SSID and Channel
     if (!gs_api_handle_cmd_resp(
              AtLibGs_Assoc((int8_t*) provSsid, (int8_t*) "",
                            (int8_t*) provChannel)))
          return false;

     /* Reset the receive buffer */
     AtLib_FlushRxBuffer();

     GS_Api_SetResponseTimeoutHandle(TIMEOUT_RESPONSE_INTERVAL_LOW);

     return true;
}

/**
*  @brief  Stop provisioning
*
*  @return void
*/
void GS_API_StopProvisioning(void){
     // Web client can't be shut off, so just reset the device
     gs_api_handle_cmd_resp(AtLibGs_Reset());     
}

/**
*  @brief  Checks WiFi network status
*
*  Calls AT+WSTATUS command and parses standard response.
*  After this call, AtLib_ParseSSIDResponse or AtLib_ParseNodeIpAddress
*  should be called to parse node ip or target ssid
*
*  @return True if successful
*/
bool GS_API_WlanStatus(){
     return (gs_api_handle_cmd_resp(AtLibGs_WlanConnStatShort()));
}

/**
*  @brief  Checks if the GS module is associated on the WiFi network.
*
*  Checks Wlan status, and compares ssid string of WiFi network with given ssid.
*  If they match returns true.
*
*  @param  ssid string
*
*  @return True if successful
*/
bool GS_API_IsAssociated(uint8_t* wifi_ssid){
	char ssid[32];

	if (GS_API_WlanStatus())
	{
		if (AtLib_ParseSSIDResponse((uint8_t *) ssid))
		{
			if (strcmp((const char *) ssid, (const char *) wifi_ssid) == 0)
				return true;
		}
	}

	return false;
}

/**
*  @brief  Create Udp Server connection
*
*  @param  Host port
*  @param  Function pointer to data handler function
*
*  @return Connection id
*/
uint8_t GS_API_CreateUdpServerConnection(char* port, GS_API_DataHandler cidDataHandler){
     uint8_t cidStr[] = " ";
     uint16_t cid = GS_API_INVALID_CID;

     if(!gs_api_handle_cmd_resp(AtLibGs_UdpServer_Start((int8_t*)port)))
          return cid;

     if(AtLib_ParseUdpServerStartResponse(cidStr))
     {
          // need to convert from ASCII to numeric
          cid = gs_api_parseCidStr(cidStr);
          if(cid != GS_API_INVALID_CID){
               cidDataHandlers[cid] = cidDataHandler;
          }
     }

     return cid;
}

/**
*  @brief  Create Udp client connection
*
*  @param  Server IP (string)
*  @param  Server Port (string)
*  @param  Local port (string)
*  @param  Function pointer to data handler function
*
*  @return Connection id
*/
uint8_t GS_API_CreateUdpClientConnection(char* serverIp, char* serverPort, char* localPort, GS_API_DataHandler cidDataHandler){
     uint8_t cidStr[] = " ";
     uint16_t cid = GS_API_INVALID_CID;

     if(!gs_api_handle_cmd_resp(AtLibGs_UdpClientStart((int8_t*) serverIp, (int8_t*) serverPort, (int8_t*) localPort)))
          return cid;

     if(AtLib_ParseUdpServerStartResponse(cidStr)){
          // need to convert from ASCII to numeric
          cid = gs_api_parseCidStr(cidStr);
          if(cid != GS_API_INVALID_CID){
               cidDataHandlers[cid] = cidDataHandler;
          }
     }

     return cid;
}

/**
*  @brief  Create Tcp Server Connection
*
*  @param  Host port (string)
*  @param  Function pointer to data handler function
*
*  @return Connection id
*/
uint8_t GS_API_CreateTcpServerConnection(char* port, GS_API_DataHandler cidDataHandler){
     uint8_t cidStr[] = " ";
     uint16_t cid = GS_API_INVALID_CID;
 	 unsigned int response_timeout_temp;

   	 response_timeout_temp = GS_Api_GetResponseTimeoutHandle();
 	 GS_Api_SetResponseTimeoutHandle(TIMEOUT_RESPONSE_INTERVAL_HIGH);

     if(!gs_api_handle_cmd_resp(AtLibGs_TcpServer_Start((int8_t*)port)))
     {
    	  GS_Api_SetResponseTimeoutHandle(response_timeout_temp);
          return cid;
     }

     if(AtLib_ParseTcpServerStartResponse(cidStr)){
          // need to convert from ASCII to numeric
          cid = gs_api_parseCidStr(cidStr);
          if(cid != GS_API_INVALID_CID){
               cidDataHandlers[cid] = cidDataHandler;
          }
     }

     GS_Api_SetResponseTimeoutHandle(response_timeout_temp);
     return cid;
}

/**
*  @brief  Create Tcp client connection
*
*  @param  Server IP (string)
*  @param  Server Port (string)
*  @param  Function pointer to data handler function
*
*  @return Connection id
*/
uint8_t GS_API_CreateTcpClientConnection(char* serverIp, char* serverPort, GS_API_DataHandler cidDataHandler){
     uint8_t cidStr[] = " ";
     uint16_t cid = GS_API_INVALID_CID;
 	 unsigned int response_timeout_temp;

   	 response_timeout_temp = GS_Api_GetResponseTimeoutHandle();
 	 GS_Api_SetResponseTimeoutHandle(TIMEOUT_RESPONSE_INTERVAL_HIGH);

     if(!gs_api_handle_cmd_resp(AtLibGs_TcpClientStart((int8_t*) serverIp, (int8_t*) serverPort)))
     {
    	  GS_Api_SetResponseTimeoutHandle(response_timeout_temp);
          return cid;
     }
     if(AtLib_ParseTcpServerStartResponse(cidStr)){
          // need to convert from ASCII to numeric
          cid = gs_api_parseCidStr(cidStr);
          if(cid != GS_API_INVALID_CID){
               cidDataHandlers[cid] = cidDataHandler;
          }
     }

     GS_Api_SetResponseTimeoutHandle(response_timeout_temp);
     return cid;
}

/**
*  Send udp data
*
*  @param  Connection id
*  @param  Data buffer
*  @param Data length
*
*  @return True if successful
*/
bool GS_API_SendUdpClientData(uint8_t cid, uint8_t* dataBuffer, uint16_t dataLength){
     return AtLib_BulkDataTransfer(CID_INT_TO_HEX(cid), dataBuffer, dataLength) == HOST_APP_MSG_ID_ESC_CMD_OK;
}

/**
*  Send TCP data
*
*  @param  Connection id
*  @param  Data buffer
*  @param Data length
*
*  @return True if successful
*/
bool GS_API_SendTcpData(uint8_t cid, uint8_t* dataBuffer, uint16_t dataLength){
     return ((AtLib_BulkDataTransfer(CID_INT_TO_HEX(cid), dataBuffer, dataLength)) == HOST_APP_MSG_ID_ESC_CMD_OK);
}

/**
*  @brief  Send UDP data (as server) to last connected client
*
*  @param  Connection id
*  @param  Data buffer
*  @param  Data length
*
*  @return void
*/
bool GS_API_SendUdpServerDataToLastClient(uint8_t cid, uint8_t* dataBuffer, uint16_t dataLength){
     char ipAddress[HOST_APP_RX_IP_MAX_SIZE];
     char port[HOST_APP_RX_PORT_MAX_SIZE];

     // Get the last UDP client information
     AtLib_GetUdpServerClientConnection(ipAddress, port);
     // Send the data to the UDP client
     AtLib_UdpServerBulkDataTransfer(cid, ipAddress, port, dataBuffer, dataLength);
     return true;
}

/**
*  @brief  Close single connection
*
*  Close current connection defined by cid
*
*  @param  Connection id
*
*  @return void
*/
void GS_API_CloseConnection(uint8_t cid){
     AtLibGs_Close(CID_INT_TO_HEX(cid));
     cidDataHandlers[cid] = NULL;
}

/**
*  @brief  Close All Connections
*
*  Close all connections and delete all data handlers
*
*  @return void
*/
void GS_API_CloseAllConnections(){
     AtLibGs_CloseAll();
     memset(cidDataHandlers, 0, sizeof(cidDataHandlers));
}

/**
*  @brief  Handle error messages from GS module
*
*  @param  Error message
*
*  @return void
*/
void GS_API_HandleErrorMessage(HOST_APP_MSG_ID_E error_message){
	App_HandleErrorMessage(error_message);
}

void GS_API_CheckForData(void){
     uint8_t rxData;
     HOST_APP_MSG_ID_E Error_Message;

     /* Read one byte at a time - Use non-blocking call */
     while (GS_HAL_recv(&rxData, 1, 0)) {
          /* Process the received data */
          switch(Error_Message = AtLib_ReceiveDataProcess(rxData)){
          case HOST_APP_MSG_ID_TCP_SERVER_CLIENT_CONNECTION:
          {
               uint8_t cidServerStr[] = " ";
               uint8_t cidClientStr[] = " ";
               uint8_t cidServer = GS_API_INVALID_CID;
               uint8_t cidClient = GS_API_INVALID_CID;

               if(AtLib_ParseTcpServerClientConnection((char *)cidServerStr, (char *)cidClientStr, tcpServerClientIp, tcpServerClientPort)){
                    cidServer = gs_api_parseCidStr(cidServerStr);
                    cidClient = gs_api_parseCidStr(cidClientStr);
                    gs_api_setCidDataHandler(cidClient, gs_api_getCidDataHandler(cidServer));
               }
               GS_API_Printf("TCP Server Client Connection %d, %d, %s, %s\r\n", cidServer, cidClient, tcpServerClientIp, tcpServerClientPort);
          }
          break;

          case HOST_APP_MSG_ID_RESPONSE_TIMEOUT:
          case HOST_APP_MSG_ID_NONE:
               // Do nothing
               break;

          default:
        	  GS_API_HandleErrorMessage(Error_Message);
        	  break;
          }
     }
}

/**
*  @brief  Get IP address from response
*
*  Issues Wlan connection Status command, and if successful, extracts user IP address.
*
*  @param  Pointer to IP address string
*
*  @return True if successful
*/
bool GS_API_GetIPAddress(uint8_t* ipAddr){
     if(AtLibGs_WlanConnStat() != HOST_APP_MSG_ID_OK){
          return false;
     }
  
     return AtLib_ParseIpAddress((char *) ipAddr);
}
  
/**
*  @brief  Do DNS resolve for desired url address
*
*  Issues Wlan connection Status command, and if successful, extracts user IP address.
*
*  @param  Desired url
*  @param  Return value of host IP
*
*  @return True if successful
*/
bool GS_API_DNSResolve(int8_t* url, uint8_t* hostIPaddr){
	 unsigned int response_timeout_temp;

  	 response_timeout_temp = GS_Api_GetResponseTimeoutHandle();
	 GS_Api_SetResponseTimeoutHandle(TIMEOUT_RESPONSE_INTERVAL_HIGH);

     if (AtLibGs_DNSLookup(url) != HOST_APP_MSG_ID_OK){
    	 GS_Api_SetResponseTimeoutHandle(response_timeout_temp);
    	 return false;
     }

     GS_Api_SetResponseTimeoutHandle(response_timeout_temp);
     return AtLib_ParseDNSLookupResponse((uint8_t*) hostIPaddr);
}

/**
*  @brief  Set up max retries socket options for desired socket
*
*  @param  Connection id
*  @param  Max retries in seconds
*
*  @return True if successful
*/
bool GS_Api_SetUpSocket_MaxRT(uint8_t cid, uint32_t max_rt){

	return gs_api_handle_cmd_resp(AtLib_SetSocketOptions(cid, ATLIB_SOCKET_OPTION_TYPE_TCP, ATLIB_SOCKET_OPTION_PARAM_TCP_MAXRT, max_rt));
}

/**
*  @brief  Set up keep alive socket options for desired socket
*
*  @param  Connection id
*  @param  Keep alive in seconds
*
*  @return True if successful
*/
bool GS_Api_SetUpSocket_TcpKeepAlive(uint8_t cid, uint32_t keepalive){

	if (!gs_api_handle_cmd_resp(AtLib_SetSocketOptions(cid, ATLIB_SOCKET_OPTION_TYPE_SOCK, ATLIB_SOCKET_OPTION_PARAM_SO_KEEPALIVE, 1)))
		return false;

	if (!gs_api_handle_cmd_resp(AtLib_SetSocketOptions(cid, ATLIB_SOCKET_OPTION_TYPE_TCP, ATLIB_SOCKET_OPTION_PARAM_TCP_KEEPALIVE_COUNT, 1)))
		return false;

	return gs_api_handle_cmd_resp(AtLib_SetSocketOptions(cid, ATLIB_SOCKET_OPTION_TYPE_TCP, ATLIB_SOCKET_OPTION_PARAM_TCP_KEEPALIVE, keepalive));
}

/**
*  @brief  Get cid info
*
*  Calls AT+CID=? command
*  Send get cid info command.
*  Does not parse result.
*
*  @return True if successful
*/
bool GS_Api_GetCidInfo(){
	return gs_api_handle_cmd_resp(AtLib_GetCIDInfo());
}

/**
*  @brief  Get memory trace info
*
*  Calls AT+MEMTRACE command
*  Send get memory trace info command.
*  Does not parse result.
*
*  @return True if successful
*/
bool GS_Api_getMemoryInfo(){
	return gs_api_handle_cmd_resp(AtLib_GetMemoryInfo());
}


// SSL/HTTPS  configuration

/**
*  @brief  Load certificate into GS module
*
*  Calls AT+TCERTADD=<Name>,<Format>,<Size>,<Location><CR><ESC>W<data of size above> command,
*  and parses standard response.
*  Load desired certificate with desired name into GS module
*
*  @param  Desired cert name
*  @param  Certificate size
*  @param  Certificate array
*
*  @return True if successful
*/
bool GS_API_LoadCertificate(uint8_t* cert_name, uint32_t cert_size, uint8_t* cacert){
	return gs_api_handle_cmd_resp(AtLib_AddSSLCertificate((char *) cert_name, 0 /*binary*/, cert_size, 1 /*RAM*/, (char *) cacert));
}

/**
*  @brief  Remove certificate from GS module
*
*  Calls AT+TCERTDEL=<certificate name> command, and parses standard response.
*  removes certificate from GS module memory
*
*  @param  Desired cert name
*
*  @return True if successful
*/
bool GS_API_RemoveCertificate(uint8_t* cert_name){
	return gs_api_handle_cmd_resp(AtLib_DeleteSSLCertificate((char *) cert_name));
}

/**
*  @brief  Open SSL secured connection
*
*  Calls AT+SSLOPEN=<cid>,<cert name> command, and parses standard response.
*  Open SSL secured connection on desired tcp socket
*
*  @param  Connection id
*  @param  Certificate name
*
*  @return True if successful
*/
bool GS_API_OpenSSLconnection(uint8_t cid, uint8_t* cert_name){
	unsigned int response_timeout_temp;
	bool result;

 	response_timeout_temp = GS_Api_GetResponseTimeoutHandle();
	GS_Api_SetResponseTimeoutHandle(TIMEOUT_RESPONSE_INTERVAL_HIGH);

	result = gs_api_handle_cmd_resp(AtLib_SSLOpen(cid, (char*) cert_name));
	GS_Api_SetResponseTimeoutHandle(response_timeout_temp);

	return result;
}

/**
*  @brief  Close SSL secured connection
*
*  Calls AT+SSLCLOSE=<cid> command, and parses standard response.
*  Close SSL secured connection on desired tcp socket
*
*  @param  Connection id
*
*  @return True if successful
*/
bool GS_API_CloseSSLconnection(uint8_t cid){
	return gs_api_handle_cmd_resp(AtLib_SSLClose(cid));
}




/**
*  @brief  Set time into GS module
*
*  Calls AT+SETTIME=[<dd/mm/yyyy>,<HH:MM:SS>],[ System time in milliseconds since epoch(1970)]
*  command, and parses standard response.
*
*  @param  Desired time string in format
*  "dd/mm/yyyy,HH:MM:SS" or
*  ",xxxxxxxxxxxxx"
*
*  @return True if successful
*/
bool GS_API_SetTime(uint8_t* time){
	return gs_api_handle_cmd_resp(AtLibGs_SetTime((int8_t *) time));
}

/**
*  @brief  Returns system time from GS module
*
*  Calls AT+GETTIME=? command and parses response.
*
*  @param  Return time string. Time will be in milliseconds since epoch
*
*  @return True if successful
*/
bool GS_Api_GetSystemTime(char* timeStr){

	if (gs_api_handle_cmd_resp(AtLib_GetTime()))
		if (AtLib_ParseSystemTime(timeStr) == 1)
		{
			return true;
		}
	return false;
}

/**
*  @brief  Http config function
*
*  Calls AT+HTTPCONF=<Param>,<Value> command and parses standard response
*
*  @param  Http parameter type
*  @param  Parameter string
*
*  @return True if successful
*/
bool GS_Api_HttpClientConfig(int parm, char* val){

	return (gs_api_handle_cmd_resp(AtLib_HTTPConf(parm, val)));
}

/**
*  @brief  Open http client connection
*
*  Calls AT+HTTPOPEN=<host >[, <Port Number>, <SSL Flag>, <certificate name>,<proxy>,<Connection Timeout>,<client certificate name>,<client key name>]
*  command in order to open http client connection.
*  Parses for OK response from GS module and returns connection id.
*
*  @param  Host ip string
*  @param  Host port
*  @param  Function pointer to data handler
*
*  @return Connection id
*/
uint8_t GS_Api_HttpClientOpen(char* host, int hostPort, GS_API_DataHandler cidDataHandler){
    uint8_t cid = GS_API_INVALID_CID;
	 unsigned int response_timeout_temp;

  	 response_timeout_temp = GS_Api_GetResponseTimeoutHandle();
	 GS_Api_SetResponseTimeoutHandle(TIMEOUT_RESPONSE_INTERVAL_HIGH);

	if (!gs_api_handle_cmd_resp(AtLibGs_HTTPOpen(host, hostPort, &cid)))
	{
		GS_Api_SetResponseTimeoutHandle(response_timeout_temp);
		return GS_API_INVALID_CID;
	}
	if (GS_Api_IsCidValid(cid))
		cidDataHandlers[cid] = cidDataHandler;

	GS_Api_SetResponseTimeoutHandle(response_timeout_temp);
    return cid;
}

/**
*  @brief  Close http client connection
*
*  Calls AT+HTTPCLOSE=<CID> command, and parses standard response.
*  Will close http connection with desired cid.
*
*  @param  Connection id
*
*  @return True if successful
*/
bool GS_Api_HttpCloseConn(uint8_t cid){
	return (gs_api_handle_cmd_resp(AtLib_HTTPClose(cid)));
}

/**
*  @brief  Issue Http get over desired cid and web page
*
*  Calls AT+HTTPSEND=<CID>,<Type>,<Timeout>,<Page>[,Size of the content]
*  command and parses standrad response
*
*  @param  Connection id
*  @param  web page string
*
*  @return true if successful
*/
bool GS_Api_HttpGet(uint8_t cid, char* page){
	unsigned int response_timeout_temp;
	bool result;

 	response_timeout_temp = GS_Api_GetResponseTimeoutHandle();
	GS_Api_SetResponseTimeoutHandle(TIMEOUT_RESPONSE_INTERVAL_HIGH);

	result = (AtLibGs_HTTPSend(cid, ATLIBGS_HTTPSEND_GET, 10, page, 0, 0));
	GS_Api_SetResponseTimeoutHandle(response_timeout_temp);

	return result;
}

/**
*  @brief  Set Gpio output pins on GS module (drive led)
*
*  Calls AT+DGPIO=<GPIO-NO>,<SET/RESET(0/1) command and parses standard response
*
*  @param  GPIO number
*  @param  GPIO state
*
*  @return True if successful
*/
static bool GS_Api_GpioSetState( ATLIB_GPIO_PIN_E gpio, ATLIB_GPIO_STATE_E state ){
	return (gs_api_handle_cmd_resp(AtLib_SetGPIO(gpio, state)));
}

bool GS_Api_Gpio30_Set(bool state){
	if (state == true)
		return (GS_Api_GpioSetState(ATLIBGS_GPIO30, ATLIBGS_HIGH));
	else
		return (GS_Api_GpioSetState(ATLIBGS_GPIO30, ATLIBGS_LOW));
}


static bool GS_Api_IsCidValid(uint8_t cid){
	if ((cid >= 0) && (cid <= 16))
		return true;
	else
		return false;
}

bool GS_Api_EnableSoftFlowControl(char mode){
	return (gs_api_handle_cmd_resp(AtLib_EnableSoftFlowControl((uint32_t) mode)));
}

void GS_Api_SetResponseTimeoutHandle(unsigned int timeout){
	AtLib_Response_Handle_Timeout = timeout;
}

unsigned int GS_Api_GetResponseTimeoutHandle(){
	return AtLib_Response_Handle_Timeout;
}

/**
*  @brief  Parse cid in DISCONNECT response
*
*  Should be called when DISCONNECT message is caught.
*  Parses response and returns cid.
*
*  @return Connection id
*/
uint8_t GS_Api_ParseDisconnectCid(){
	uint8_t cidStr[] = " ";
	uint8_t cid = GS_API_INVALID_CID;

    if(AtLib_ParseTcpServerStartResponse(cidStr))
    {
         // need to convert from ASCII to numeric
         cid = gs_api_parseCidStr(cidStr);
    }
    return cid;
}



/** Private Method Implementation **/

/**
   @brief AtCmdLib calls this function for all incoming data

   @param cid Incoming data connection ID
   @param rxData Incoming data value
*/
void App_ProcessIncomingData(uint8_t cid, uint8_t rxData) {
     // Check for and call handler for incoming CID and Data
     if(cid < CID_COUNT && cidDataHandlers[cid])
          cidDataHandlers[cid](cid, rxData);
     else
          GS_API_Printf("RX Data with no handler for cid %d\r\n", cid);
}

/**
   @brief Retrieves the data handler for the connection ID
   @private
   @param cid Connection ID to retrieve the data handler function pointer for
   @return Data Handler function pointer for CID, or null if it doesn't exist
*/
static GS_API_DataHandler gs_api_getCidDataHandler(uint8_t cid){
     if(cid < CID_COUNT){
          return cidDataHandlers[cid];
     } else {
          return NULL;
     }
}

/**
   @brief Sets the data handler for the connection ID
   @private
   @param cid Connection ID to set the data handler function pointer for
   @param cidDataHandler Function pointer for handling CID data
*/
static void gs_api_setCidDataHandler(uint8_t cid, GS_API_DataHandler cidDataHandler){
     if(cid < CID_COUNT){
          cidDataHandlers[cid] = cidDataHandler;
     }
}

/**
   @brief Converts a cid in ascii to an integer

   For example, converts 'e' or 'E' to 14
   @private
   @param cidStr CID in ascii
   @return CID as an integer, or GS_API_INVALID_CID if it couldn't be converted
*/
static uint8_t gs_api_parseCidStr(uint8_t* cidStr){
     unsigned int  cid = GS_API_INVALID_CID;

     if(sscanf((char*)cidStr, "%x",(unsigned int*) &cid)){
          if(cid >= CID_COUNT){
               cid = GS_API_INVALID_CID;
          }
     }
     return (uint8_t)cid;
}

/**
   @brief Checks for a OK response to a command
   @private
   @param msg Message returned by command sent to Gainspan module
   @return true if command response was OK, false if not
*/
static bool gs_api_handle_cmd_resp(HOST_APP_MSG_ID_E msg) {

     if (msg != HOST_APP_MSG_ID_OK) {
          GS_API_Printf("CMD ERR %d", msg);
          return false;
     } else {
          return true;
     }

}
