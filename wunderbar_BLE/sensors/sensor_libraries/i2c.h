
/** @file   i2c.h
 *  @brief  This driver contains declarations of functions for working with TWI-I2C Nordic module
 *          and corresponding macros, constants,and global variables.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */
 
/* -- Includes -- */

#ifndef _NRF51822_I2C_
#define _NRF51822_I2C_

#include "types.h"

/**@brief TWI error codes. */
#define TWI_ERROR_WRITE_TIMEOUT -1
#define TWI_ERROR_WRITE_NACK    -2
#define TWI_ERROR_READ_TIMEOUT  -3
#define TWI_ERROR_READ_NACK     -4

typedef struct 
{
	/* TASKS */
		HW_WO TASKS_STARTRX;    // Start 2-Wire master receive sequence.
		HW_UU unused1[0x01];    // padding
		HW_WO TASKS_STARTTX;    // Start 2-Wire master transmit sequence.
		HW_UU unused2[0x02];    // padding
		HW_WO TASKS_STOP;       // Stop 2-Wire transaction.
		HW_UU unused3[0x01];    // padding
		HW_WO TASKS_SUSPEND;    // Suspend 2-Wire transaction.
		HW_WO TASKS_RESUME;     // Resume 2-Wire transaction.
		HW_UU unused4[0x38];    // padding
	/* EVENTS */
		HW_RW EVENTS_STOPPED;   // Two-wire stopped.
		HW_RW EVENTS_RXDREADY;  // Two-wire ready to deliver new RXD byte received.
		HW_UU unused5[0x04];    // padding
		HW_RW EVENTS_TXDSENT;   // Two-wire finished sending last TXD byte.
		HW_UU unused6[0x01];    // padding
		HW_RW EVENTS_ERROR;     // Two-wire error detected.
		HW_UU unused7[0x04];    // padding
		HW_RW EVENTS_BB;        // Two-wire byte boundary.
		HW_UU unused8[0x03];    // padding
		HW_RW EVENTS_SUSPENDED; // Two-wire suspended.
		HW_UU unused9[0x2D];    // padding
	/* REGISTERS */
		HW_RW SHORTS;           // Shortcuts for TWI.
		HW_UU unused10[0x40];   // padding
		HW_RW INTENSET;         // Interrupt enable set register.
		HW_RW INTENCLR;         // Interrupt enable clear register.
		HW_UU unused11[0x6E];   // padding
	/* DEVICE REGISTERS */
		HW_RW ERRORSRC;         // Two-wire error source. Write error field to 1 to clear error. (bit #1 set if nack after address, bit #2 set if nack after data, set bits to 1 to clear)
		HW_UU unused12[0x0E];   // padding
		HW_RW ENABLE;           // Enable two-wire master. (0 = disable TWI, 5 = enable TWI)
		HW_UU unused13[0x01];   // padding
		HW_RW PSELSCL;          // Pin select for SCL.
		HW_RW PSELSDA;          // Pin select for SDA.
		HW_UU unused14[0x02];   // padding
		HW_RW RXD;              // RX data from last transfer
		HW_RW TXD;              // TX data for next transfer
		HW_UU unused15[0x01];   // padding
		HW_RW FREQUENCY;        // Two-wire frequency.
		HW_UU unused16[0x18];   // padding
		HW_RW ADDRESS;          // Address used in the two-wire transfer.
		HW_UU unused17[0x29C];  // padding
		HW_RW POWER;            // Peripheral power control.
} 
TWI_STRUCT;


#define TWI0_HW ((TWI_STRUCT*)(0x40003000))
#define TWI1_HW ((TWI_STRUCT*)(0x40004000))

#define TWI_DISABLED      0
#define TWI_ENABLED       5

#define TWI_SHORT_BB_SUS  1
#define TWI_SHORT_BB_STP  2

typedef enum 
{
  K100 = 0x01980000,       /**< 100kbps */
  K250 = 0x04000000,       /**< 250kbps */
  K400 = 0x06680000        /**< 400kbps */
} 
TWI_FREQUENCY;


/** @brief  Initialize TWI peripheral for given setting.
 *
 *  @param  i2c  Pointer to an record that relates to the Two-Wire module.
 *  @param  scl  I2C interface SCL pin.
 *  @param  sda  I2C interface SDA pin.
 *  @param  freq Bus frequency
 *
 *  @return false in case that error is occurred, otherwise true.
 */
bool i2c_init (TWI_STRUCT* i2c, uint8_t scl, uint8_t sda, TWI_FREQUENCY freq);

/** @brief  Sends data byte via the I2C bus.
 *
 *  @param  i2c  Pointer to an record that relates to the Two-Wire module.
 *  @param  addr Slave address.
 *  @param  len  Number of bytes to be sent.
 *  @param  buf  Data to be sent.
 *  @param  stop Trigger STOP task flag.
 *
 *  @return Number of transferred bytes or negative error code.
 */
int i2c_write(TWI_STRUCT* i2c, uint8_t addr, uint16_t len, uint8_t* buf, bool stop);

/** @brief  Reads a byte from the I²C bus.
 *
 *  @param  i2c  Pointer to an record that relates to the Two-Wire module.
 *  @param  addr Slave address.
 *  @param  len  Number of bytes to be received.
 *  @param  buf  Pointer to the receive buffer.
 *
 *  @return Number of received bytes or negative error code.
 */
int i2c_read (TWI_STRUCT* i2c, uint8_t addr, uint16_t len, uint8_t* buf);

/** @brief  This function do write then read bytes from bus.
 *
 *  @param  i2c  Pointer to an record that relates to the Two-Wire module.
 *  @param  addr Slave address.
 *  @param  len_w  Number of bytes to be sent.
 *  @param  buf_w  Pointer to the sent data.
 *  @param  len_r  Number of bytes to be received.
 *  @param  buf_w  Pointer to the receive buffer. 
 *
 *  @return Summation of number of received and number of sent bytes, or negative error code.
 */
int i2c_write_read(TWI_STRUCT* i2c, uint8_t addr, uint16_t len_w, uint8_t* buf_w, uint16_t len_r, uint8_t* buf_r);

/** @brief  Enable I2C module.
 *
 *  @param  i2c  Pointer to an record that relates to the Two-Wire module.
 *
 *  @return Void.
 */
void i2c_enable(TWI_STRUCT* i2c);

/** @brief  Disable I2C module.
 *
 *  @param  i2c  Pointer to an record that relates to the Two-Wire module.
 *
 *  @return Void.
 */
void i2c_disable(TWI_STRUCT* i2c);

#endif /* _NRF51822_I2C_ */
