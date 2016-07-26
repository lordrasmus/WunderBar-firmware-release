/** @file   User_init.c
 *  @brief  File contains function for user modules initialization
 *          and stack initialization
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */


#include <string.h>
#include <stdint.h>

#include <hardware/Hw_modules.h>
#include <Common_Defaults.h>
#include "Events.h"

#include "User_Init.h"
#include "GS/GS_User/GS_Certificate.h"
#include "Sensors/Sensors_main.h"
#include "MQTT/MQTT_Api_Client/MQTT_Api.h"


wcfg_t wunderbar_configuration;


// static declarations

// to signal that when is ok to use ext int for spi purposes
static char ExtIntEn = 0;
static volatile unsigned int dummyread;
static unsigned int Sleep_CountDown = SLEEP_COUNTDOWN_MS;

static void Set_EI2_pin();
static void Init_vlps();
static void enter_vlps();
static unsigned int ADC_Measure(char channel);
static void my_VREF_Init(void);
static void Load_Wunderbar_Configuration();
static void Init_FPU();




	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////



/**
*  @brief  Reset Gain Span module by driving ExtReset pin low
*
*  @return void
*/
void Reset_Wifi(){
	GPIO_SetRstInputWifi();
	// wait while wifi rst pin is low
	while (GPIO_GetRstValueWifi() == false)
		;

	// take control of reset pin
	GPIO_SetRstOutputWifi();
	// pull reset pins down
	GPIO_ClrRstWifi();

	// release reset pin
	GPIO_SetRstInputWifi();
}

/**
*  @brief  Reset Nordic chip by driving NRset pin low
*
*  @return void
*/
void Reset_Nordic(){
	int cnt = 10000;

	// take control of reset pin
	GPIO_SetRstOutputNordic();
	// pull reset pins down
	GPIO_ClrRstNordic();
	// hold them down for some time
	while (cnt --)
		;
	// release reset pin
	GPIO_SetRstInputNordic();
}

/**
*  @brief  Global User init
*
*  Reset all connected modules and call initialize routines
*
*  @return void
*/
void Global_Peripheral_Init(){
	unsigned int battery_adc = 0;

	PMC_LVDSC1 = PMC_LVDSC1_LVDV(1);	// Set Low Voltage Detect trip point

	Init_FPU();                     	// init FPU module
	SPI_Init();							// init SPI module

	Reset_Nordic();						// reset nRF module
	Reset_Wifi();						// reset WiFi module

	Set_EI2_pin();						// Configure Ext int pin
	ExtIntEn = 1;						// Set Ext int flag

	ADC0_SC2 = ADC_SC2_REFSEL(0x01);  	// use internal reference 1.2 V for adc
	my_VREF_Init();                   	// init internal reference

	MSTimerDelay(500);					// small delay to stabilize

	while (battery_adc < 3500)			// Wait if battery is empty
	{
		battery_adc = ADC_Measure(ADC_VOLTAGE_SENSE_CHANNEL);	// measure adc voltage on battery pin
		battery_adc = (battery_adc * (unsigned int) (3.67 * VOLTAGE_REFERENCE_BANDGAP)) >> 16;

		MSTimerDelay(100);
	}

	GPIO_LedOn();						// turn on Kinetis led to signal all is OK

	// ---------------- //
	// Application init //

	Load_Wunderbar_Configuration();		// load configuration from the flash

	Sensors_Init();						// init sensors stack

	TI1_Enable();						// enable timer 1 interrupt (used for main state machine)

	Init_vlps();						// Init power saving mode
}

/**
*  @brief  Checks of the wifi module is ready
*
*  On power up GS module will drive RST pin high to indicate ready state.
*
*  @return Sstate of the rst pin
*/
int Chec_Wifi_Rst_Stable(){
	if (GPIO_GetRstValueWifi())
		return 1;
	return 0;
}

/**
*  @brief  Returns ExtIntEn flag
*
*  @return 1  - If ext int is ready to be used
*/
char Check_ExtIntEn(){
	return ExtIntEn;
}

