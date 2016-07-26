
/** @file   mpu6500.h
 *  @brief  This driver contains declarations of functions for working with MPU6500 sensor 
 *          and corresponding macros, constants,and global variables.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */
 
#ifndef MPU6500
#define MPU6500

#include "i2c.h"
#include "wunderbar_common.h"

#define MPU6500_I2C_ADDR    0x68                           /**< I2C address of sensor. */
#define MPU6500_I2C_SCL_PIN   23                           /**< I2C interface SCL pin. */
#define MPU6500_I2C_SDA_PIN   24                           /**< I2C interface SDA pin. */ 
#define MPU6500_I2C_INT_PIN   27                           /**< Interrupt pin. */ 

#define MPU6500_STARTUP_TIME  100000                       /**< Starutp time of MPU6500 sensor. */ 
#define MPU6500_WAKEUP_TIME   35000                        /**< Wakeup time of MPU6500 sensor. */ 

/**@brief MPU6500 Registers. */

/**@brief Value in these registers store results of manufacturing test to compare to own tests. */
#define MPU6500_SELF_TEST_X_GYRO  0x01
#define MPU6500_SELF_TEST_Y_GYRO  0x02
#define MPU6500_SELF_TEST_Z_GYRO  0x03
#define MPU6500_SELF_TEST_X_ACCEL 0x0D
#define MPU6500_SELF_TEST_Y_ACCEL 0x0E
#define MPU6500_SELF_TEST_Z_ACCEL 0x0F

/**@brief These registers are used to remove DC bias from the output. 2's complement. Values in these registers
          are added to the sensor output before the value is placed in the respective register.
*/
#define MPU6500_XG_OFFSET_H 0x13
#define MPU6500_XG_OFFSET_L 0x14
#define MPU6500_YG_OFFSET_H 0x15
#define MPU6500_YG_OFFSET_L 0x16
#define MPU6500_ZG_OFFSET_H 0x17
#define MPU6500_ZG_OFFSET_L 0x18

/**@brief Sample Rate divider. */
#define MPU6500_SMPLRT_DIV     0x19

/**@brief 
					BIT
					7   | reserved
					6   | FIFO_MODE    | 0 = overwrite oldest when full 1= don't write when full
					5:3 | EXT_SYNC_SET | Enables the FSYNC pin data to be sampled. 0 = disabled
					2:0 | DLPF_CONFIG
*/
#define MPU6500_CONFIG         0x1A

/**@brief 
					BIT
					7   | X Gyro Selftest
					6   | Y Gyro Selftest
					5   | Y Gyro Selftest
					4:3 | Gyro Full Scale Select; 00: +/-250; 01: 500; 10: 1000; 11: 2000 dps 
					2   | reserved
					1:0 | bypass DLPF
*/
#define MPU6500_GYRO_CONFIG    0x1B

/**@brief 
          BIT
					7   | X Accelerometer Selftest
					6   | Y Accelerometer Selftest
					5   | Y Accelerometer Selftest
					4:3 | Accel Full Scale; 00:+/-2; 01:4; 10:8; 11:16g
					2:0 | reserved
*/
#define MPU6500_ACCEL_CONFIG   0x1C

/**@brief 
					BIT
					7:4 | reserved
					3   | bypass DLPF
					2:0 | Accelerometer low pass filter setting (s. Table p.15,16)
*/
#define MPU6500_ACCEL_CONFIG_2 0x1D

/**@brief 
					BIT
					Low Power Accelerometer Output Data Rate (ODR) Control
					7:4 | reserved
					3:0 | freq of wakeup/sample (s. table p.17)
*/
#define MPU6500_LP_ACCEL_ODR   0x1E

/**@brief Wake-on Motion Threshold threshold value for the Wake on 
          Motion Interrupt for accel x/y/z axes. LSB = 4mg. Range is 0mg to 1020mg.
*/
#define MPU6500_WOM_THR        0x1F

/**@brief 
					FIFO enable
					7 | Temp
					6 | Gyro X
					5 | Gyro Y
					4 | Gyro Z
					3 | Accelerometer
					2 | Slave 2
					1 | Slave 1
					0 | Slave 0
*/
#define MPU6500_FIFO_EN        0x23

/**@brief Registers 0x26 - 0x36 are for configuring I2C passthrough we don't do that. */

