/**HEADER********************************************************************
* 
* Copyright (c) 2010 Freescale Semiconductor;
* All Rights Reserved
*
*************************************************************************** 
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: main.c$
* $Version : 
* $Date    : 
*
* Comments:
*
*   This file contains device driver for mass storage class. This code tests
*   the FAT functionalities.
*
*END************************************************************************/

/**************************************************************************
Include the OS and BSP dependent files that define IO functions and
basic types. You may like to change these files for your board and RTOS 
**************************************************************************/
/**************************************************************************
Include the USB stack header files.
**************************************************************************/
#include "derivative.h"
#include "hidef.h"
#include <string.h>
#include <stdlib.h>    /* ANSI memory controls */
#include "stdio.h"
#include "sci.h"
#include "Bootloader.h"
#include "Boot_loader_task.h"
#if (defined _MCF51MM256_H) || (defined _MCF51JE256_H)
#include "exceptions.h"
#endif
#if (defined __MCF52259_H__)
#include "flash_cfv2.h"
#elif (defined _MCF51JM128_H)
#include "flash.h"
#elif (defined MCU_MK60N512VMD100)
#include "flash_FTFL.h"
#elif (defined MCU_MK64F12) || (defined MCU_MK24F12)
#include "flash_FTFE.h"
#else
#include "flash_FTFL.h"
#endif
/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/
#if (defined _MCF51JM128_H)
	const byte NVPROT_INIT @0x0000040D = PROT_VALUE;
	const byte NVOPT_INIT  @0x0000040F  = 0x00;    // flash unsecure
#elif (defined __MCF52259_H__)
	// Initialized in cfm.c
#elif (defined MCU_MK60N512VMD100)
	  // Protect bootloader flash 0x0 - 0xBFFF
	  #pragma define_section cfmconfig ".cfmconfig" ".cfmconfig" ".cfmconfig" far_abs R
	  static __declspec(cfmconfig) uint_8 _cfm[0x10] = {
	   /* NV_BACKKEY3: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY2: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY1: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY0: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY7: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY6: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY5: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY4: KEY=0xFF */
	    0xFFU,
	   /* NV_FPROT3: PROT=0xF8 */
	    PROT_VALUE3,
	   /* NV_FPROT2: PROT=0xFF */
	    PROT_VALUE2,
	   /* NV_FPROT1: PROT=0xFF */
	    PROT_VALUE1,
	   /* NV_FPROT0: PROT=0xFF */
	    PROT_VALUE0,
	   /* NV_FSEC: KEYEN=1,MEEN=3,FSLACC=3,SEC=2 */
	    0x7EU,
	   /* NV_FOPT: ??=1,??=1,??=1,??=1,??=1,??=1,EZPORT_DIS=1,LPBOOT=1 */
	    0xFFU,
	   /* NV_FEPROT: EPROT=0xFF */
	    0xFFU,
	   /* NV_FDPROT: DPROT=0xFF */
	    0xFFU
	  };
#elif (defined MCU_MK64F12)
#elif (defined MCU_MK24F12)
	  // Protect bootloader flash 0x0 - 0xBFFF
	  #pragma define_section cfmconfig ".cfmconfig" ".cfmconfig" ".cfmconfig" far_abs R
	  static __declspec(cfmconfig) uint_8 _cfm[0x10] = {
	   /* NV_BACKKEY3: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY2: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY1: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY0: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY7: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY6: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY5: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY4: KEY=0xFF */
	    0xFFU,
	   /* NV_FPROT3: PROT=0xFC */
	    PROT_VALUE3,
	   /* NV_FPROT2: PROT=0xFF */
	    PROT_VALUE2,
	   /* NV_FPROT1: PROT=0xFF */
	    PROT_VALUE1,
	   /* NV_FPROT0: PROT=0xFF */
	    PROT_VALUE0,
	   /* NV_FSEC: KEYEN=1,MEEN=3,FSLACC=3,SEC=3 */
	    0x7EU,
	   /* NV_FOPT: ??=1,??=1,??=1,??=1,??=1,??=1,EZPORT_DIS=1,LPBOOT=1 */
	    0xFFU,
	   /* NV_FEPROT: EPROT=0xFF */
	    0xFFU,
	   /* NV_FDPROT: DPROT=0xFF */
	    0xFFU
	  };	  
#else
	#error Undefined MCU for flash protection
#endif
	