/**
*  @brief  Check if the master module ID is empty
*
*  Check if the master module exists in flash
*  After onboarding process it will be loaded and used later on.
*
*  @param  Pointer to wcfg struct (Loaded from flash)
*
*  @return 1  - if master module id exists, 0  - missing
*/
char Check_MainBoard_ID_Exists(wcfg_t* wcfg){
	uint8_t i;

	for (i = 0; i < sizeof(wcfg->wunderbar.id); i ++)
	{
		if ((char) wcfg->wunderbar.id[i] != (char) 0xFF)
			return 1;
	}
	return 0;
}

/**
*  @brief  Resotre sleep countdown counter
*
*  Should be called when an action is performed to avoid going into sleep mode.
*
*  @return void
*/
void Sleep_Restore_Countdown(){
#ifdef __SLEEP__
	Sleep_CountDown = SLEEP_COUNTDOWN_MS;
#endif
}

/**
*  @brief  Check if sleep conditions are met.
*
*  IF sleep condition is met (no actions for last SLEEP_COUNTDOWN_MS milliseconds)
*  initiate sleep mode on kinetis.
*
*  @return void
*/
void Sleep_CheckConditions(){
	if (Sleep_CountDown == 0)
	{
		GPIO_LedOff();
		enter_vlps();
		Sleep_Restore_Countdown();
	}
}

/**
*  Count down sleep counter
*
*  If mqtt is running and no actions are performed countdown sleep counter.
*  Should be called from timer interrupt (TI2)
*
*  @return void
*/
void Sleep_DecrementCountDown(){
	if (Sleep_CountDown != 0)
		Sleep_CountDown -= TIMER2_INT_PERIOD;
}



	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////


/**
*  @brief  Initializes FPU module of the chip
*
*  @return void
*/
static void Init_FPU(){
    asm ("MOVW r1,#60808 "); 			// load CPACR
    asm ("MOVT r1,#57344");				// (0xE000ED88)
    asm ("LDR r0,[r1]");
    asm ("ORR.W r0,r0,#0x00F00000");	// Enable FPU
    asm ("STR r0, [r1]");				// on CP10, CP11
    asm ("MOV.W r0,#0x000000");
    asm ("VMSR FPSCR, r0");
}

/**
*  @brief  Load wunderbar configuration from flash after power up / reset
*
*  Load wunderbar configuration from flash.
*  Load predefined settings from flash (if defined)
*
*  @return void
*/
static void Load_Wunderbar_Configuration(){
	const wcfg_t *p_const_wcfg = (void*) FLASH_CONFIG_IMAGE_ADDR;
	const unsigned int *p_const_size = (void *) FLASH_CERTIFICATE_IMAGE_ADDRESS;

	wunderbar_configuration = *p_const_wcfg;

#ifdef __USE_DEFAULTS__
	if (wunderbar_configuration.wifi.ssid[0] == 0xFF)
	{
		strcpy((char *) wunderbar_configuration.wifi.ssid,     		(const char *) DEFAULT_SSID);
	}
	if (wunderbar_configuration.wifi.password[0] == 0xFF)
	{
		strcpy((char *) wunderbar_configuration.wifi.password, 		(const char *) DEFAULT_PASSWORD);
	}

	if (wunderbar_configuration.wunderbar.id[0] == 0xFF)
	{
		strcpy((char *) wunderbar_configuration.wunderbar.id,       (const char *) DEFAULT_USERNAME);
	}
	if (wunderbar_configuration.wunderbar.security[0] == 0xFF)
	{
		strcpy((char *) wunderbar_configuration.wunderbar.security, (const char *) DEFAULT_SECURITY);
	}

	if (wunderbar_configuration.cloud.url[0] == 0xFF)
	{
		strcpy((char *) wunderbar_configuration.cloud.url, 			(const char *) DEFAULT_MQTT_SERVER_URL);
	}
#endif

	if (*p_const_size == 0xFFFFFFFF)
	{
		GS_Cert_StoreInFlash((char *) cacert);
	}
}

