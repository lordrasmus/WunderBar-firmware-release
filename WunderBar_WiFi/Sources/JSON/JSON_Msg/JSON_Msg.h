/** @file   JSON_Msg.h
 *  @brief  File contains functions for processing incoming JSON messages.
 *			Parses JSON string and stores all token found.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#define MAX_TOKEN_NUMBER   50

// public functions

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
int JSON_Msg_Parse(char* msg);

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
int JSON_Msg_FindToken(char* tokStr, int cnt);

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
int JSON_Msg_ReadArray(char* ArraName, char* arr);

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
char* JSON_Msg_GetTokStr(int count);
