/** @file   Commmon_Default.h
 *  @brief  File contains common common declarations used in entire project
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#ifndef COMMON_DEFAULTS_H_
#define COMMON_DEFAULTS_H_


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// firmware and hardware revision

#define KINETIS_FIRMWARE_REV 				"1.0.0"
#define MAIN_BOARD_HW_REV    				"1.02"


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#define __SSL__								// use SLL for connection


#define __SLEEP__


//#define __USE_DEFAULTS__                   // if defined, settings bellow will be used if no configs are loaded


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// wunderbar configuration struct

typedef struct wcfg_tag {
  struct {
    uint8_t id[40];
    uint8_t security[30];
  } wunderbar;
  struct {
	uint8_t ssid[30];
	uint8_t password[30];
  } wifi;
  struct {
	uint8_t url[30];
	uint8_t ip[30];
  } cloud;
} wcfg_t;

typedef struct {
	uint8_t pass_htu[256];
	uint8_t pass_gyro[256];
	uint8_t pass_light[256];
	uint8_t pass_mic[256];
	uint8_t pass_bridge[256];
	uint8_t pass_ir[256];
} ble_pass_t;

extern wcfg_t wunderbar_configuration;


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// default settings

#define DEFAULT_SSID         				"MikroE Public"
#define DEFAULT_PASSWORD     				"mikroe.guest"
#define DEFAULT_USERNAME     				"d8b08377-bcfc-49bd-9a2f-2ca0b1b006b2"
#define DEFAULT_SECURITY     				"NEs84oMuHttA"
#define DEFAULT_MQTT_SERVER_URL 			"mqtt.relayr.io"


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// fixed Access Point configuration

#define AP_PARAMETER_SSID    				"Wunderbar AP"
#define AP_PARAMETER_IP      				"192.168.1.1"
#define AP_PARAMETER_SUBNET  				"255.255.255.0"
#define AP_PARAMETER_PORT    				"8010"


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// fixed MQTT configuration

#ifdef __SSL__
#define MQTT_RELAYR_SERVER_PORT  			"8883"             // tcp-ssl
#else
#define MQTT_RELAYR_SERVER_PORT  			"1883"             // no tcp-ssl
#endif

#define MQTT_RELAYR_SERVER_PING_PORT    	80
#define MQTT_RELAYR_SERVER_PING_ADDRESS 	"/ping"
#define MQTT_RELAYR_SERVER_GET_CERT_PORT 	80
#define MQTT_RELAYR_SERVER_GET_CERT_ADDRESS "/cacert.der"


#define MQTT_SERVER_RESPONSE_TIMEOUT 		20000
#define MQTT_MQTTVERSION             		3
#define MQTT_KEEPALIVEINTERVAL       		60
#define MQTT_PING_INTERVAL 					55
#define MQTT_CLEANSESSION            		1

#define MQTT_MSG_OPT_QOS_PUB               	0
#define MQTT_MSG_OPT_DUP 					0
#define MQTT_MSG_OPT_RETAINED				0
#define MQTT_MSG_OPT_QOS_SUB				0

#define MQTT_TOPIC_PREFIX            		"/v1"
#define WUNDERBAR_SECURITY_LENGTH  			12

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// fixed wifi configs

#define WIFI_DEFAULT_CHANNEL				""
#define WIFI_DEFAULT_WEPID_CFG        		"0"
#define WIFI_DEFAULT_CONNTYPE_CFG     		"0"
#define WIFI_DEFAULT_DHCPDENABLED_CFG 		"1"
#define WIFI_DEFAULT_SECURITY_CFG     		"0"


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#endif // COMMON_DEFAULTS_H_