/**
*  @brief  Init VREF as internal bandgap voltage reference (1.2V)
*
*  @return void
*/
static void my_VREF_Init(void){

	SIM_SCGC4 |= SIM_SCGC4_VREF_MASK ;

    VREF_SC = (VREF_SC_VREFEN_MASK | VREF_SC_MODE_LV(2)); 	// Tight-regulation mode buffer enabled is recommended over low buffered mode
    while (!(VREF_SC & VREF_SC_VREFST_MASK)  ){} 			// wait till the VREFSC is stable
}

/**
*  @brief  Measure 16bit adc value on desired channel and average it.
*
*  @param  desired channel to read
*
*  @return 16bit adc value
*/
static unsigned int ADC_Measure(char channel){
	unsigned int ad1_value = 0, temp = 0, i;

	for (i = 0; i < 16; i ++)
	{
		while (AD1_MeasureChan(true, (byte) channel) != ERR_OK)
			;

		AD1_GetValue16((word *) &temp);
		ad1_value += temp;
	}

	return (ad1_value >> 4);
}

/**
*  @brief  Set Ext Int 2 internal pull down
*
*  @return void
*/
static void Set_EI2_pin(){
	/* set in put PORTA pin 10 */
//	PORTA_PCR10 = PORT_PCR_MUX(1);
//	GPIOA_PDDR &= ~((uint32_t)1 << 10);

	/* pull down*/
	PORTA_PCR10 &= ~PORT_PCR_PS_MASK;
	PORTA_PCR10 |= PORT_PCR_PE_MASK;
}

// ----------------------------------------------------------- //
// Power save block

/**
*  @brief  Enter vlps mode
*
*  Should be initialized first
*
*  @return void
*/
static void enter_vlps()
{

/* enable wakwup on uart rx edge */
UART0_S2 |= UART_S2_RXEDGIF_MASK;
UART0_BDH |= UART_BDH_RXEDGIE_MASK;
/* wait for write to complete to SMC before stopping core */
dummyread = UART0_BDH;
/* Now execute the stop instruction to go into VLPS */
if (!Sleep_CountDown) // check once more to make sure there's nothing to do...
	asm("WFI");
}

/***************************************************************/
/* VLPS mode entry routine. Puts the processor into VLPS mode
* directly from run or VLPR modes.
*
* Mode transitions:
* RUN to VLPS
* VLPR to VLPS
*
* Kinetis K:
Power Mode Entry Code
Power Management for Kinetis and ColdFire+ MCUs, Rev. 1, 11/2012
24 Freescale Semiconductor, Inc.
* when VLPS is entered directly from RUN mode,
* exit to VLPR is disabled by hardware and the system will
* always exit back to RUN.
*
* If however VLPS mode is entered from VLPR the state of
* the LPWUI bit determines the state the MCU will return
* to upon exit from VLPS.If LPWUI is 1 and an interrupt
* occurs you will exit to normal run mode instead of VLPR.
* If LPWUI is 0 and an interrupt occurs you will exit to VLPR.
*
* For Kinetis L:
* when VLPS is entered from run an interrupt will exit to run.
* When VLPS is entered from VLPR an interrupt will exit to VLPS
* Parameters:
* none
*/
/***************************************************************/

/**
*  @brief  Init vlps mode
*
*  @return void
*/
static void Init_vlps(){
	/* power save config */
	MCG_C5 |= MCG_C5_PLLSTEN0_MASK;
	SMC_PMCTRL |= SMC_PMCTRL_LPWUI_MASK;

	/*The PMPROT register may have already been written by init
	code. If so then this next write is not done since
	PMPROT is write once after RESET.
	This Write allows the MCU to enter the VLPR, VLPW,
	and VLPS modes. If AVLP is already written to 0
	Stop is entered instead of VLPS*/
	SMC_PMPROT = SMC_PMPROT_AVLP_MASK;
	/* Set the STOPM field to 0b010 for VLPS mode */
	SMC_PMCTRL &= ~SMC_PMCTRL_STOPM_MASK;
	SMC_PMCTRL |= SMC_PMCTRL_STOPM(0x2);
	/* wait for write to complete to SMC before stopping core */
	dummyread = SMC_PMCTRL;
	/* Set the SLEEPDEEP bit to enable deep sleep mode (STOP) */
	SCB_SCR |= SCB_SCR_SLEEPDEEP_MASK;
}
