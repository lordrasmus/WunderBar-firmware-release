/** @file   Sensors_SPI.c
 *  @brief  File contains functions for SPI communication with ble master module
 *  		Messages are formated into SPI frame format and sent for further processing
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

// message type
//
//  HEADER   3 bytes :
//
//  msg data id            ---  determine a data type
//                              sensor type or response type
//  msg field id    	   ---  determine characteristic type
//                              sensor characteristic or config characteristic
//  read/write             ---  determine action read or write
//
//  MESSAGE DATA  following bytes
//
//  msg data 	           ---  send appropriate message data (length depends on chosen data id and field id)
//

#include <string.h>

#include <hardware/Hw_modules.h>
#include "wunderbar_common.h"
#include "Sensors_main.h"


#define DUMMY_BYTE 		0xFF


// static declarations

static bool Sensors_SPI_Read(char* msg, char count);
static bool Sensors_SPI_Send(char* msg, int count);




	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////


/**
*  @brief  Send message to BT via SPI
*
*  Function receives SPI frame, which should be sent to master ble module.
*  First, message size will be calculated and entire message will be sent to ble module.
*
*  @param  SPI frame
*
*  @return True if desired number of bytes were sent.
*/
bool Sensors_SPI_SendMsg(spi_frame_t* SPI_msg){
	char count;
	bool result;

	// get size of SPI frame
	count = sensors_get_msg_size(SPI_msg->data_id, SPI_msg->field_id) + SPI_PACKET_HEADER_SIZE;

	// SPI Chip Select Activate
	SPI_CS_Activate();

	// send all data by SPI
	result = Sensors_SPI_Send((char*) SPI_msg, count);

	// SPI Chip Select Deactivate
	SPI_CS_Deactivate();

	return result;
}


/**
*  @brief  Read data from BT via SPI
*
*  Master ble module sill trigger ext interrupt when need something t send.
*  This function should be called to collect this data. Function will read
*  header bytes first and calculate how much bytes are left and read them too.
*  After entire message is read, it will form SPI frame and send it for further processing.
*
*  @return void
*/
void Sensors_SPI_ReadMsg(){
	spi_frame_t SPI_msg;
	char count;

	memset((void *) &SPI_msg, DUMMY_BYTE, sizeof(spi_frame_t));

	SPI_CS_Activate();					// SPI Chip Select Activate

	// read SPI frame header first
	if (Sensors_SPI_Read((char*) &SPI_msg, SPI_PACKET_HEADER_SIZE) == false)
	{
		goto exit;
	}

	count = sensors_get_msg_size(SPI_msg.data_id, SPI_msg.field_id);

	// now read data bytes
	if (Sensors_SPI_Read((char*) &SPI_msg.data[0], count) == false)
	{
		goto exit;
	}

	SPI_CS_Deactivate();				// SPI Chip Select Deactivate

	Sensors_Process_Data(&SPI_msg);		// Process received data by SPI

exit :
    SPI_CS_Deactivate();				// SPI Chip Select Deactivate
}




	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////



/**
*  @brief  SPI send desired number of bytes
*
*  Send desired number of bytes via SPI without toggling chip select line.
*  User should activate and deactivate CS manually before and after this call.
*
*  @param  Pointer to output buffer
*  @param  Number of bytes to send.
*
*  @return True if desired number of bytes were sent.
*/
static bool Sensors_SPI_Send(char* msg, int count){
	if (count != SPI_Write(msg, count))
		return false;
	else
		return true;
}

/**
*  @brief  SPI Read desired number of bytes
*
*  Read desired number of bytes over SPI without toggling chip select line.
*  User should activate and deactivate CS manually before and after this call.
*
*  @param  Pointer to input buffer.
*  @param  Number of bytes to read.
*
*  @return True if desired number of bytes were read.
*/
static bool Sensors_SPI_Read(char* msg, char count){

	if (!count)
		return true;

	if (SPI_Read(msg, count) != count)
		return false;

	return true;
}