/**************************************************************************
   Global variables
**************************************************************************/
extern uint_8                       filetype;      /* image file type */     
static uint_32                      New_sp,New_pc; /* stack pointer and program counter */
/**************************************************************************
   Funciton prototypes
**************************************************************************/
#if MAX_TIMER_OBJECTS
extern uint_8 TimerQInitialize(uint_8 ControllerId);
#endif

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : GPIO_Init
* Returned Value : none
* Comments       : Init LEDs and Buttons
*
*END*--------------------------------------------------------------------*/
void GPIO_Bootloader_Init()
{
    /* Body */
    SIM_SCGC5   |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;    /* Enable clock gating to PORTA, PORTD and PORTE */

    /* set input PORTD pin 8 */
    PORTD_PCR8 =  PORT_PCR_MUX(1);
    GPIOD_PDDR &= ~((uint_32)1 << 8);
    PORTD_PCR8 &= ~(PORT_PCR_PE_MASK|PORT_PCR_PS_MASK); /* on board pull up used */

    /* Enable LED Port A pin 29 */                                    
    PORTA_PCR29 |= PORT_PCR_SRE_MASK        /* Slow slew rate */
                // |  PORT_PCR_ODE_MASK        /* Open Drain Enable */
                |  PORT_PCR_DSE_MASK        /* High drive strength */
            ;
    PORTA_PCR29 = PORT_PCR_MUX(1);
    
    GPIOA_PCOR  |= 1 << 29; /* pin=0, led off */
    GPIOA_PDDR  |= 1 << 29; /* pin output */
    
    /* Gainspan reset Port D pin 5 */                                    
    PORTD_PCR5 |= PORT_PCR_SRE_MASK        /* Slow slew rate */
                // |  PORT_PCR_ODE_MASK        /* Open Drain Enable */
                |  PORT_PCR_DSE_MASK        /* High drive strength */
            ;
    PORTD_PCR5 = PORT_PCR_MUX(1);
    
    GPIOD_PCOR  |= 1 << 5; /* pin=0, gainspan off */
    GPIOD_PDDR  |= 1 << 5; /* pin output */    

    /* Nordic reset Port E pin 24 */                                    
    PORTE_PCR24 |= PORT_PCR_SRE_MASK        /* Slow slew rate */
                // |  PORT_PCR_ODE_MASK        /* Open Drain Enable */
                |  PORT_PCR_DSE_MASK        /* High drive strength */
            ;
    PORTE_PCR24 = PORT_PCR_MUX(1);
    
    GPIOE_PCOR  |= 1 << 24; /* pin=0, nordic off */
    GPIOE_PDDR  |= 1 << 24; /* pin output */   

#if defined(MCU_MK60N512VMD100)
    SIM_SCGC5   |= SIM_SCGC5_PORTA_MASK;    /* Enable clock gating to PORTA */
    /* Enable LEDs Port A pin 28 & 29 */                                    
    PORTA_PCR28 |= PORT_PCR_SRE_MASK        /* Slow slew rate */
                |  PORT_PCR_ODE_MASK        /* Open Drain Enable */
                |  PORT_PCR_DSE_MASK        /* High drive strength */
            ;
    PORTA_PCR28 = PORT_PCR_MUX(1);
    PORTA_PCR29 |= PORT_PCR_SRE_MASK        /* Slow slew rate */
                |  PORT_PCR_ODE_MASK        /* Open Drain Enable */
                |  PORT_PCR_DSE_MASK        /* High drive strength */
            ;
    PORTA_PCR29 = PORT_PCR_MUX(1);
    GPIOA_PSOR  |= 1 << 28 | 1 << 29;
    GPIOA_PDDR  |= 1 << 28 | 1 << 29;
    /* set in put PORTA pin 19 */
    PORTA_PCR19 =  PORT_PCR_MUX(1);
    GPIOA_PDDR &= ~((uint_32)1 << 19);
    PORTA_PCR19 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;/* pull up*/
#endif

#ifdef _MCF51JM128_H
    /* init buttons on port G */
    PTGDD &= 0xF0;                          /* set PTG0-3 to input */
    PTGPE |= 0x0F;                          /* enable PTG0-3 pull-up resistor */
    /* Enable LEDs Port E pin 2 & 3 */
    PTEDD_PTEDD2= 1;
    PTEDD_PTEDD3= 1;
    PTED_PTED2  = 1;
    PTED_PTED3  = 1;
#endif

#ifdef __MCF52259_H__     
    MCF_GPIO_DDRTA &= ~MCF_GPIO_DDRTA_DDRTA0;			// PTA0 as Input
    MCF_GPIO_PTAPAR &= ~(MCF_GPIO_PTAPAR_PTAPAR0(3));	// set PTA0 as GPIO
#endif
    
} /* EndBody */
/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : Switch_mode
* Returned Value : none
* Comments       : Jump between application and bootloader
*
*END*--------------------------------------------------------------------*/
void Switch_mode(void)
{
    /* Body */
    volatile uint_32  temp = 1;   /* default the button is not pressed */
    /* Get PC and SP of application region */
    New_sp  = ((uint_32_ptr)IMAGE_ADDR)[0];
    New_pc  = ((uint_32_ptr)IMAGE_ADDR)[1];
    /* Check switch is pressed*/
#ifdef  __MCF52259_H__ 
    temp =(uint_8) ((1<<0) & MCF_GPIO_SETTA);      	/* DES READ PTA0, SW1 of TWR-MCF5225X */
#elif defined(MCU_MK60N512VMD100)
    temp =(uint_32) ((1<<19) & GPIOA_PDIR);               /* DES READ SW1 of TWK60 */
#elif defined(_MCF51JM128_H)
    temp =(uint_32) ((1<<1) & PTGD);                      /* DES READ SW1 of JM128EVB */
#endif /* End __MCF52259_H__ */

    temp =(uint_32) ((1ul<<8) & GPIOD_PDIR);               /* DES READ BTN of WUNDERBAR */

    if(temp)
    {
        if((New_sp != 0xffffffff)&&(New_pc != 0xffffffff))
        {
            /* Run the application */
#if (!defined __MK_xxx_H__)
            asm
            {
                move.w   #0x2700,sr
                move.l   New_sp,a0
                move.l   New_pc,a1
                move.l   a0,a7
                jmp     (a1)
            }
#elif defined(__CWCC__)
            asm
            {
                ldr   r4,=New_sp
                ldr   sp, [r4]
                ldr   r4,=New_pc
                ldr   r5, [r4]
                blx   r5
            }
#elif defined(__IAR_SYSTEMS_ICC__)
            asm("mov32   r4,New_sp");
            asm("ldr     sp,[r4]");
            asm("mov32   r4,New_pc");
            asm("ldr     r5, [r4]");
            asm("blx     r5");
#endif /* end (!defined __MK_xxx_H__) */
        } /* EndIf */
    }

} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : erase_flash
* Returned Value : none
* Comments       : erase flash memory in application area
*
*END*--------------------------------------------------------------------*/

