/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 ******************************************************************************
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
 **************************************************************************//*!
 *
 * @file main_kinetis.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief   This software is the USB driver stack for S08 family
 *****************************************************************************/
#include "types.h"
#include "derivative.h" /* include peripheral declarations */
#include "user_config.h"
#include "RealTimerCounter.h"
#include "Wdt_kinetis.h"
#include "hidef.h"
#ifdef BOOTLOADER_APP       /* Bootloader application */
#include "Boot_loader_task.h"
#endif

extern uint_32 ___VECTOR_RAM[];            //Get vector table that was copied to RAM
extern uint_32 __vector_table[];            //Get vector table that was copied to RAM

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
#if MAX_TIMER_OBJECTS
extern uint_8 TimerQInitialize(uint_8 ControllerId);
#endif
extern void TestApp_Init(void);
extern void TestApp_Task(void);
#if (defined MCU_MK60N512VMD100) || (defined MCU_MK64F12) 
#define BSP_CLOCK_SRC                   (50000000ul)       // crystal, oscillator freq
#elif defined MCU_MK24F12 
#define BSP_CLOCK_SRC                   (12000000ul)       // crystal, oscillator freq
#else
#define BSP_CLOCK_SRC                   (8000000ul)       // crystal, oscillator freq
#endif
#define BSP_REF_CLOCK_SRC               (2000000ul)       // must be 2-4MHz

#define BSP_CORE_DIV                    (1)
#define BSP_BUS_DIV                     (1)
#define BSP_FLEXBUS_DIV                 (1)
#define BSP_FLASH_DIV                   (2)

/* BSP_CLOCK_MUL from interval 24 - 55 */
#define BSP_CLOCK_MUL                   (24)    /* 48MHz */

#define BSP_REF_CLOCK_DIV               (BSP_CLOCK_SRC / BSP_REF_CLOCK_SRC)

#define BSP_CLOCK                       (BSP_REF_CLOCK_SRC * BSP_CLOCK_MUL)
#define BSP_CORE_CLOCK                  (BSP_CLOCK / BSP_CORE_DIV)          /* CORE CLK, max 100MHz */
#define BSP_SYSTEM_CLOCK                (BSP_CORE_CLOCK)                    /* SYSTEM CLK, max 100MHz */
#define BSP_BUS_CLOCK                   (BSP_CLOCK / BSP_BUS_DIV)       /* max 50MHz */
#define BSP_FLEXBUS_CLOCK               (BSP_CLOCK / BSP_FLEXBUS_DIV)
#define BSP_FLASH_CLOCK                 (BSP_CLOCK / BSP_FLASH_DIV)     /* max 25MHz */

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static void Init_Sys(void);
static unsigned char pll_init();
static void wdog_disable(void);
static void StartKeyPressSimulationTimer(void);
static void KeyPressSimulationTimerCallback(void);
static void display_led(uint_8 val);

/****************************************************************************
 * Global Variables
 ****************************************************************************/
volatile uint_8 kbi_stat;	   /* Status of the Key Pressed */


/*****************************************************************************
 * Global Functions
 *****************************************************************************/
/******************************************************************************
 * @name        main
 *
 * @brief       This routine is the starting point of the application
 *
 * @param       None
 *
 * @return      None
 *
 *****************************************************************************
 * This function initializes the system, enables the interrupts and calls the
 * application
 *****************************************************************************/
void main(void)
{
#ifdef BOOTLOADER_APP       /* Bootloader application */
	GPIO_Bootloader_Init();
	sci_init();
	Switch_mode();              /* switch between the application and the bootloader mode */
	Init_Sys();        /* initial the system */
#endif
    
#if MAX_TIMER_OBJECTS
    (void)TimerQInitialize(0);
#endif
    (void)TestApp_Init(); /* Initialize the USB Test Application */
    
    while(TRUE)
    {
       Watchdog_Reset();
       /* Call the application task */
       TestApp_Task();
       
    }
}

/*****************************************************************************
 * Local Functions
 *****************************************************************************/
/*****************************************************************************
 *
 *    @name     GPIO_Init
 *
 *    @brief    This function Initializes LED GPIO
 *
 *    @param    None
 *
 *    @return   None
 *
 ****************************************************************************
 * Intializes the GPIO
 ***************************************************************************/
