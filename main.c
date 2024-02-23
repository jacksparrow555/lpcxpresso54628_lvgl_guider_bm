/* ###################################################################
**     Filename    : main.c
**     Project     : ISPU24_ZAS_PCBV1.0
**     Processor   : MKL02Z32VFM4
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2021-03-05, 14:14, # CodeGen: 0
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

/*
 * ###################################################################################################################
 * main.c
 *
 *  Created on: 5 mar 2021
 *      Author: Piotr.Dzierzak
 *
 * Najmniejszy zegar:
 * Core clock:	1,3 MHz
 * Bus clock:	0.16384 MHz
 *
 * Switch - wybor rodzaju wyjscia: 	ON-czestotliwosc (2-42Hz), przeciazenie 1Hz w przypadku kiedy Uwy<21,5V.
 * 									OFF-wyjscie przekaznikowe: zwarte jak OK, rozwarte jak Uwy<21,5V
 * 									histereza wyjsca=21500mV+20mV OK
 * 									histereza wyjsca=21500mV-20mV NOK
 * WDT=1000ms.
 *
 * ADC 12 bit@ 100 pomiarow trwa 240 ms.
 * Czyli stan LED jest odswiezany co 240ms ==ok 4Hz.
 *

 *###################################################################################################################
*/

/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "WAIT1.h"
#include "FRQ.h"
#include "LED1.h"
#include "SwitchS2.h"
#include "AD1_MIV.h"
#include "AdcLdd1.h"
#include "PPG1.h"
#include "WDog1.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* User includes (#include below this line is not maintained by Processor Expert) */
#include <math.h>	//do zaokroglania

#include <string.h>
#include <stdio.h>
//#include "SEGGER_RTT.h"



/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{


  for(;;)
  {

      //LED1_NegVal(NULL);
//=================================================================================================================================================================
       if(SWITCH_ON==SwitchS2_GetVal(NULL))	//SKANOWANIE USTAWIENIA SWITCHA
    	  SwitchBiezacy=eSWITCH_ON;
      else
    	  SwitchBiezacy=eSWITCH_OFF;

      if(SwitchBiezacy!=SwitchUstawiony)
   	  {
		  LicznikSwitchZmiana++;


	  	  if(LicznikSwitchZmiana>40)			//40*0,25s=10 SEKUND
	  	  {
	  		  LED1_SetVal(NULL);			//LED_OFF
	  		  WAIT1_Waitms(100);
   	  	  	  while(1);						//RESTART APLIKACJI - JESLI W TRAKCIE PRACY SWITCH ZOSTANIE PRZELACZONY TO BEDZIE NIEZGODNOSC I ZAINICJOWANYM HARDWAREM.
      	  	  	  	  	  	  	  	  	  	//RESTART I PONOWNA INICJALIZACJA ZGODNIE Z NOWYM USTAWIENIEM SWITCHA
	  	  }
   	  } else LicznikSwitchZmiana=0;
//=================================================================================================================================================================







  }

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
