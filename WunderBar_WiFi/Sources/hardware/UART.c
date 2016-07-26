#include "Hw_modules.h"


#define UART_BLOCK_TIMEOUT 1000  // 1000 ms will be waiting for response


/**
 *  @brief  UART write function
 *
 *  Send desired number of bytes via UART module
 *
 *  @param  Pointer to output buffer
 *  @param  Number of bytes to send
 *
 *  @return void
 */
void GS_HAL_send(uint8_t* StrPtr, uint32_t Size){
	uint32_t written;
	AS1_SendBlock((AS1_TComData *) StrPtr, (word) Size, (word *) &written);
}

/**
 *  @brief  UART receive function
 *
 *  Receive bytes from UART and store them in input buffer. If blocking call is used, function will wait for the
 *  desired number of bytes to be received.
 *
 *  @param  Pointer to input buffer
 *  @param  Number of bytes to read
 *  @param  Block flag
 *
 *  @return Number of successfully read bytes.
 */
unsigned int GS_HAL_recv(uint8_t* recvbyte, uint32_t Size, uint32_t block){
	word read, result = 0;
	uint64_t time;

	if (block)
	{
		time = MSTimerGet();
		while (Size)
		{
			while ((AS1_RecvBlock((AS1_TComData*)recvbyte, (word)Size, (word*)&read) != ERR_OK) ||
																						(MSTimerDelta(time) > UART_BLOCK_TIMEOUT))
				;

			recvbyte += read;
			Size -= read;
			result += read;
		}
	}
	else
	{
		if (AS1_RecvBlock((AS1_TComData*)recvbyte, (word)Size, (word*)&read) == ERR_OK)
			result = read;
	}
	return result;
}

/**
 *  @brief  Clear UART input buffer
 *
 *  @return void
 */
void GS_HAL_ClearBuff(){
	uint8_t dummy;

	while (AS1_GetCharsInRxBuf())
		GS_HAL_recv(&dummy, 1, 0);
}
