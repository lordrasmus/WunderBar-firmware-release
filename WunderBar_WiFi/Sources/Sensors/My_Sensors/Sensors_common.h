/** @file   Sensors_common.c
 *  @brief  File contains common functions used for handling messages
 *  		from and to all sensor boards
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */


#ifndef SENSORS_COMMON_H_
#define SENSORS_COMMON_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../wunderbar_common.h"
#include "../../JSON/JSON_Msg/JSON_Msg.h"


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// json message path constants

#define JSON_MSG_ID            "msg_id"
#define JSON_MSG_FREQ          "frequency"
#define JSON_MSG_CMD           "cmd"
#define JSON_MSG_GYRO          "gyro"
#define JSON_MSG_ACCEL         "accel"
#define JSON_MSG_LIGHT         "light"
#define JSON_MSG_PROX          "prox"
#define JSON_MSG_SOUND         "sound"
#define JSON_MSG_HYSTERESIS    "hy"
#define JSON_MSG_LOW           "lo"
#define JSON_MSG_HIGH          "hi"
#define JSON_MSG_RANGE         "rng"
#define JSON_MSG_PASSKEY       "pass"
#define JSON_MSG_TEMPERATURE   "temp"
#define JSON_MSG_HUMIDITY      "hum"
#define JSON_MSG_CONFIG        "sensorcfg"
#define JSON_MSG_RGBC_GAIN     "rgbc_gain"
#define JSON_MSG_PROX_DRIVE    "prox_drive"
#define JSON_MSG_DOWN_BRIDGE   "down_ch_payload"
#define JSON_MSG_UP_BRIDGE     "up_ch_payload"
#define JSON_MSG_BAUDRATE      "baudrate"
#define JSON_MSG_RESOLUTION    "resolution"


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// common message templates

#define Template_battery_level 	"{\"ts\":%s,\"val\":%s}"
#define Template_firmware_rev   "{\"ts\":%s,\"firmware\":\"%s\"}"
#define Template_hardware_rev   "{\"ts\":%s,\"hardware\":\"%s\"}"


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// functions

/**
 *  @brief  Fill threshold 16bit int struct from float values
 *
 *  Three threshold values in int format are represented as its real value multiplied by 100.
 *
 *  @param  Thresholf float struct
 *  @param  Return threshold 16bit int struct
 *
 *  @return void
 */
void Sensors_ConvertFloat_2_int16(threshold_float_t f_threshold, threshold_int16_t* t_int16);

/**
 *  @brief  Fill threshold 32bit int struct from float values
 *
 *  Three threshold values in int format are represented as its real value multiplied by 100.
 *
 *  @param  Thresholf float struct
 *  @param  Return threshold 32bit int struct
 *
 *  @return void
 */

/**
 *  @brief  Fill threshold 32bit int struct from float values
 *
 *  Three threshold values in int format are represented as its real value multiplied by 100.
 *
 *  @param  Thresholf float struct
 *  @param  Return threshold 32bit int struct
 *
 *  @return void
 */
void Sensors_ConvertFloat_2_int32(threshold_float_t f_threshold, threshold_int32_t* t_int32);

/**
 *  @brief  Convert string of integer into string with two decimal spaces (x / 100)
 *
 *  Input value which is 100 times greater than real (integer representation),
 *  should be transformed into string with its real value.
 *
 *  @param  Return string
 *  @param  Input value
 *
 *  @return void
 */
void Sensors_Convert_f_Str(char* txt, int x);

/**
 *  @brief  Extracts led state characteristic from JSON message
 *
 *  Searches for led state characteristic and reads its bool value.
 *
 *  @param  Return value of led state
 *
 *  @return 0  - successful,  -1 - failed
 */
int  Sensors_Extract_LedState(led_state_t* led);

/**
 *  @brief  Extracts frequency characteristic from JSON message
 *
 *  Searches for frequency characteristic and reads its int value.
 *
 *  @param  Return value of frequency
 *
 *  @return 0  - successful,  -1 - failed
 */
int  Sensors_Extract_Frequency(frequency_t* freq);

/**
 *  @brief  Extracts beacon frequency characteristic from JSON message
 *
 *  Searches for beacon frequency characteristic and reads its int value.
 *
 *  @param  Return value of beacon frequency
 *
 *  @return 0  - successful,  -1 - failed
 */
int  Sensors_Extract_BeaconFreq(beaconFrequency_t* beaconfreq);