void GPIO_Init()
{
	display_led(1); /* pin=1, led on */

/*    SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
    PORTE_PCR6|= PORT_PCR_SRE_MASK     Slow slew rate 
              |  PORT_PCR_ODE_MASK     Open Drain Enable 
              |  PORT_PCR_DSE_MASK     High drive strength 
              ;
    PORTE_PCR6 = PORT_PCR_MUX(1);
    PORTE_PCR7|= PORT_PCR_SRE_MASK     Slow slew rate 
              |  PORT_PCR_ODE_MASK     Open Drain Enable 
              |  PORT_PCR_DSE_MASK     High drive strength 
              ;
    PORTE_PCR7 = PORT_PCR_MUX(1);
    PORTE_PCR8|= PORT_PCR_SRE_MASK     Slow slew rate 
              |  PORT_PCR_ODE_MASK     Open Drain Enable 
              |  PORT_PCR_DSE_MASK     High drive strength 
              ;
    PORTE_PCR8 = PORT_PCR_MUX(1);
    PORTE_PCR9|= PORT_PCR_SRE_MASK     Slow slew rate 
              |  PORT_PCR_ODE_MASK     Open Drain Enable 
              |  PORT_PCR_DSE_MASK     High drive strength 
              ;
    PORTE_PCR9 = PORT_PCR_MUX(1);
    GPIOE_PCOR |= 1 << 6 | 1 << 7 | 1 << 8 | 1 << 9;
    GPIOE_PDDR |= 1 << 6 | 1 << 7 | 1 << 8 | 1 << 9;*/

    // SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
    // PORTC_PCR8|= PORT_PCR_SRE_MASK    /* Slow slew rate */
    //           |  PORT_PCR_ODE_MASK    /* Open Drain Enable */
    //          |  PORT_PCR_DSE_MASK    /* High drive strength */
    //           ;
    // PORTC_PCR8 = PORT_PCR_MUX(1);
    // PORTC_PCR9|= PORT_PCR_SRE_MASK    /* Slow slew rate */
    //           |  PORT_PCR_ODE_MASK    /* Open Drain Enable */
    //           |  PORT_PCR_DSE_MASK    /* High drive strength */
    //           ;
    // PORTC_PCR9 = PORT_PCR_MUX(1);
    // GPIOC_PSOR |= 1 << 8 | 1 << 9;
    // GPIOC_PDDR |= 1 << 8 | 1 << 9;
    
    /* setting for port interrupt */
#if (defined MCU_MK40N512VMD100) ||  (defined MCU_MK53N512CMD100)
	/* set in put PORTC5*/
	PORTC_PCR5 =  PORT_PCR_MUX(1);
	GPIOC_PDDR &= ~((uint_32)1 << 5);
	/* pull up*/
	PORTC_PCR5 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	/* GPIO_INT_EDGE_HIGH */
	PORTC_PCR5 |= PORT_PCR_IRQC(9);	
	/* set in put PORTC13*/
	PORTC_PCR13 =  PORT_PCR_MUX(1);
	GPIOC_PDDR &= ~((uint_32)1 << 13);
	/* pull up*/
	PORTC_PCR13 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	/* GPIO_INT_EDGE_HIGH */
	PORTC_PCR13 |= PORT_PCR_IRQC(9);
	/* enable interrupt */
	PORTC_ISFR |=(1<<5);
	PORTC_ISFR |=(1<<13);
	NVICICPR2 = 1 << ((89)%32);
	NVICISER2 = 1 << ((89)%32);
#endif
	
#if defined(MCU_MK60N512VMD100) 
	/* Enable clock gating to PORTA and PORTE */
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK |SIM_SCGC5_PORTE_MASK;
	
	/* set in put PORTA pin 19 */
	PORTA_PCR19 =  PORT_PCR_MUX(1);
	GPIOC_PDDR &= ~((uint_32)1 << 19);
	
	/* pull up*/
	PORTA_PCR19 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	
	/* GPIO_INT_EDGE_HIGH */
	PORTA_PCR19 |= PORT_PCR_IRQC(9);	
	
	/* set in put PORTE pin 26 */
	PORTE_PCR26 =  PORT_PCR_MUX(1);
	GPIOC_PDDR &= ~((uint_32)1 << 26);
	
	/* pull up*/
	PORTE_PCR26 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	
	/* GPIO_INT_EDGE_HIGH */
	PORTE_PCR26 |= PORT_PCR_IRQC(9);
	
	/* Clear interrupt flag */
	PORTA_ISFR |=(1<<19);
	PORTE_ISFR |=(1<<26);
	
	/* enable interrupt port A */
	NVICICPR2 = 1 << ((87)%32);
	NVICISER2 = 1 << ((87)%32);
	
	/* enable interrupt port E */
	NVICICPR2 = 1 << ((91)%32);
	NVICISER2 = 1 << ((91)%32);
#endif
}