uint_8 erase_flash(void)
{ 
    /* Body */
    uint_8 error = FALSE;
    uint_8 i;
    Flash_Init(59);
    printf("\n\nErasing flash memory...\n\r");
    DisableInterrupts;                      
    for (i=0;i<(MAX_FLASH1_ADDRESS -(uint_32) IMAGE_ADDR)/ERASE_SECTOR_SIZE;i++)
    {
#if (!defined __MK_xxx_H__)    
        error = Flash_SectorErase((uint_32*)((uint_32) IMAGE_ADDR + ERASE_SECTOR_SIZE*i)) ; /* ERASE 4k flash memory */
#else
        error = Flash_SectorErase((uint_32) IMAGE_ADDR + ERASE_SECTOR_SIZE*i) ; /* ERASE 4k flash memory */
#endif
        if(error != Flash_OK)
        {
            printf("\nErase flash error!\n\r");
            break;
        } /* Endif */
        printf("#");
    } /* Endfor */
    EnableInterrupts;
    printf("\n\rERASE complete!\n\r");
    return error;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : SetOutput
* Returned Value : None
* Comments       : Set out put of the LEDs
*     
*END*--------------------------------------------------------------------*/

void SetOutput
    (
        /* [IN] the output pin */
        uint_32 output,
        /* [IN] the state to set */
        boolean state
    ) 
{
    /* Body */
    /* For 52259EVB */
#if (defined __MCF52259_H__)
    if(state)
        MCF_GPIO_PORTTC |= output; 
    else
        MCF_GPIO_PORTTC &=~ output; 
#endif

/* For TWR-K60 */
#if (defined MCU_MK60N512VMD100)
    if(state)
        GPIOA_PCOR |= output; 
    else
        GPIOA_PSOR |= output; 
#endif

/* For 51JM128EVB */
#if (defined _MCF51JM128_H)
    if(state)
        PTED &=~ output; 
    else
        PTED |= output; 
#endif
} /* EndBody */

  /* EOF */
