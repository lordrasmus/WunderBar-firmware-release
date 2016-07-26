#include "Hw_modules.h"


LDD_TDeviceData *SMasterLdd1_DeviceDataPtr;

typedef struct {
  uint32_t TxCommand;                  /* Current Tx command */
  LDD_SPIMASTER_TError ErrFlag;        /* Error flags */
  uint16_t InpRecvDataNum;             /* The counter of received characters */
  uint8_t *InpDataPtr;                 /* The buffer pointer for received characters */
  uint16_t InpDataNumReq;              /* The counter of characters to receive by ReceiveBlock() */
  uint16_t OutSentDataNum;             /* The counter of sent characters */
  uint8_t *OutDataPtr;                 /* The buffer pointer for data to be transmitted */
  uint16_t OutDataNumReq;              /* The counter of characters to be send by SendBlock() */
  LDD_TUserData *UserData;             /* User device data structure */
} SMasterLdd1_TDeviceData;             /* Device data structure type */

typedef SMasterLdd1_TDeviceData* SMasterLdd1_TDeviceDataPtr ; /* Pointer to the device data structure. */

/**
 *  @brief  SPI init function
 *
 *  SPI0 module,
 *  LSB first,
 *  clock idle low,
 *  change on leading edge,
 *  2MHz clock,
 *
 *  @return void
 */
void SPI_Init() {
	SMasterLdd1_DeviceDataPtr = SM1_Init(NULL);
}

/**
 *  @brief  Nops to create small delay for chip select line
 *
 *  @return void
 */
static void SPI_Delay(){
	int cnt = 1000;

	while (cnt --)
		;
}

/**
 *  @brief  Activate SPI slave module by driving CS line low
 *
 *  @return void
 */
void SPI_CS_Activate(){
	GPIO_SpiClrCS();
	SPI_Delay();
}

/**
 *  @brief  Deactivate SPI slave module by driving CS line high
 *
 *  @return void
 */
void SPI_CS_Deactivate(){
	SPI_Delay();
	GPIO_SpiSetCS();
}

/**
*  @brief  Spi write
*
*  SPI write function. Without toggling CS.
*
*  @param  Pointer to output buffer.
*  @param  Number of bytes to send
*
*  @return Number of successfully sent bytes.
*/
unsigned int SPI_Write(char* sendbyte, char size){

	SMasterLdd1_TDeviceDataPtr DeviceDataPrv = SMasterLdd1_DeviceDataPtr;

	SM1_SendBlock(DeviceDataPrv, sendbyte, size);
	while (DeviceDataPrv->OutSentDataNum != size)
		;

	return size;
}

/**
*  @brief  Spi read
*
*  SPI read function. Without toggling CS.
*
*  @param  Pointer to input buffer.
*  @param  Number of bytes to read
*
*  @return Number of bytes that were read successfully
*/
unsigned int SPI_Read( char* recvbyte, char size){

	SMasterLdd1_TDeviceDataPtr DeviceDataPrv = SMasterLdd1_DeviceDataPtr;

	if (!size)
		return 0;

	SM1_ReceiveBlock(DeviceDataPrv, recvbyte, size);
	SM1_SendBlock(DeviceDataPrv, recvbyte, size);

	while (DeviceDataPrv->InpRecvDataNum != size)
		;

	return size;
}