/******************************************************************************
 * @name       all_led_off
 *
 * @brief      Switch OFF all LEDs on Board
 *
 * @param	   None
 *
 * @return     None
 *
 *****************************************************************************
 * This function switch OFF all LEDs on Board
 *****************************************************************************/
static void all_led_off(void)
{
	GPIOA_PCOR |= 1 << 29;
}

/******************************************************************************
 * @name       display_led
 *
 * @brief      Displays 8bit value on Board LEDs
 *
 * @param	   val
 *
 * @return     None
 *
 *****************************************************************************
 * This function displays 8 bit value on Board LED
 *****************************************************************************/
void display_led(uint_8 val)
{
    uint_8 i = 0;
    all_led_off();

	val &= 0x01;
	if(val)
		GPIOA_PSOR |= 1 << 29;
}
/*****************************************************************************
 *
 *    @name     Init_Sys
 *
 *    @brief    This function Initializes the system
 *
 *    @param    None
 *
 *    @return   None
 *
 ****************************************************************************
 * Intializes the MCU, MCG, KBI, RTC modules
 ***************************************************************************/
static void Init_Sys(void)
{
	unsigned char *ptr;
	
   /* Point the VTOR to the new copy of the vector table */
#if (defined MCU_MK64F12) || (defined MCU_MK24F12)
	SCB_VTOR = (uint_32)__vector_table;
#else	
    SCB_VTOR = (uint_32)___VECTOR_RAM;
#endif	
	  
	NVICICER1|=(1<<21);                      /* Clear any pending interrupts on USB */
	NVICISER1|=(1<<21);                      /* Enable interrupts from USB module */  

    /* SIM Configuration */
	GPIO_Init();
	pll_init();
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
}

/******************************************************************************
*   @name        IRQ_ISR_PORTA
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void IRQ_ISR_PORTA(void)
{
	NVICICPR2 = 1 << ((87)%32);
	NVICISER2 = 1 << ((87)%32);	
	DisableInterrupts;
	if(PORTA_ISFR & (1<<19))
	{
		kbi_stat |= 0x02; 				/* Update the kbi state */
		PORTA_ISFR |=(1<<19);			/* Clear the bit by writing a 1 to it */
	}
	EnableInterrupts;
}

/******************************************************************************
*   @name        IRQ_ISR
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void IRQ_ISR_PORTC(void)
{
	NVICICPR2 = 1 << ((89)%32);
	NVICISER2 = 1 << ((89)%32);	
	DisableInterrupts;
	if(PORTC_ISFR & (1<<5))
	{
		kbi_stat |= 0x02; 				/* Update the kbi state */
		PORTC_ISFR |=(1<<5);			/* Clear the bit by writing a 1 to it */
	}
	
	if(PORTC_ISFR & (1<<13))
	{
		kbi_stat |= 0x08;				/* Update the kbi state */
		PORTC_ISFR |=(1<<13);			/* Clear the bit by writing a 1 to it */
	}	
	EnableInterrupts;
}

/******************************************************************************
*   @name        IRQ_ISR_PORTE
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void IRQ_ISR_PORTE(void)
{
	NVICICPR2 = 1 << ((91)%32);
	NVICISER2 = 1 << ((91)%32);	
	DisableInterrupts;
	if(PORTE_ISFR & (1<<26))
	{
		kbi_stat |= 0x08;				/* Update the kbi state */
		PORTE_ISFR |=(1<<26);			/* Clear the bit by writing a 1 to it */
	}
	EnableInterrupts;
}

/*****************************************************************************
 * @name     wdog_disable
 *
 * @brief:   Disable watchdog.
 *
 * @param  : None
 *
 * @return : None
 *****************************************************************************
 * It will disable watchdog.
  ****************************************************************************/
static void wdog_disable(void)
{
	/* Write 0xC520 to the unlock register */
	WDOG_UNLOCK = 0xC520;
	
	/* Followed by 0xD928 to complete the unlock */
	WDOG_UNLOCK = 0xD928;
	
	/* Clear the WDOGEN bit to disable the watchdog */
	WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}