/**
 *  @brief  Extracts text characteristic from JSON message
 *
 *  Searches desired token in json message, and returns its text value.
 *  Token should be of text type.
 *
 *  (not implemented)
 *
 *  @param  Pointer to string that we search
 *
 *  @return 0  - successful,  -1 - failed
 */
//int  Sensors_Extract_Pass(char* passkey);

/**
 *  @brief  Extract  three threshold float values
 *
 *  Searches for threshold token in json message from given token number.
 *  And stores three float threshold values in aoppropriate struct.
 *
 *  @param  given start token number
 *  @param  Return float threshold struct
 *
 *  @return 0  - successful,  -1 - failed
 */
int  Sensors_FloatReadThreshould(char cnt, threshold_float_t* f_threshold);

/**
 *  @brief  Extract three threshold int values
 *
 *  Searches for threshold token in json message from given token number.
 *  And stores three int threshold values in aoppropriate struct.
 *
 *  @param  given start token number
 *  @param  Return int threshold struct
 *
 *  @return 0  - successful,  -1 - failed
 */
int  Sensors_IntReadThreshold(char cnt, threshold_int16_t* i_threshold);

/**
 *  @brief  Form firmware or hardware revision string
 *
 *  @param  non zero terminated string
 *
 *  @return C string
 */
void Sensors_FormFrmHwRevStr(char* txt);

/**
 *  @brief  Search and stores message ID
 *
 *  Finds message id in json message and stores it for furhter use.
 *  It will be needed to generate response message.
 *
 *  @return Object number of detected string msg ID,  0 if not successful
 */
char Sensors_JSON_StoreMsgId();

/**
 *  @brief  Gets current message ID
 *
 *  Returns pointer to string with saved message id.
 *
 *  @return Oointer to string with msg ID
 */
char* Sensors_JSON_GetStoredMsgId();

/**
 *  @brief  Discard previously saved message id
 *
 *  @return void
 */
void Sensors_JSON_DiscardMsgId();

/**
 *  @brief  Gets int value for desired token string
 *
 *  Searches desired token in json message from given token number, and returns its integer value.
 *  Token should be of int type.
 *
 *  @param  Pointer to string that we search
 *  @param  Token number from which we search,
 *  @param  Return value for desired int data
 *
 *  @return 0  - successful,  -1 -  failed
 */
int  Sensors_JSON_ReadSingleIntValue(char* tokStr, int cnt, int* value);

/**
 *  @brief  Gets float value for desired token string
 *
 *  Searches desired token in json message from given token number, and returns its float value.
 *  Token should be of float type.
 *
 *  @param  Pointer to string that we search
 *  @param  Token number from which we search,
 *  @param  Return value for desired float data
 *
 *  @return 0  - successful,  -1 -  failed
 */
int  Sensors_JSON_ReadSingleFloatValue(char* tokStr, int cnt, float* value);

/**
 *  @brief  Get ble firmware revision string
 *
 *  @return pointer to ble firmware version string
 */
char* Sensors_GetBleFirmRevStr();


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// Sensors Process Data functions (down from cloud)

int Sensors_htu_ProcessData   (spi_frame_t* SPI_msg, char* msg);
int Sensors_ir_ProcessData    (spi_frame_t* SPI_msg, char* msg);
int Sensors_light_ProcessData (spi_frame_t* SPI_msg, char* msg);
int Sensors_gyro_ProcessData  (spi_frame_t* SPI_msg, char* msg);
int Sensors_sound_ProcessData (spi_frame_t* SPI_msg, char* msg);
int Sensors_bridge_ProcessData(spi_frame_t* SPI_msg, char* msg);
int MainBoard_ProcessData     (spi_frame_t* SPI_msg, char* msg);


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// Sensors Update functions (up on cloud)

void Sensors_htu_Update   (spi_frame_t* SPI_msg, char* buf);
void Sensors_ir_Update    (spi_frame_t* SPI_msg, char* buf);
void Sensors_light_Update (spi_frame_t* SPI_msg, char* buf);
void Sensors_gyro_Update  (spi_frame_t* SPI_msg, char* buf);
void Sensors_sound_Update (spi_frame_t* SPI_msg, char* buf);
void Sensors_bridge_Update(spi_frame_t* SPI_msg, char* buf);
void MainBoard_Update     (spi_frame_t* SPI_msg, char* buf);


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#endif // SENSORS_COMMON_H_
