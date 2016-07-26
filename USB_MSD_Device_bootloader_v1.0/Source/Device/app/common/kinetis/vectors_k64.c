#include "derivative.h" /* include peripheral declarations */

/*--------------------------------------------------------------*/
  typedef void (*const tIsrFunc)(void);
  typedef struct {
    uint32_t * __ptr;
    tIsrFunc __fun[101];
  } tVectorTable;

#ifndef _SERIAL_AGENT_
  extern void USB_ISR();
#endif
#if (defined(_SERIAL_BRIDGE_)|defined(_SERIAL_AGENT_))
  extern void UART3_RTx_ISR(void);
  extern void UART3_Err_ISR(void);
#endif

#ifdef USED_PIT1
  extern void  pit1_isr(void);
#endif 
#ifdef USED_PIT0
  extern void  Timer_ISR(void);
#endif
  extern void IRQ_ISR_PORTA();
#if (defined MCU_MK40N512VMD100) ||  (defined MCU_MK53N512CMD100)
  extern void IRQ_ISR_PORTC();
#endif
  extern void IRQ_ISR_PORTE();
  extern void __thumb_startup( void );
  extern uint32_t __SP_INIT[];
  
  #pragma define_section vectortable ".vectortable" ".vectortable" ".vectortable" far_abs R
 
 void Cpu_INT_NMIInterrupt(void)
 {
   
 }
 
 void Cpu_Interrupt(void)
 {

 }
 
 /*lint -save  -e926 -e927 -e928 -e929 Disable MISRA rule (11.4) checking. Need to explicitly cast pointers to the general ISR for Interrupt vector table */
 static __declspec(vectortable) tVectorTable __vect_table = { /* Interrupt vector table */
   (uint32_t *) __SP_INIT,                                              /* 0 (0x00000000) (prior: -) */
   {
    (tIsrFunc)__thumb_startup,                              /* 1 (0x00000004) (prior: -) */
    (tIsrFunc)Cpu_INT_NMIInterrupt,                         /* 2 (0x00000008) (prior: -2) */
    (tIsrFunc)Cpu_Interrupt,                               /* 3 (0x0000000C) (prior: -1) */
    (tIsrFunc)Cpu_Interrupt,                               /* 4 (0x00000010) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 5 (0x00000014) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 6 (0x00000018) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 7 (0x0000001C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 8 (0x00000020) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 9 (0x00000024) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 10 (0x00000028) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 11 (0x0000002C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 12 (0x00000030) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 13 (0x00000034) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 14 (0x00000038) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 15 (0x0000003C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 16 (0x00000040) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 17 (0x00000044) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 18 (0x00000048) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 19 (0x0000004C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 20 (0x00000050) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 21 (0x00000054) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 22 (0x00000058) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 23 (0x0000005C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 24 (0x00000060) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 25 (0x00000064) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 26 (0x00000068) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 27 (0x0000006C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 28 (0x00000070) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 29 (0x00000074) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 30 (0x00000078) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 31 (0x0000007C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 32 (0x00000080) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 33 (0x00000084) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 34 (0x00000088) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 35 (0x0000008C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 36 (0x00000090) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 37 (0x00000094) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 38 (0x00000098) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 39 (0x0000009C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 40 (0x000000A0) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 41 (0x000000A4) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 42 (0x000000A8) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 43 (0x000000AC) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 44 (0x000000B0) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 45 (0x000000B4) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 46 (0x000000B8) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 47 (0x000000BC) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 48 (0x000000C0) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 49 (0x000000C4) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 50 (0x000000C8) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 51 (0x000000CC) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 52 (0x000000D0) (prior: -) */
#if (defined(_SERIAL_BRIDGE_)|defined(_SERIAL_AGENT_))  
   (tIsrFunc)UART3_RTx_ISR,           /* 53  0x0000010C   -   ivINT_UART3_RX_TX              unused by PE */
   (tIsrFunc)UART3_Err_ISR,           /* 54  0x00000110   -   ivINT_UART3_ERR                unused by PE */
#else
   (tIsrFunc)Cpu_Interrupt,           /* 53  0x0000010C   -   ivINT_UART3_RX_TX              unused by PE */
   (tIsrFunc)Cpu_Interrupt,           /* 54  0x00000110   -   ivINT_UART3_ERR                unused by PE */
#endif      
    (tIsrFunc)Cpu_Interrupt,                               /* 55 (0x000000DC) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 56 (0x000000E0) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 57 (0x000000E4) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 58 (0x000000E8) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 59 (0x000000EC) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 60 (0x000000F0) (prior: -) */
#ifdef _CMT_H
   (tIsrFunc)cmt_isr,           	  /* 61  0x00000144   -   ivINT_CMT                      unused by PE */
#else
   (tIsrFunc)Cpu_Interrupt,           /* 61  0x00000144   -   ivINT_CMT                      unused by PE */
#endif
    (tIsrFunc)Cpu_Interrupt,                               /* 62 (0x000000F8) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 63 (0x000000FC) (prior: -) */
#ifdef USED_PIT0
   (tIsrFunc)Timer_ISR,           	  /* 64  0x00000150   -   ivINT_PIT0                     unused by PE */
#else
   (tIsrFunc)Cpu_Interrupt,           /* 64  0x00000150   -   ivINT_PIT0                     unused by PE */
#endif
#ifdef USED_PIT1
   (tIsrFunc)pit1_isr,           	  /* 65  0x00000154   -   ivINT_PIT1                      unused by PE */
#else
   (tIsrFunc)Cpu_Interrupt,           /* 65  0x00000154   -   ivINT_PIT1                      unused by PE */
#endif
    (tIsrFunc)Cpu_Interrupt,                               /* 66 (0x00000108) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 67 (0x0000010C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 68 (0x00000110) (prior: -) */
#ifndef _SERIAL_AGENT_
   (tIsrFunc)USB_ISR,                 /* 69  0x00000164   -   ivINT_USB0                     unused by PE */
#else
   (tIsrFunc)Cpu_Interrupt,           /* 69  0x00000164   -   ivINT_USB0                     unused by PE */
#endif
    (tIsrFunc)Cpu_Interrupt,                               /* 70 (0x00000118) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 71 (0x0000011C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 72 (0x00000120) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 73 (0x00000124) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 74 (0x00000128) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 75 (0x0000012C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 76 (0x00000130) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 77 (0x00000134) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 78 (0x00000138) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 79 (0x0000013C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 80 (0x00000140) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 81 (0x000000FC) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 82 (0x00000100) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 83 (0x00000104) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 84 (0x00000108) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 85 (0x0000010C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 86 (0x00000110) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 87 (0x00000114) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 88 (0x00000118) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 89 (0x0000011C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 90 (0x00000120) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 91 (0x00000124) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 92 (0x00000128) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 93 (0x0000012C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 94 (0x00000130) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 95 (0x00000134) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 96 (0x00000138) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 97 (0x0000013C) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 98 (0x00000140) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 99 (0x00000144) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 100 (0x00000148) (prior: -) */
    (tIsrFunc)Cpu_Interrupt,                               /* 101 (0x0000014C) (prior: -) */
   }
 };
