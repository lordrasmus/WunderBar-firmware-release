/** @file   JSON_Msg.c
 *  @brief  File contains functions for processing incoming JSON messages.
 *			Parses JSON string and stores all token found.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../Jsmn/Jsmn.h"
#include "JSON_Msg.h"


// static declarations

typedef char JSONString_t[40];
static JSONString_t JSON_str_array[MAX_TOKEN_NUMBER];

static jsmntok_t tok[MAX_TOKEN_NUMBER];
static int TotalTokensFound;

static int JSON_Msg_Parser(char* msg, int count, JSONString_t json_string[]);
static int JSON_Msg_FindArray(char* TokArrayStr, int cnt, int* arrCnt);



	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////


/**
 *  @brief  JSON message parse.
 *
 *  Parses JSON message with default max token number
 *  Stores results into static string array
 *
 *  @param  pointer JSON message which should be parsed.
 *
 *  @return number of found json objects
 */
int JSON_Msg_Parse(char* msg){
	return JSON_Msg_Parser(msg, MAX_TOKEN_NUMBER, JSON_str_array);
}

/**
 *  @brief  Search for desired token string from given object count
 *
 *  Searches for desired token within previously parsed JSON string.
 *  JSON_Msg_Parse should be called prior.
 *
 *  @param  Pointer to string that we search
 *  @param  Token number from which we search
 *
 *  @return Token number of detected object incremented by one,  0xFF not successful
 */
int JSON_Msg_FindToken(char* tokStr, int cnt){
	int i;

	for (i = cnt; i < TotalTokensFound; i ++)
		if (strcmp((const char *) &JSON_str_array[i][0], (const char *) tokStr) == 0)
			break;

	if (i >= TotalTokensFound)
		return -1;
	else
		return (i + 1);
}

/**
 *  @brief  Gets pointer to token string for desired token number
 *
 *  Returns pointer to object string of desired token number.
 *  Can be used with JSON_Msg_FindToken function
 *
 *  @param  Desired token number
 *
 *  @return Pointer to token string or null pointer if invalid
 */
char* JSON_Msg_GetTokStr(int count){
	if ((count > 0) && (count <= MAX_TOKEN_NUMBER))
		return JSON_str_array[count];
	else
		return NULL;
}

/**
 *  @brief  Read JSON array with desired name
 *
 *  Read json array and place array members in return char array
 *  Output will be formatted as array f uint8_t elements.
 *
 *  @param  Pointer to search array name
 *  @param  Return array with char members
 *
 *  @return Number of members found in array
 */
int JSON_Msg_ReadArray(char* ArrName, char* arr){
	int arrCnt = 0, r, i, value;
	// find desired array
	if ((r = JSON_Msg_FindArray(ArrName, 0, &arrCnt)) > 0)
	{
		for (i = 0; i < arrCnt; i ++)
		{
			sscanf(JSON_Msg_GetTokStr(r++), "%d", &value);
			*arr ++ = (char) value;
		}
	}
	return arrCnt;
}





	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////


 /**
 *  @brief  JSON message parser.
 *
 *  Function will parse JSON message and extract all JSON strings and primitives for further process.
 *  All Json tokens will be stored in static array of strings and will be available for later use.
 *  Function uses jasmin json parser.
 *
 *  @param  Pointer to input JSON message string.
 *  @param  Max number of tokens that can be stored
 *  @param  Array of strings where tokens will be stored
 *
 *  @return Number of total found tokens.
 */
static int JSON_Msg_Parser(char* msg, int count, JSONString_t json_string[]){
	jsmn_parser p;
	uint8_t i;

	jsmn_init(&p);

	if ((TotalTokensFound = jsmn_parse(&p, msg, strlen(msg), tok, count)) > count)
		return -1;  // error we can not store all tokens

	if (TotalTokensFound > 0)
	{
		for (i = 0; i < TotalTokensFound; i ++)
		{
			if ((tok[i].type == JSMN_STRING) || (tok[i].type == JSMN_PRIMITIVE))
			{
				memcpy((void *) json_string[i], (const void *) &msg[tok[i].start], tok[i].end - tok[i].start);
				json_string[i][tok[i].end - tok[i].start] = 0;
			}
		}
	}

	return TotalTokensFound;
}

/**
 *  @brief  Find Token Array by string.
 *
 *  Finds desired json array from given token number and returns number of array members.
 *
 *  @param  Desired json array name
 *  @param  Token number from which we will search
 *  @param  Return value with number of array members.
 *
 *  @return Token number of first array member or -1 if error.
 */
static int JSON_Msg_FindArray(char* TokArrayStr, int cnt, int* arrCnt){
	int r;

	r = JSON_Msg_FindToken(TokArrayStr, cnt);
	if (r > 0)
	{
		if (tok[r].type == JSMN_ARRAY)
			*arrCnt = tok[r].size;
		else
			*arrCnt = 0;
	}
	return (r + 1);
}