/**@brief 
					INT Pin / Bypass Enable Configuration
					7 | ACTL              | 1: INT pin active low
					6 | OPEN              | 1: INT pin open drain, 0: push-pull
					5 | LATCH_INT_EN      | 1: INT held until cleared 0: pulse width 50us
					4 | INT_ANYRD_2CLEAR  | 1: INT cleared on any read 0: on reading INT_STATUS register
					3 | ACTL_FSYNC        | 1: fsunc as interrupt 0: disable
					2 | FSYNC_INT_MODE_EN | i2c bypass mode
					1 | BYPASS_EN
					0 | reserved
*/
#define MPU6500_INT_PIN_CFG    0x37

/**@brief 
					Interrupt Enable
					7 | reserved
					6 | WOM          | 1: wake on motion
					5 | reserved 
					4 | FIFO_OF      | 1: fifo overflow
					3 | FSYNC        | 1: fsync propagation
					2 | reserved
					1 | reserved
					0 | RAW_READY_EN |
*/
#define MPU6500_INT_ENABLE     0x38

/**@brief  Interrupt Status same bits as interrupt enable, above. */
#define MPU6500_INT_STATUS     0x3A

/**@brief Accelerometer Hi and Lo. */
#define MPU6500_ACCEL_XOUT_H   0x3B
#define MPU6500_ACCEL_XOUT_L   0x3C
#define MPU6500_ACCEL_YOUT_H   0x3D
#define MPU6500_ACCEL_YOUT_L   0x3E
#define MPU6500_ACCEL_ZOUT_H   0x3F
#define MPU6500_ACCEL_ZOUT_L   0x40

/**@brief Temperature Hi and Lo.
          TEMP C = (TEMP_OUT - RoomTemp_Offset)/Temp_Sensitivity
*/
#define MPU6500_TEMP_OUT_H     0x41
#define MPU6500_TEMP_OUT_L     0x42

/**@brief Gyro output:
          GYRO_OUT = Gyro_Sensitivity * X/Y/Z_angular_rate
*/
#define MPU6500_GYRO_XOUT_H    0x43
#define MPU6500_GYRO_XOUT_L    0x44
#define MPU6500_GYRO_YOUT_H    0x45
#define MPU6500_GYRO_YOUT_L    0x46
#define MPU6500_GYRO_ZOUT_H    0x47
#define MPU6500_GYRO_ZOUT_L    0x48

/**@brief Registers 0x49 .. 0x60 external sensor data 0x63 .. 0x67 I2C slave data */

/**@brief 
				Signal Path Reset
				7:3 | reserved
				2   | Gyro reset
				1   | Acc reset
				0   | Temp reset
*/
#define MPU6500_SIGNAL_PATH_RESET 0x68

/**@brief 
				Accelerometer Interrupt Control
				7   | enable Wake on Motion
				6   | 1 = Compare the current sample with the previous sample. 0 = not used (?)
				5:0 | reserved
*/
#define MPU6500_ACCEL_INTEL_CTRL  0x69

/**@brief 
				User Control
				7 | DMP_EN     | Digital Motion Processor (turn on ?)
				6 | FIFO_EN
				5 | I2C_MST_EN
				4 | I2C_IF_DIS
				3 | DMP_RST
				2 | FIFO_RST
				1 | I2C_MST_RST
				0 | SIG_COND_RST
*/
#define MPU6500_USER_CTRL         0x6A

/**@brief 
				7   | DEVICE_RESET | reset all and restore default
				6   | SLEEP        | set to sleep mode
				5   | CYCLE        | cycle b/t sleep and measure at rate defined by LP_ACCEL_ODR
				4   | GYRO_STANDBY | allow quick gyro enable
				3   | TEMP_DIS     | 1: disable temp sensor
				2:0 | CLKSEL       | 0,6: internal 20Mhz ; 1-5: auto select ; stop clock  
*/
#define MPU6500_PWR_MGMT_1        0x6B

/**@brief 
				7:6 | LP_WAKE_CTRL | 0 : 1.25Hz ; 1: 5 Hz ; 2: 20Hz ; 3: 40Hz 
				5   | DISABLE_XA
				4   | DISABLE_YA
				3   | DISABLE_ZA
				2   | DISABLE_XG
				1   | DISABLE_YG
				0   | DISABLE_ZG
*/
#define MPU6500_PWR_MGMT_2        0x6C

/**@brief 
				7:5 | reserved
				4:0 | num bytes in fifo
*/
#define MPU6500_FIFO_COUNT_H      0x72
#define MPU6500_FIFO_COUNT_L      0x73

/**@brief Used to read and write to FIFO. */
#define MPU6500_FIFO_R_W          0x74

/**@brief ID Register, contains 0x70. */
#define MPU6500_WHO_AM_I_REG      0x75

