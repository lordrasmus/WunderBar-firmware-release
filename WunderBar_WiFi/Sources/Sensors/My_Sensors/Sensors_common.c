/** @file   Sensors_common.c
 *  @brief  File contains common functions used for handling messages
 *  		from and to all sensor boards
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <stdio.h>
#include "Sensors_common.h"

float atof(char * s);

// static declarations

static char Sensors_MsgID[20];

static char Sensors_Convert_x_2_ascii(uint8_t x);


///////////////////////////////////////
/*         public functions          */
///////////////////////////////////////


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
void Sensors_ConvertFloat_2_int16(threshold_float_t f_threshold, threshold_int16_t* t_int16){
	t_int16->sbl   = (int16_t) (f_threshold.sbl  * 100);
	t_int16->low   = (int16_t) (f_threshold.low  * 100);
	t_int16->high  = (int16_t) (f_threshold.high * 100);
}

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
void Sensors_ConvertFloat_2_int32(threshold_float_t f_threshold, threshold_int32_t* t_int32){
	t_int32->sbl  = (int32_t) (f_threshold.sbl  * 100);
	t_int32->low  = (int32_t) (f_threshold.low  * 100);
	t_int32->high = (int32_t) (f_threshold.high * 100);
}

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
void Sensors_Convert_f_Str(char* txt, int x){
	char temp_txt[9], *ptr;
	char neg = 0;

	if (x < 0){
		neg = 1;
		x = -x;
	}

	temp_txt[8] = 0;
	ptr = &temp_txt[7];
	*ptr-- = Sensors_Convert_x_2_ascii((x /       1) % 10);
	*ptr-- = Sensors_Convert_x_2_ascii((x /      10) % 10);
	*ptr-- = '.';
	*ptr-- = Sensors_Convert_x_2_ascii((x /     100) % 10);
	*ptr-- = Sensors_Convert_x_2_ascii((x /    1000) % 10);
	*ptr-- = Sensors_Convert_x_2_ascii((x /   10000) % 10);
	*ptr-- = Sensors_Convert_x_2_ascii((x /  100000) % 10);
	*ptr   = Sensors_Convert_x_2_ascii((x / 1000000) % 10);

	while ((*ptr == '0') && (ptr < &temp_txt[4]))
		ptr ++;

	if (neg)
		* --ptr = '-';

	strcpy(txt, (const char *) ptr);
}

/**
 *  @brief  Search and stores message ID
 *
 *  Finds message id in json message and stores it for furhter use.
 *  It will be needed to generate response message.
 *
 *  @return Object number of detected string msg ID,  0 if not successful
 */
char Sensors_JSON_StoreMsgId(){
	char c;

	c = JSON_Msg_FindToken(JSON_MSG_ID, 0);
	if (c > 0)
		strcpy(Sensors_MsgID, JSON_Msg_GetTokStr(c));

	return c;
}

/**
 *  @brief  Gets current message ID
 *
 *  Returns pointer to string with saved message id.
 *
 *  @return Oointer to string with msg ID
 */
char* Sensors_JSON_GetStoredMsgId(){
	return Sensors_MsgID;
}

/**
 *  @brief  Discard previously saved message id
 *
 *  @return void
 */
void Sensors_JSON_DiscardMsgId(){
	memset(Sensors_MsgID, 0, sizeof(Sensors_MsgID));
}

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
int Sensors_JSON_ReadSingleIntValue(char* tokStr, int cnt, int* value){
	int c;

	if ((c = JSON_Msg_FindToken(tokStr, cnt)) > 0)
	{
		sscanf(JSON_Msg_GetTokStr(c), "%d", value);
		return 0;
	}
	else
		return -1;
}

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
int Sensors_JSON_ReadSingleFloatValue(char* tokStr, int cnt, float* value){
	int c;

	if ((c = JSON_Msg_FindToken(tokStr, cnt)) > 0)
	{
		*value = atof(JSON_Msg_GetTokStr(c));
		return 0;
	}
	else
		return -1;
}

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
//int Sensors_Extract_Pass(char* passkey){
//	char cnt;
//
//	cnt = Sensors_JSON_FindToken(JSON_MSG_PASSKEY, 0);
//	if (cnt > 0)
//	{
//		strcpy(passkey, Sensors_JSON_GetTokStr(cnt));
//		return 0;
//	}
//	else
//		return -1;
//}

