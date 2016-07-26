
/** @file   gpiote.c
 *  @brief  GPIOTE driver.
 *
 *  This contains the driver declarations for the GPIOTE module.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

#ifndef _NRF51822_GPIOTE_
#define _NRF51822_GPIOTE_

#include "types.h"

typedef struct {
/* TASKS */
  HW_WO TASKS_OUT[0x04]; // Task for writing to pin specified by PSEL in CONFIG[0..3].
  HW_UU unused1[0x3C];   // padding
/* EVENTS */
  HW_RW EVENTS_IN[0x04]; // Event generated from pin specified by PSEL in CONFIG[0..3].
  HW_UU unused2[0x1B];   // padding
  HW_RW EVENTS_PORT;     // Event generated from multiple input pins.
  HW_UU unused3[0x61];   // padding
/* REGISTERS */
  HW_RW INTENSET;        // Interrupt enable set register.
  HW_RW INTENCLR;        // Interrupt enable clear register.
  HW_UU unused4[0x81];   // padding
/* DEVICE REGISTERS */
  HW_RW CONFIG[0x04];    // Configuration for OUT[0..3] task and IN[0..3] event.
  HW_UU unused5[0x2B7];  // padding
  HW_RW POWER;           // Peripheral power control.
} GPIOTE_STRUCT;


#define GPIOTE_HW ((GPIOTE_STRUCT*)(0x40006000))

typedef enum {
  GPIOTE_MODE_DISABLED = 0, // Disabled. Pin specified by PSEL will not be acquired by the GPIOTE module.
  GPIOTE_MODE_EVENT    = 1, // Event mode. The pin specified by PSEL will be configured as an input and the 
                            // IN[n] event will be generated if operation specified in POLARITY occurs on the pin.
  GPIOTE_MODE_TASK     = 3  // Task mode. The pin specified by PSEL will be configured as an output and 
                            // triggering the OUT[n] task will perform the operation specified by POLARITY on the 
                            // pin. When enabled as a task the GPIOTE module will acquire the pin and the
                            // pin can no longer be written as a regular output pin from the GPIO module.
} GPIOTE_MODE;

typedef enum {
  GPIOTE_POLARITY_LOTOHI = 1, // Task mode: Set pin from OUT[n] task.
                              // Event mode: Generate IN[n] event when rising edge on pin.
  GPIOTE_POLARITY_HITOLO = 2, // Task mode: Clear pin from OUT[n] task.
                              // Event mode: Generate IN[n] event when falling edge on pin.
  GPIOTE_POLARITY_TOGGLE = 3  // Task mode: Toggle pin from OUT[n].
                              // Event mode: Generate IN[n] when any change on pin.
} GPIOTE_POLARITY;

typedef enum {
  GPIOTE_OUTINIT_LOW  = 0, // Task mode: Initial value of pin before task triggering is low.
  GPIOTE_OUTINIT_HIGH = 1  // Task mode: Initial value of pin before task triggering is high.
} GPIOTE_OUTINIT;

typedef enum {
  GPIOTE_INT_0_IDX = 0,
  GPIOTE_INT_1_IDX = 1,
  GPIOTE_INT_2_IDX = 2,
  GPIOTE_INT_3_IDX = 3,
  GPIOTE_INT_PORT_IDX = 31 
} GPIOTE_INT_IDX;

/** @brief Configure GPIOTE module
 *
 *  @param idx       Interrupt idx to activate.
 *  @param mode      GPIOTE mode.
 *  @param polarity  GPIOTE polarity.
 *  @param init      Initial value of the output when the GPIOTE channel is configured.
 *
 *  @return false in case error occurred, otherwise true.
 */
bool gpiote_configure(uint8_t idx, uint8_t pin, GPIOTE_MODE mode, GPIOTE_POLARITY polarity, GPIOTE_OUTINIT init);

/** @brief Enable an interrupt.
 *
 *  @param idx interrupt idx to activate.
 *
 *  @return Void.
 */
void gpiote_enable_interrupt(GPIOTE_INT_IDX idx);

/** @brief Disable an interrupt.
 *
 *  @param idx interrupt idx to deactivate.
 *
 *  @return Void.
 */
void gpiote_disable_interrupt(GPIOTE_INT_IDX idx);

#endif /* _NRF51822_GPIOTE_ */