/*****************************************************************************
 * @name     pll_init
 *
 * @brief:   Initialization of the MCU.
 *
 * @param  : None
 *
 * @return : None
 *****************************************************************************
 * It will configure the MCU to disable STOP and COP Modules.
 * It also set the MCG configuration and bus clock frequency.
 ****************************************************************************/
static unsigned char pll_init()
{      
	/*This assumes that the MCG is in default FEI mode out of reset. */
    
    /* First move to FBE mode */
#if (defined MCU_MK60N512VMD100) 
	/* Enable external oscillator, RANGE=0, HGO=, EREFS=, LP=, IRCS= */
	MCG_C2 = 0;
#elif (defined MCU_MK64F12) 
	/* Enable external oscillator, RANGE=0, HGO=, EREFS=, LP=, IRCS= */
	MCG_C2 = MCG_C2_RANGE0(2);
#elif (defined MCU_MK24F12)	
	/* Enable external oscillator, RANGE=0, HGO=, EREFS=, LP=, IRCS= */
	// MCG_C7 = MCG_C7_OSCSEL(2);
	// MCG_C2 = MCG_C2_RANGE0(2);
	MCG_C2 = MCG_C2_RANGE0(1) | MCG_C2_EREFS0_MASK;
#else	
	/* Enable external oscillator, RANGE=2, HGO=1, EREFS=1, LP=0, IRCS=0 */
	MCG_C2 = MCG_C2_RANGE(2) | MCG_C2_HGO_MASK | MCG_C2_EREFS_MASK|MCG_C2_IRCS_MASK;
#endif
	
    /* Select external oscilator and Reference Divider and clear IREFS to start ext osc 
	   CLKS=2, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0 */
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(6);

#ifndef MCU_MK64F12    
#ifndef MCU_MK60N512VMD100
	/* wait for oscillator to initialize */
    while (!(MCG_S & MCG_S_OSCINIT0_MASK)){};  
#endif
#endif   
   
   /* wait for Reference clock Status bit to clear */
    while (MCG_S & MCG_S_IREFST_MASK){}; 

    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2){}; /* Wait for clock status bits to show clock source is ext ref clk */
    
#if (defined MCU_MK64F12) || (defined MCU_MK24F12)
	MCG_C5 = MCG_C5_PRDIV0(BSP_REF_CLOCK_DIV - 1);
#elif (defined MCU_MK60N512VMD100)
	MCG_C5 = MCG_C5_PRDIV(BSP_REF_CLOCK_DIV - 1);
#else
    MCG_C5 = MCG_C5_PRDIV(BSP_REF_CLOCK_DIV - 1) | MCG_C5_PLLCLKEN_MASK;
#endif
    // Ensure MCG_C6 is at the reset default of 0. LOLIE disabled, 
    // PLL enabled, clk monitor disabled, PLL VCO divider is clear 
    MCG_C6 = 0;

    /* Set system options dividers */
    
    SIM_CLKDIV1 =   SIM_CLKDIV1_OUTDIV1(BSP_CORE_DIV - 1) | 	/* core/system clock */
                    SIM_CLKDIV1_OUTDIV2(BSP_BUS_DIV - 1)  | 	/* peripheral clock; */
					SIM_CLKDIV1_OUTDIV3(BSP_FLEXBUS_DIV - 1) |  /* FlexBus clock driven to the external pin (FB_CLK)*/
					SIM_CLKDIV1_OUTDIV4(BSP_FLASH_DIV - 1);     /* flash clock */
     
    // Set the VCO divider and enable the PLL, LOLIE = 0, PLLS = 1, CME = 0, VDIV = 
#if (defined MCU_MK64F12) || (defined MCU_MK24F12)
    MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV0(BSP_CLOCK_MUL - 24);  // 2MHz * BSP_CLOCK_MUL 
#else
    MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(BSP_CLOCK_MUL - 24);  // 2MHz * BSP_CLOCK_MUL 
#endif    

    while (!(MCG_S & MCG_S_PLLST_MASK)){};  // wait for PLL status bit to set 
#if (defined MCU_MK64F12) || (defined MCU_MK24F12)
    while (!(MCG_S & MCG_S_LOCK0_MASK)){};  // Wait for LOCK bit to set 
#else    
   	while (!(MCG_S & MCG_S_LOCK_MASK)){};  // Wait for LOCK bit to set 
#endif   	
    	    
    // Transition into PEE by setting CLKS to 0
    // CLKS=0, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0 
    MCG_C1 &= ~MCG_C1_CLKS_MASK;

    // Wait for clock status bits to update 
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3){};

    return 0;
}


/* EOF */
