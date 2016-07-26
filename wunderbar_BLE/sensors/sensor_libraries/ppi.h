
/** @file   ppi.h
 *  @brief  Programmable Peripheral Interconnect driver.
 *
 *  This contains the driver declarations for the Programmable Peripheral Interconnect module.
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

/* -- Includes -- */

#ifndef _NRF51822_PPI_
#define _NRF51822_PPI_

#include "types.h"

// Chapter 15

typedef struct {
  HW_RW EEP; // event endpoint
  HW_RW TEP; // task endpoint
} EEP_TEP;

typedef struct {
  HW_WO EN; // event endpoint
  HW_WO DIS; // task endpoint
} PPI_CH_GRP;

typedef struct {
  PPI_CH_GRP CHG_E_D[0x04];             // channel group enable/disable
  HW_UU      unused1[0x138];            // padding
  HW_RW      CHEN;                      // Channel enable
  HW_RW      CHENSET;                   // enable set
  HW_RW      CHENCLR;                   // enable clear
  EEP_TEP    CH[16];                    // Channel 0-15 event & task endpoints
  HW_RW      CHG[4];                    // Channel group 0-3
} PPI_STRUCT;

// Table 3
#define PPI_HW ((PPI_STRUCT*)(0x4001F000))

/** @brief Function connects event and task for corresponding chanel.
 *
 *  @param channel  PPI channel.
 *  @param event    Pointer to event register.
 *  @param task     Pointer to task register.
 *
 *  @return Void.
 */
void ppi_connect(uint8_t channel, volatile void* event, volatile void* task);

/** @brief Function enables PPI chanel.
 *
 *  @param channel  PPI channel to be enabled.
 *
 *  @return Void.
 */
void ppi_channel_set(uint8_t channel);

/** @brief Function enables PPI chanel.
 *
 *  @param channel  PPI channel to be enabled.
 *
 *  @return Void.
 */
void ppi_channel_clear(uint8_t channel);

#endif /* _NRF51822_PPI_ */