/**
 *  @brief  Extracts beacon frequency characteristic from JSON message
 *
 *  Searches for beacon frequency characteristic and reads its int value.
 *
 *  @param  Return value of beacon frequency
 *
 *  @return 0  - successful,  -1 - failed
 */
int Sensors_Extract_BeaconFreq(beaconFrequency_t* beaconfreq){
	if (Sensors_JSON_ReadSingleIntValue(JSON_MSG_FREQ, 0, (int*) beaconfreq) == 0)
		return 0;
	else
		return -1;
}

/**
 *  @brief  Extracts frequency characteristic from JSON message
 *
 *  Searches for frequency characteristic and reads its int value.
 *
 *  @param  Return value of frequency
 *
 *  @return 0  - successful,  -1 - failed
 */
int Sensors_Extract_Frequency(frequency_t* freq){
	if (Sensors_JSON_ReadSingleIntValue(JSON_MSG_FREQ, 0, (int*) freq) == 0)
		return 0;
	else
		return -1;
}

/**
 *  @brief  Extracts led state characteristic from JSON message
 *
 *  Searches for led state characteristic and reads its bool value.
 *
 *  @param  Return value of led state
 *
 *  @return 0  - successful,  -1 - failed
 */
int Sensors_Extract_LedState(led_state_t* led){
	int temp_led;
	if (Sensors_JSON_ReadSingleIntValue(JSON_MSG_CMD, 0, (int*) &temp_led) == 0)
	{
		if (temp_led == 1)
			*led = true;
		else
			*led = false;
		return 0;
	}
	else
		return -1;
}

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
int Sensors_FloatReadThreshould(char cnt, threshold_float_t* f_threshold){
	// extract hysteresis value
	if (Sensors_JSON_ReadSingleFloatValue(JSON_MSG_HYSTERESIS, cnt, (float*) &f_threshold->sbl) != 0)
		return -1;
	// extract low value
	if (Sensors_JSON_ReadSingleFloatValue(JSON_MSG_LOW, cnt, (float*) &f_threshold->low) != 0)
		return -1;
	// extract high value
	if (Sensors_JSON_ReadSingleFloatValue(JSON_MSG_HIGH, cnt, (float*) &f_threshold->high) != 0)
		return -1;
	// return OK
	return 0;
}

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
int Sensors_IntReadThreshold(char cnt, threshold_int16_t* i_threshold){
	// extract hysteresis value
	if (Sensors_JSON_ReadSingleIntValue(JSON_MSG_HYSTERESIS, cnt, (int*) &i_threshold->sbl) != 0)
		return -1;
	// extract low value
	if (Sensors_JSON_ReadSingleIntValue(JSON_MSG_LOW, cnt, (int*) &i_threshold->low) != 0)
		return -1;
	// extract high value
	if (Sensors_JSON_ReadSingleIntValue(JSON_MSG_HIGH, cnt, (int*) &i_threshold->high) != 0)
		return -1;
	// return OK
	return 0;
}

/**
 *  @brief  Form firmware or hardware revision string
 *
 *  @param  non zero terminated string
 *
 *  @return C string
 */
void Sensors_FormFrmHwRevStr(char* txt){
	char* ptr;

	ptr = txt;
	while ((*ptr != 0xFF) && ((ptr - txt) < SPI_PACKET_DATA_SIZE))
		ptr ++;

	*ptr = 0;
}



///////////////////////////////////////
/*         static functions          */
///////////////////////////////////////


/**
 *  @brief  Returns ascii value of a number.
 *
 *  @return ascii
 */
static char Sensors_Convert_x_2_ascii(uint8_t x){
	return x + '0';
}