/**@brief 
				Accelerometer Offset Registers
				H: 7:0 upper bits of offset cancellation
				L: 7:1 lower bits of offset cancellation
				+/- 16g Offset cancellation in all Full Scale modes, 15 bit 0.98-mg steps
*/
#define MPU6500_XA_OFFSET_H       0x77
#define MPU6500_XA_OFFSET_L       0x78
#define MPU6500_YA_OFFSET_H       0x7A
#define MPU6500_YA_OFFSET_L       0x7B
#define MPU6500_ZA_OFFSET_H       0x7D
#define MPU6500_ZA_OFFSET_L       0x7E

/**@brief This record holds properties of I2C interface used for communication with sensor. */
typedef struct 
{
    TWI_STRUCT * i2c;
} 
mpup6500_struct_t;

/**@brief Fload format Cartesian coordinate*/
typedef struct
{
    float x; 
    float y;
    float z;
}
coord_float_t;

/** @brief  This function is called to initialize i2c interface to MPU6500 sensor, and set default configuration.
 *
 *  @param  mpu  Pointer to an record that relates to the MPU6500 sensor.
 *  @param  i2c  Pointer to an record that relates to the Two-Wire module.
 *
 *  @return  false in case that error is occurred, otherwise true.
 */
bool mpu6500_init(mpup6500_struct_t * mpu, TWI_STRUCT * i2c);

/** @brief  This function configures full scale modes for accelerometer and gyroscope.
 *
 *  @param  mpu    Pointer to an record that relates to the MPU6500 sensor.
 *  @param  config Pointer to an sensor config record.
 *
 *  @return false in case that error is occurred, otherwise true.
 */
bool mpu6500_config(mpup6500_struct_t * mpu, sensor_gyro_config_t * config);

/** @brief  This function reads WHO_AM_I register. This register is used to verify the identity of the device. The contents of WHO_AM_I is an 8-bit device ID.
 *
 *  @param  mpu  Pointer to an record that relates to the MPU6500 sensor.
 *  @param  i2c  Pointer to an record that relates to the Two-Wire module.
 *
 *  @return  false in case that error is occurred, otherwise true.
 */
bool mpu6500_who_am_i(mpup6500_struct_t * mpu, uint8_t * data);

/** @brief  This function retrieves the contents of MPU6500 sensor internal register.
 *
 *  @param  mpu  Pointer to an record that relates to the MPU6500 sensor.
 *  @param  i2c  Pointer to an record that relates to the Two-Wire module.
 *  @param  reg  MPU6500 sensor internal register address.
 *  @param  data Provided storage used to hold the register value. 
 *
 *  @return false in case that error is occurred, otherwise true.
 */
bool mpu6500_read_register(mpup6500_struct_t * mpu, uint8_t reg, uint8_t * data);

/** @brief  This function retrieves bytes of measured coordinates.
 *
 *  @param  mpu  Pointer to an record that relates to the MPU6500 sensor.
 *  @param  reg  MPU6500 sensor internal "measured coordinates" register address (MPU6500_ACCEL_XOUT_H/MPU6500_GYRO_XOUT_H).
 *  @param  data Provided storage used to hold the measured data. 
 *
 *  @return false in case that error is occurred, otherwise true.
 */
bool mpu6500_get_data(mpup6500_struct_t * mpu, uint8_t reg, uint8_t * data);

/** @brief  This function sends sensor to sleep mode.
 *
 *  @param  mpu  Pointer to an record that relates to the MPU6500 sensor.
 *
 *  @return false in case that error is occurred, otherwise true.
 */
bool mpu6500_sleep(mpup6500_struct_t * mpu);

/** @brief  This function wakes up sensor.
 *
 *  @param  mpu Pointer to an record that relates to the MPU6500 sensor.
 *
 *  @return false in case that error is occurred, otherwise true.
 */
bool mpu6500_wakeup(mpup6500_struct_t * mpu);

/** @brief  This function retrieves accelerometer measured coordinates.
 *
 *  @param  mpu  Pointer to an record that relates to the MPU6500 sensor.
 *  @param  data Provided storage used to hold the measured data. 
 *
 *  @return false in case that error is occurred, otherwise true.
 */
bool mpu6500_get_acc(mpup6500_struct_t * mpu, coord_float_t * data);

/** @brief  This function retrieves gyroscope measured coordinates.
 *
 *  @param  mpu  Pointer to an record that relates to the MPU6500 sensor.
 *  @param  data Provided storage used to hold the measured data. 
 *
 *  @return false in case that error is occurred, otherwise true.
 */
bool mpu6500_get_gyro(mpup6500_struct_t * mpu, coord_float_t * data);

#endif /* MPU6500 */
