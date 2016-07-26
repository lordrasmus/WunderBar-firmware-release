/* ###################################################################
**     Filename    : main.c
**     Project     : Kinetis_MK24FN1M0VDC12_msd_bootPE
**     Processor   : MK24FN1M0VLQ12
**     Version     : Driver 01.01
**     Compiler    : CodeWarrior ARM C Compiler
**     Date/Time   : 2014-06-15, 15:14, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.01
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "Pins1.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PDD_Includes.h"
#include "Init_Config.h"

#include "types.h"
#include "derivative.h" /* include peripheral declarations */
#include "user_config.h"
#include "RealTimerCounter.h"
#include "Wdt_kinetis.h"
#include "hidef.h"
#ifdef BOOTLOADER_APP       /* Bootloader application */
#include "Boot_loader_task.h"
#endif

/* User includes (#include below this line is not maintained by Processor Expert) */

static void Init_Sys(void)
{
	
   /* Point the VTOR to the new copy of the vector table */
/*#if (defined MCU_MK64F12) || (defined MCU_MK24F12)
	SCB_VTOR = (uint_32)__vector_table;
#else	
    SCB_VTOR = (uint_32)___VECTOR_RAM;
#endif	*/
	  
	NVICICER1|=(1<<21);                      /* Clear any pending interrupts on USB */
	NVICISER1|=(1<<21);                      /* Enable interrupts from USB module */  

    /* SIM Configuration */
	// GPIO_Init();
	// pll_init();
    MPU_CESR=0x00;
    
    /************* USB Part **********************/
    /*********************************************/   
    /* Configure USBFRAC = 0, USBDIV = 1 => frq(USBout) = 1 / 2 * frq(PLLin) */
    SIM_CLKDIV2 &= SIM_CLKDIV2_USBFRAC_MASK | SIM_CLKDIV2_USBDIV_MASK;
    SIM_CLKDIV2|= SIM_CLKDIV2_USBDIV(0);
    
    /* Enable USB-OTG IP clocking */
    SIM_SCGC4|=(SIM_SCGC4_USBOTG_MASK);          
    
    /* Configure USB to be clocked from PLL */
    SIM_SOPT2  |= SIM_SOPT2_USBSRC_MASK /*| (3ul << SIM_SOPT2_PLLFLLSEL_SHIFT)*/ | SIM_SOPT2_PLLFLLSEL_MASK;
    
    /* Configure enable USB regulator for device */
    SIM_SOPT1 |= SIM_SOPT1_USBREGEN_MASK;
    /*
    // USB_CLK_RECOVER_IRC_EN = 3ul;
    ptr = (unsigned char *)0x40072144;
    *ptr = 3ul;
       
    // USBx_CLK_RECOVER_CTRL = 0x80ul;
    ptr = (unsigned char *)0x40072140;
    *ptr = 0x80ul;*/
}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/
  Init_Sys();
  /* Write your code here */
  /* For example: for(;;) { } */

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
    (void)TestApp_Init(); /* Initialize the USB Test Application */
    
    while(TRUE)
    {
       Watchdog_Reset();
       /* Call the application task */
       TestApp_Task();
       
    }
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.3 [05.09]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
